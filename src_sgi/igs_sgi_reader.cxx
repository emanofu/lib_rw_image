// #include <iomanip> /* std::setw */
// #include <iostream> /* std::cout */
#include <vector> /* std::vector */
#include <sstream> /* std::ostringstream */
#include <limits> /* std::numeric_limits */
#include <stdexcept> /* std::domain_error(-) */

#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_sgi_extensions.h"
#include "igs_sgi_decode_rle.h"
#include "igs_sgi_decode_verbatim.h"
#include "igs_sgi_read_handle.h"
#include "igs_sgi_reader.h"

/* pixelタイプ
画像種類	pixel channels	sampleing bits
---------------+---------------+------------------------------------
RGBA		4		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
RGB		3		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
grayscale	1		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
---------------+---------------+------------------------------------
(※1) by_bi means std::numeric_limits<unsigned char>::digits
*/
/*----------------------------------------------------------*/
using igs::sgi::reader;

/* File書式名 */
const char *const reader::name_ =
	"SGI(Silicon Graphics Image)/RGB File Format"
	;
const char *const reader::name(void) const { return this->name_; }

/* 拡張子数 */
const unsigned int
reader::ext_count(void) {
	return sizeof(igs::sgi::extensions)/sizeof(char *);
}
/* 拡張子名リスト */
const char *const *const
reader::ext_lists(void) {
	return igs::sgi::extensions;
}

/* ファイルヘッダタグ確認 */
const bool
reader::istag(
	const size_t bytes_count
	, const unsigned char *const tag_array
) {
	bool byte_swap_sw;
	return igs::sgi::read_handle::istag(
		bytes_count,tag_array,byte_swap_sw);
}

/* 用意すべき画像と作業情報のサイズを得る */
void
reader::get_size(
	const std::string& file_path
	, size_t& image_bytes
	, size_t& history_bytes
) {
	igs::sgi::read_handle::get_size(
		file_path,image_bytes,history_bytes
	);
}

/* 読む
	正常終了、例外処理throw、(デストラクタ)、で
	内部メモリ解放し、ファイルを閉じること
*/
namespace {
 void header_to_prop(
	igs::sgi::header::parameters *header_params_ptr,
	igs::image::properties &props
 ) {
	props.height  = header_params_ptr->us_ysize;
	props.width   = header_params_ptr->us_xsize;
	props.channels= header_params_ptr->us_zsize;
	props.bits    = header_params_ptr->c_bytes_per_ch *
			std::numeric_limits<unsigned char>::digits;
	if (igs::image::orientation::topleft != props.orie_side) {
		props.orie_side = igs::image::orientation::botleft;
			/* SGI画像ファイル上の原点位置は左下 */
	}
	igs::sgi::read_handle::get_reso(
		header_params_ptr->ca_imagename,
		props.reso_unit,
		props.reso_x,
		props.reso_y
	);
 }
}

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
	std::vector<unsigned char> file_image;
	file_image.resize(igs::resource::bytes_of_file(file_path.c_str()));
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* ファイルそのまま読む */
	{
	 igs::resource::ifbinary cl_fp(file_path.c_str());
	 //cl_fp.read(&file_image.at(0),file_image.size());
	 const size_t fsiz = file_image.size();
	 size_t siz = segment_bytes;
	 for (size_t pos = 0; pos < fsiz; pos += segment_bytes) {
	  if (true == break_sw) { // Cancel
		break_sw = false;
		return;
	  }
	  if ((fsiz-pos) < segment_bytes) { siz=fsiz-pos; } // last
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
	igs::sgi::header::parameters *header_params_ptr=0;
	unsigned long *seek_set_ptr=0;
	unsigned long *bytes_size_ptr=0;
	unsigned char *file_image_ptr=0;
	char *history_ptr=0;
	igs::sgi::read_handle::get_data(
		file_image.size()
		, &file_image.at(0)
		, byte_swap_sw
		, header_params_ptr
		, seek_set_ptr
		, bytes_size_ptr
		, file_image_ptr
		, history_ptr
	);

	/* 読み込める画像かチェック */
	igs::sgi::read_handle::check_parameters(header_params_ptr);

	/* 画像情報設定 */
 	header_to_prop( header_params_ptr, props);

	/* サイズ確認 */
	if (image_bytes < (
		static_cast<size_t>(header_params_ptr->us_ysize)
		* header_params_ptr->us_xsize
		* header_params_ptr->us_zsize
		* header_params_ptr->c_bytes_per_ch
	)) {
		std::ostringstream os;
		os	<< "memory size<"
			<< image_bytes
			<< "> is less than h<"
			<< static_cast<int>(header_params_ptr->us_ysize)
			<< "> x w<"
			<< static_cast<int>(header_params_ptr->us_xsize)
			<< "> x ch<"
			<< static_cast<int>(header_params_ptr->us_zsize)
			<< "> x by<"
			<< static_cast<int>(header_params_ptr->c_bytes_per_ch)
			<< ">"
			;
		throw std::domain_error(os.str());
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* 読み込んだデータをOpenGL型画像データに変換する */
	using namespace igs::sgi::header::compression;
	if (verbatim == header_params_ptr->c_storage) {
		igs::sgi::decode_verbatim decoder(
			file_image_ptr,	// source
			image_array,	// destonation
	static_cast<unsigned int>(header_params_ptr->us_ysize),
	static_cast<unsigned int>(header_params_ptr->us_xsize),
	static_cast<unsigned int>(header_params_ptr->us_zsize),
	static_cast<unsigned int>(header_params_ptr->c_bytes_per_ch)
		);
		bool rvs_sw = false;
		if (igs::image::orientation::topleft==props.orie_side) {
			rvs_sw = true;
		}
		for (int zz = 0; zz < props.channels; ++zz) {
		 for (int yy = 0; yy < props.height; ++yy) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			decoder.segment_exec(yy,zz, rvs_sw);
		 }
		}
	} else {
		igs::sgi::decode_rle decoder(
			&file_image.at(0),
			seek_set_ptr,
			bytes_size_ptr,
			image_array, // destonation
	static_cast<int>(image_bytes),  // destonation
	static_cast<int>(header_params_ptr->us_ysize),
	static_cast<int>(header_params_ptr->us_xsize),
	static_cast<int>(header_params_ptr->us_zsize),
	static_cast<int>(header_params_ptr->c_bytes_per_ch)
		);
		bool rvs_sw = false;
		if (igs::image::orientation::topleft==props.orie_side) {
			rvs_sw = true;
		}
		for (int zz = 0; zz < props.channels; ++zz) {
		 for (int yy = 0; yy < props.height; ++yy) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			decoder.segment_exec(yy,zz, rvs_sw);
		 }
		}
	}
	if (0 != history_ptr) {
		history_str += history_ptr;
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif
}
