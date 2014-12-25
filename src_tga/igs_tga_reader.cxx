#include <vector> /* std::vector */
// #include <iomanip> /* std::setw */
// #include <iostream> /* std::cout */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error(-) */
#include <limits> /* std::numeric_limits */

#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_tga_extensions.h"
#include "igs_tga_decode.h"
#include "igs_tga_read_handle.h"
#include "igs_tga_reader.h"

using igs::tga::reader;

/* File書式名 */
const char *const reader::name_=
	"TGA(Truevision(Advanced Raster) Graphics Adapter) File Format"
	;
const char *const reader::name(void) const { return this->name_; }

/* 拡張子数 */
const unsigned int reader::ext_count(void) {
	return sizeof(igs::tga::extensions)/sizeof(char *);
}
/* 拡張子名リスト */
const char *const *const reader::ext_lists(void) {
	return igs::tga::extensions;
}

/* ファイルヘッダタグ確認 */
const bool reader::istag(
	const size_t bytes_count
	,const unsigned char *const tag_array
) {
	bool byte_swap_sw;
	return igs::tga::read_handle::istag(
		bytes_count,tag_array,byte_swap_sw
	);
}

/* 用意すべき画像と作業情報のサイズを得る */
void reader::get_size(
	const std::string& file_path
	, size_t& image_bytes
	, size_t& history_bytes
) {
	unsigned int imagebytes=0;
	unsigned int historybytes=0;
	igs::tga::read_handle::get_size(
		file_path,imagebytes,historybytes
	);
	image_bytes = imagebytes;
	history_bytes = historybytes;
}

/* 読む
	正常終了、例外処理throw、(デストラクタ)、で
	内部メモリ解放し、ファイルを閉じる
*/
#if defined TES_SPEED
#include "tes_rs.h"
#endif

