#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_tif_error.h"
#include "igs_tif_read_tile.h"
#include "igs_resource_irw.h"

/* tile読み込み */
//------------------------------------------------------------
namespace {
 unsigned int u_trcopy_tile_8bits(
	unsigned int u_tile_wi,
	unsigned int u_w_expand_tile,
	unsigned int u_h_expand_tile,
	unsigned int u_d_pixel,
	uint8 *ui8p_top_tile,
	unsigned int u_wi,
	uint8 *ui8p_top_image
 ) {
	unsigned int u_tile_scanline_size = u_tile_wi  * u_d_pixel,
		u_image_scanline_size = u_wi * u_d_pixel,
	  u_expand_tile_scanline_size = u_w_expand_tile * u_d_pixel;
	unsigned int xx, yy;
	uint8	*ui8p_y_tile  = ui8p_top_tile,
		*ui8p_y_image = ui8p_top_image;

	for (yy = 0; yy < u_h_expand_tile; ++yy,
		ui8p_y_tile += u_tile_scanline_size,
		ui8p_y_image += u_image_scanline_size
	) {
		for (xx = 0; xx < u_expand_tile_scanline_size; ++xx) {
			ui8p_y_image[xx] = ui8p_y_tile[xx];
		}
	}
	return u_h_expand_tile * u_expand_tile_scanline_size;
 }
 unsigned int u_trcopy_tile_16bits(
	unsigned int u_tile_wi,
	unsigned int u_w_expand_tile,
	unsigned int u_h_expand_tile,
	unsigned int u_d_pixel,
	uint16 *ui16p_top_tile,
	unsigned int u_wi,
	uint16 *ui16p_top_image
 ) {
	unsigned int u_tile_scanline_size = u_tile_wi  * u_d_pixel,
		u_image_scanline_size = u_wi * u_d_pixel,
	  u_expand_tile_scanline_size = u_w_expand_tile * u_d_pixel;
	unsigned int xx, yy;
	uint16 *ui16p_y_tile = ui16p_top_tile,
		*ui16p_y_image = ui16p_top_image;

	for (yy = 0; yy < u_h_expand_tile; ++yy,
		ui16p_y_tile += u_tile_scanline_size,
		ui16p_y_image += u_image_scanline_size
	) {
		for (xx = 0; xx < u_expand_tile_scanline_size; ++xx) {
			ui16p_y_image[xx] = ui16p_y_tile[xx];
		}
	}
	return u_h_expand_tile * u_expand_tile_scanline_size;
 }
}
//------------------------------------------------------------
using igs::tif::read_tile;

unsigned int read_tile::_u_current_tile_w(void) const
{
	if (	(this->_u_wi - this->_u_current_x_image) <
		 this->_u_tile_wi
	) {
	 return this->_u_wi - this->_u_current_x_image;
	}
	return this->_u_tile_wi;
}
unsigned int read_tile::_u_current_tile_h(void) const
{
	if (	(this->_u_he - this->_u_current_y_image) <
		 this->_u_tile_he
	) {
	 return this->_u_he - this->_u_current_y_image;
	}
	return this->_u_tile_he;
}
//------------------------------------------------------------
read_tile::read_tile(
	igs::tif::read_handle& handle_ref,
	unsigned char *dest_top,
	const size_t dest_size
):
	_u_he(0),
	_u_wi(0),
	_u_bi(0),
	_u_tile_he(0),
	_u_tile_wi(0),
	_u_phot(0),
	_u_tile_num_he(0),
	_u_tile_num_wi(0),
	_u_pixel_size(0),
	_u_read_bytes_size(0),
	_u_read_bytes_crnt(0),
	_u_current_count(0),
	_u_current_x_image(0),
	_u_current_y_image(0),
	dest_size_(dest_size),
	dest_top_(dest_top)
{
	this->_u_bi = handle_ref.bitspersample();

	switch (this->_u_bi) {
	case std::numeric_limits<unsigned char>::digits:
	case std::numeric_limits<unsigned short>::digits:
		break;
	default:
		std::ostringstream os;
		os	<< "TIFFTAG_BITSPERSAMPLE must "
			<< std::numeric_limits<unsigned char>::digits
			<< " or "
			<< std::numeric_limits<unsigned short>::digits
			<< ",but " << this->_u_bi;
		throw std::domain_error(os.str());
	}

	this->_u_he = handle_ref.imagelength();
	this->_u_wi = handle_ref.imagewidth();
	const unsigned int dd = handle_ref.samplesperpixel();

	this->_u_tile_he = handle_ref.tilelength();
	this->_u_tile_wi = handle_ref.tilewidth();

	this->_u_tile_num_he =
		  this->_u_he / this->_u_tile_he +
		((this->_u_he % this->_u_tile_he)? 1: 0);
	this->_u_tile_num_wi =
		  this->_u_wi / this->_u_tile_wi +
		((this->_u_wi % this->_u_tile_wi)? 1: 0);

	this->_u_pixel_size = this->_u_bi /
		std::numeric_limits<unsigned char>::digits *
		dd;

	this->_cl_mem_tile.resize(
		this->_u_tile_he * this->_u_tile_wi *
		this->_u_pixel_size
	);

	this->_u_phot = handle_ref.photometric();

	this->_u_read_bytes_size =
		this->_u_he * this->_u_wi *
		this->_u_pixel_size;
	this->_u_read_bytes_crnt = 0;

	this->_u_current_count = 0;

	this->_u_current_x_image = 0;
	this->_u_current_y_image = 0;

	this->dest_size_ = dest_size;
	this->dest_top_ = dest_top;
}

