#include <iostream> /* std::cout */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_image_function_invert.h"
#include "igs_tif_error.h"
#include "igs_tif_read_strip.h"

/* strip読み込み */
//------------------------------------------------------------
using igs::tif::read_strip;
/*
 handle_ref.i_strip_size()
	tsize_t TIFFStripSize(TIFF* tif)
	typedef int32 tsize_t;
 handle_ref.u_strip_count()
	tstrip_t TIFFNumberOfStrips(TIFF* tif)
	typedef uint32 tstrip_t;
*/
read_strip::read_strip(
	igs::tif::read_handle& handle_ref,
	unsigned char *dest_top,
	const size_t dest_size
):
	strip_size_(TIFFStripSize(handle_ref)),
	number_of_strip_(TIFFNumberOfStrips(handle_ref)),
	strip_counter_(0),
	dest_size_(dest_size),
	dest_current_(dest_top),
	read_encoded_strip_bytes_(0)
{}

unsigned int read_strip::count(void) const
{ return this->number_of_strip_; }

void read_strip::segment_exec(igs::tif::read_handle& handle_ref) {
	/* 呼出回数チェック */
	if (this->number_of_strip_ <= this->strip_counter_) {
		std::ostringstream os;
		os	<< "over call TIFFReadEncodedStrip(,"
			<< this->strip_counter_
			<< ",p_data,"
			<< this->strip_size_
			<< ")";
		throw std::domain_error(os.str());
	}

	/*
	ファイルから単位(?)読み込み
	tsize_t TIFFReadEncodedStrip(
		TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size);
	typedef uint32 tstrip_t;
	typedef void* tdata_t;
	typedef int32 tsize_t;
	typedef unsigned int uint32;
	typedef int int32;
	--> bufに置かれたデータの実際のバイト数を返す.
	*/
	const tsize_t t_ret = TIFFReadEncodedStrip(
		handle_ref,
		this->strip_counter_,
		this->dest_current_,
		this->strip_size_
	);
	if (t_ret < 0) {
		throw std::domain_error( handle_ref.get_error_msg() );
	}

	/* 呼出回数カウントアップ */
	++ this->strip_counter_;

	/* 次の画像の置き場へ移動 */
	this->dest_current_ += t_ret;

	/* memory overチェック。
	!!!最期のstripのthis->strip_size_がわからないので、
	前もってサイズチェックができない!!! */
	this->read_encoded_strip_bytes_ += t_ret;
	if (this->dest_size_ < this->read_encoded_strip_bytes_) {
		std::ostringstream os;
		os	<< "overd memory<"
			<< this->dest_size_
			<< "> TIFFReadEncodedStrip(,"
			<< this->strip_counter_
			<< ",,"
			<< this->strip_size_
			<< ")";
		throw std::domain_error(os.str());
	}
}
//------------------------------------------------------------