void
reader::get_data(
	const std::string& file_path
	, igs::image::properties& props
	, const size_t image_bytes
	, unsigned char *image_array
	, std::string& history_str
	, bool& break_sw /* 中断Switch。外からtureで中断 */
	, const size_t segment_bytes
			/* 分割読込の個々サイズ(tif以外で使用する) */
) {
#if defined TES_SPEED
igs::test::speed::start();
#endif

	/* ファイルイメージ用メモリ確保 */
	std::vector<unsigned char> file_image(
		igs::resource::bytes_of_file(file_path.c_str())
	);

#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* ファイルそのまま読む */
    {
	igs::resource::ifbinary cl_fp(file_path.c_str());
	// cl_fp.read(&file_image.at(0),file_image.size());
	const size_t fsiz = file_image.size();
	size_t siz = segment_bytes;
	for (size_t pos = 0; pos < fsiz; pos += segment_bytes) {
	  if (true == break_sw) { // Cancel
		break_sw = false;
		return;
	  }
	  if ((fsiz-pos) < segment_bytes) {siz=fsiz-pos;} // last
	  cl_fp.read(&file_image.at(pos),siz);
	}
	cl_fp.close();
    }
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* 情報変換する */
	bool byte_swap_sw;
	igs::tga::header::parameters param;
	unsigned char	*fi_ptr=0;
	size_t		fi_byt=0;
	unsigned char	*fi_cmap_ptr=0;
	igs::tga::read_handle::get_data(
		file_image.size()
		, &file_image.at(0)
		, byte_swap_sw
		, param
		, fi_ptr
		, fi_byt
		, fi_cmap_ptr
	);

	/* 読み込める画像かチェック */
	igs::tga::read_handle::check_parameters(param);

	/* 画像情報設定 */
	props.height  = param.height;
	props.width   = param.width;
	props.channels = igs::tga::read_handle::dest_channels(
		  param.bits_par_pixel
		, param.cmap_bits_depth
	);
	props.bits = std::numeric_limits<unsigned char>::digits;
	if ((int)param.descriptor & 0x20) {
	 if ((int)param.descriptor & 0x10) {
		props.orie_side = igs::image::orientation::toprigh;
	 } else {
		props.orie_side = igs::image::orientation::topleft;
	 }
	} else {
	 if ((int)param.descriptor & 0x10) {
		props.orie_side = igs::image::orientation::botrigh;
	 } else {
		props.orie_side = igs::image::orientation::botleft;
			/* TGA画像ファイル上の原点位置(default)は左下 */
	 }
	}
	props.reso_unit = igs::image::resolution::not_defined;
	props.reso_x = 0;
	props.reso_y = 0;

	/* サイズ確認 */
	if (image_bytes < (static_cast<size_t>(param.height)
		* param.width
		* props.channels
		* 1
	)) {
		std::ostringstream os;
		os	<< "memory size<"
			<< image_bytes
			<< "> is less than h<"
			<< param.height
			<< "> x w<"
			<< param.width
			<< "> x ch<"
			<< props.channels
			<< "> x by<"
			<< 1
			<< '>'
			;
		throw std::domain_error(os.str());
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
    {
	igs::tga::decode::pixel dcdr(
		fi_cmap_ptr
		, param.cmap_index_of_entry
		, param.cmap_count_of_entries
		, param.cmap_bits_depth
		, byte_swap_sw
	);

	igs::tga::pixel::bgra32	  *t32 = igs::resource::pointer_cast<
	igs::tga::pixel::bgra32 *>(fi_ptr);
	igs::tga::pixel::bgr24	  *t24 = igs::resource::pointer_cast<
	igs::tga::pixel::bgr24 *>(fi_ptr);
	igs::tga::pixel::argb1555 *t15 = igs::resource::pointer_cast<
	igs::tga::pixel::argb1555 *>(fi_ptr);
	igs::tga::pixel::argb0555 *t05 = igs::resource::pointer_cast<
	igs::tga::pixel::argb0555 *>(fi_ptr);
	igs::tga::pixel::cmap8    *tm8 = igs::resource::pointer_cast<
	igs::tga::pixel::cmap8 *>(fi_ptr);
	igs::tga::pixel::bw8      *tg8 = igs::resource::pointer_cast<
	igs::tga::pixel::bw8 *>(fi_ptr);

	igs::image::pixel::rgba32 *b32 = igs::resource::pointer_cast<
	igs::image::pixel::rgba32 *>(image_array);
	igs::image::pixel::rgb24  *b24 = igs::resource::pointer_cast<
	igs::image::pixel::rgb24 *>(image_array);
	igs::image::pixel::gray8  *gr8 = igs::resource::pointer_cast<
	igs::image::pixel::gray8 *>(image_array);

	using igs::tga::decode::uncompressed;
	using igs::tga::decode::run_length;

	const size_t iby = image_bytes;

	switch (param.image_type) {
	case  0: /* no image data included */
		break;
	case  1: /* uncompressed, color_mapped image */
	  switch (param.cmap_bits_depth) {
	  case 32:
	  case 16: uncompressed(tm8,fi_byt,b32,iby,dcdr,break_sw);break;
	  case 24:
	  case 15: uncompressed(tm8,fi_byt,b24,iby,dcdr,break_sw);break;
	  }
		break;
	case  2: /* uncompressed, true_color image */
	  switch (param.bits_par_pixel) {
	  case 32: uncompressed(t32,fi_byt,b32,iby,dcdr,break_sw);break;
	  case 24: uncompressed(t24,fi_byt,b24,iby,dcdr,break_sw);break;
	  case 16: uncompressed(t15,fi_byt,b32,iby,dcdr,break_sw);break;
	  case 15: uncompressed(t05,fi_byt,b24,iby,dcdr,break_sw);break;
	  }
		break;
	case  3: /* uncompressed, black and white image */
		   uncompressed(tg8,fi_byt,gr8,iby,dcdr,break_sw);break;
	case  9: /* run_length encoded color_mapped image */
	  switch (param.cmap_bits_depth) {
	  case 32:
	  case 16: run_length(tm8,fi_byt,b32,iby,dcdr,break_sw); break;
	  case 24:
	  case 15: run_length(tm8,fi_byt,b24,iby,dcdr,break_sw); break;
	  }
		break;
	case 10: /* run_length encoded true_color image */
	  switch (param.bits_par_pixel) {
	  case 32: run_length(t32,fi_byt,b32,iby,dcdr,break_sw); break;
	  case 24: run_length(t24,fi_byt,b24,iby,dcdr,break_sw); break;
	  case 16: run_length(t15,fi_byt,b32,iby,dcdr,break_sw); break;
	  case 15: run_length(t05,fi_byt,b24,iby,dcdr,break_sw); break;
	  }
		break;
	case 11: /* run_length encoded black and white image */
		   run_length(tg8,fi_byt,gr8,iby,dcdr,break_sw); break;
	}
    }
	file_image.clear();
  
#if defined TES_SPEED
igs::test::speed::stop();
#endif
}