unsigned int read_tile::count(void) const
{
	return this->_u_tile_num_wi * this->_u_tile_num_he;
}

void read_tile::segment_exec(igs::tif::read_handle& handle_ref)
{
	/* 呼出回数チェック */
	if ((this->_u_tile_num_wi * this->_u_tile_num_he) <=
	this->_u_current_count) {
		std::ostringstream os;
		os	<< "over call TIFFReadTile(,,"
			<< this->_u_current_x_image
			<< ","
			<< this->_u_current_y_image
			<< ",0,0)";
		throw std::domain_error(os.str());
	}
	/* memory overチェック */
	if (this->_u_read_bytes_size <= this->_u_read_bytes_crnt) {
		std::ostringstream os;
		os	<< "over memory<"
			<< this->_u_read_bytes_size
			<< "> TIFFReadTile(,,"
			<< this->_u_current_x_image
			<< ","
			<< this->_u_current_y_image
			<< ",0,0)";
		throw std::domain_error(os.str());
	}

	/* ファイルからタイル読み込み
	tsize_t TIFFReadTile(
		TIFF* tif, tdata_t buf, uint32 x, uint32 y, uint32 z,
		tsample_t sample);
	typedef void* tdata_t;
	*/
	const tsize_t t_ret = TIFFReadTile(
		handle_ref,
		&this->_cl_mem_tile.at(0),
		this->_u_current_x_image,
		this->_u_current_y_image,
		0, (tsample_t)0
	);
	if (t_ret < 0) {
		throw std::domain_error( handle_ref.get_error_msg() );
	}

	/* 必要なら白黒反転 */
	/**************if (PHOTOMETRIC_MINISWHITE == this->_u_phot) {
		igs::image::function::invert(
			this->_u_bi,
			this->_cl_mem_tile.size(),
			&this->_cl_mem_tile.at(0)
		);
	}*************/

	/* タイルを画像にうつす */
	unsigned char *image_array =
		this->dest_top_ +
		this->_u_pixel_size * this->_u_wi *
		this->_u_current_y_image
		+
		this->_u_pixel_size * this->_u_current_x_image;

	unsigned int u_ret;
	if (std::numeric_limits<unsigned char>::digits ==
	static_cast<int>(this->_u_bi)) {
		u_ret = u_trcopy_tile_8bits(
			this->_u_tile_wi,
			this->_u_current_tile_w(),
			this->_u_current_tile_h(),
			this->_u_pixel_size,
			&this->_cl_mem_tile.at(0),
			this->_u_wi,
			image_array
		);
	} else {
		u_ret = u_trcopy_tile_16bits(
			this->_u_tile_wi,
			this->_u_current_tile_w(),
			this->_u_current_tile_h(),
			this->_u_pixel_size,
	igs::resource::pointer_cast<uint16 *>(&this->_cl_mem_tile.at(0)),
			this->_u_wi,
	igs::resource::pointer_cast<uint16 *>(image_array)
		);
	}

	/* 呼出回数カウントアップ */
	++ this->_u_current_count;

	/* 次のタイル位置を計算しとく */
	this->_u_current_x_image =
		(this->_u_current_count%this->_u_tile_num_wi) *
		 this->_u_tile_wi;
	this->_u_current_y_image =
		(this->_u_current_count/this->_u_tile_num_wi) *
		 this->_u_tile_he;

	/* 読み込み済の画像の大きさ */
	this->_u_read_bytes_crnt += u_ret;
}
void read_tile::clear(void)
{ this->_cl_mem_tile.clear(); }
read_tile::~read_tile()
{ try { this->clear(); } catch(...) {} }
//------------------------------------------------------------
