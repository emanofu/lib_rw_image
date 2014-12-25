#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_tga_read_handle.h"

const bool
igs::tga::read_handle::istag(
	  const unsigned int bytes_count
	, const unsigned char *const tag_array
	, bool& byte_swap_sw
) {
	if (bytes_count < 3) { return false; }

	/* tgaファイルは"Little Endian Order固定"として読み込む */
	byte_swap_sw = (false == igs::resource::cpu_is_little_endian());

	if ((	   (0 == tag_array[1])
		|| (1 == tag_array[1])
	    ) && (
		   (0 == tag_array[2])
		|| (1 == tag_array[2])
		|| (2 == tag_array[2])
		|| (3 == tag_array[2])
		|| (9 == tag_array[2])
		|| (10 == tag_array[2])
		|| (11 == tag_array[2])
	)) {
		return true; /* OK */
	}
	return false; /* Error */
}
namespace {
 /* ファイル書式が違っていればthrow */
 void check_tag(
	const unsigned int bytes_count
	, const unsigned char *const tag_array
	, bool& byte_swap_sw
 ) {
	if (igs::tga::read_handle::istag(
		bytes_count, tag_array, byte_swap_sw)) { return; }

	std::ostringstream os;
	  os	<< "file head(";
	for (unsigned int ii=0; ii < bytes_count; ++ii) {
	 if (0 < ii) {
	  os	<< ' ';
	 }
	  os	<< (int)tag_array[ii];
	}
	  os
		<< ")"
		<< "is not tga image"
		<< std::dec;
	throw std::domain_error(os.str());
 }
 void byte_swap_header(igs::tga::header::parameters& param) {
	igs::resource::swap_endian( param.cmap_index_of_entry );
	igs::resource::swap_endian( param.cmap_count_of_entries );
	igs::resource::swap_endian( param.left_term_pos );
	igs::resource::swap_endian( param.lower_term_pos );
	igs::resource::swap_endian( param.width );
	igs::resource::swap_endian( param.height );
 }
 void rw_to_param(
	const igs::tga::header::parameters_rw& pa_rw
	,     igs::tga::header::parameters& param
 ) {
	param.id_length		= pa_rw.id_length;
	param.cmap_type		= pa_rw.cmap_type;
	param.image_type	= pa_rw.image_type;

	param.cmap_index_of_entry = static_cast<short>(
		(static_cast<int>(pa_rw.cmap_index_of_entry1) << 8) |
		 static_cast<int>(pa_rw.cmap_index_of_entry0) );
	param.cmap_count_of_entries = static_cast<short>(
		(static_cast<int>(pa_rw.cmap_count_of_entries1) << 8) |
		 static_cast<int>(pa_rw.cmap_count_of_entries0) );

	param.cmap_bits_depth	= pa_rw.cmap_bits_depth;
	param.left_term_pos	= pa_rw.left_term_pos;
	param.lower_term_pos	= pa_rw.lower_term_pos;
	param.width		= pa_rw.width;
	param.height		= pa_rw.height;
	param.bits_par_pixel	= pa_rw.bits_par_pixel;
	param.descriptor	= pa_rw.descriptor;
 }
}
/* tgaファイルから、可能なすべてのtgaファイル情報を得る */
void
igs::tga::read_handle::get_info(
	const std::string& file_path
	, bool& byte_swap_sw
	, igs::tga::header::parameters& param
) {
    {
	/* ファイル開く */
	igs::resource::ifbinary ifb(file_path.c_str());
      {
	/* ファイルからヘッダ部のみ読む */
	igs::tga::header::parameters_rw pa_rw;
	ifb.read(
		igs::resource::pointer_cast<unsigned char *>(&pa_rw)
		,sizeof(igs::tga::header::parameters_rw)
	);
 	rw_to_param(pa_rw,param);
      }
	/* ファイル閉じる */
	ifb.close();
    }

	/* 始めの3bytes(1番目でなく、2,3番目byteで判断)から、
	ファイル書式確認と、byte swapするかどうか */
	check_tag(
		sizeof(igs::tga::header::parameters)
		, igs::resource::pointer_cast<unsigned char *>(
			&param
		)
		, byte_swap_sw
	);

	/* fileでのendianとOnMemoryのendianが逆のときswapする */
	if (byte_swap_sw) { byte_swap_header(param); }
}

int
igs::tga::read_handle::bytes_from_bits(
	const int bits_par_pixel
) {
	switch (bits_par_pixel) {
	case 32: return 4; break;
	case 24: return 3; break;
	case 16: return 2; break;
	case 15: return 2; break;
	case  8: return 1; break;
	}
	return 0;
}
int
igs::tga::read_handle::dest_channels(
	const int bits_par_pixel
	, const int cmap_bits_depth
) {
	switch (bits_par_pixel) {
	case 32: return 4; break;
	case 24: return 3; break;
	case 16: return 4; break;
	case 15: return 3; break;
	case  8:
		if      (32 == cmap_bits_depth) { return 4; }
		else if (24 == cmap_bits_depth) { return 3; }
		else if (16 == cmap_bits_depth) { return 4; }
		else if (15 == cmap_bits_depth) { return 3; }
		break;
	}
	return 0;
}
void
igs::tga::read_handle::get_size(
	const std::string& file_path
	, unsigned int& image_size
	, unsigned int& history_size
) {
	bool byte_swap_sw = false;
	igs::tga::header::parameters param;
	igs::tga::read_handle::get_info(
		file_path, byte_swap_sw, param
	);

	/* 画像サイズ */
	image_size =
		param.width *
		param.height *
		igs::tga::read_handle::dest_channels(
			param.bits_par_pixel
			,param.cmap_bits_depth
		);

	/* historyサイズを得る。文字列の終端'\0'を含む */
	history_size = 0;
}

void
igs::tga::read_handle::check_parameters(
	igs::tga::header::parameters& param
) {
	/* ColorMap field */
	switch (param.cmap_type) {
	case 0: case 1: break;
	default:
		std::ostringstream os;
		os	<< "bad cmap_type("
			<< (int)param.cmap_type
			<< ')';
		throw std::domain_error(os.str());
	}

	/* 画像 field */
	switch (param.image_type) {
	case 0: case 1: case 2: case 3: case 9: case 10: case 11: break;
	default:
		std::ostringstream os;
		os	<< "bad image_type("
			<< (int)param.image_type
			<< ')';
		throw std::domain_error(os.str());
	}

	/* 画像 fieldと、bits depthの組み合わせ */
	if ((1 == param.image_type) ||	/* ColorMap(256色)(無圧縮) */
	    (9 == param.image_type)) {	/* ColorMap(256色)(RLE圧縮) */
		if (8 != param.bits_par_pixel) {
			std::ostringstream os;
			os	<< "Imagetype is cmap but,"
					"bad bits_par_pixel("
				<< (int)param.bits_par_pixel
				<< ')';
			throw std::domain_error(os.str());
		}
	} else
	if ((2  == param.image_type) ||	/* FullColor(BGRA/BGR)(無縮) */
	    (10 == param.image_type)) {	/* FullColor(BGRA/BGR)(RLE縮) */
		if ((32 != param.bits_par_pixel) &&
		    (24 != param.bits_par_pixel) &&
		    (16 != param.bits_par_pixel) &&
		    (15 != param.bits_par_pixel)) {
			std::ostringstream os;
			os	<< "Imagetype is fullcolor but,"
					"bad bits_par_pixel("
				<< (int)param.bits_par_pixel
				<< ')';
			throw std::domain_error(os.str());
		}
	} else
	if ((3  == param.image_type) ||	/* Grayscale(白黒)(無圧縮)  */
	    (11 == param.image_type)) {	/* Grayscale(白黒)(RLE圧縮) */
		if (8 != param.bits_par_pixel) {
			std::ostringstream os;
			os	<< "Imagetype is grayscale but,"
					"bad bits_par_pixel("
				<< (int)param.bits_par_pixel
				<< ')';
			throw std::domain_error(os.str());
		}
	}

	/* Color Map Depth */
	switch (param.cmap_bits_depth) {
	case 0: case 15: case 16: case 24: case 32: break;
	default:
		std::ostringstream os;
		os	<< "bad cmap_bits_depth("
			<< (int)param.cmap_bits_depth << ")";
		throw std::domain_error(os.str());
	}

	/* 圧縮モード */
	switch (param.bits_par_pixel) {
	case 8: case 15: case 16: case 24: case 32: break;
	default:
		std::ostringstream os;
		os	<< "bad bits_par_pixel("
			<< (int)param.bits_par_pixel << ")";
		throw std::domain_error(os.str());
	}

	/* 画像属性の内、各Pixelに関係する属性のBits数 */
	int     alpha_bits_count = param.descriptor & 0x0f;
	switch (alpha_bits_count) {
	case 0: case 1: case 8: break;
	default:
		std::ostringstream os;
		os	<< "bad alpha_bits_count of descriptor("
			<< alpha_bits_count << ")";
		throw std::domain_error(os.str());
	}
	/* ColorMap画像は、Pixelは8のみ */
	if (0 < param.cmap_type) {
		if (8 != param.bits_par_pixel) {
			std::ostringstream os;
			os	<< "Colormap but,bad bits_par_pixel("
				<< (int)param.bits_par_pixel
				<< ')';
			throw std::domain_error(os.str());
		}
	/* ColorMap画像は、cmap必須 */
		if (0 == param.cmap_count_of_entries) {
			throw std::domain_error(
			 "Colormap but cmap_count_of_entries is zero"
			);
		}
		if (0 == param.cmap_bits_depth) {
			std::ostringstream os;
			os	<< "Colormap but, bad cmap_bits_depth("
				<<  (int)param.cmap_bits_depth
				<< ')';
			throw std::domain_error(os.str());
		}
	}
}

void
igs::tga::read_handle::get_data(
	const size_t file_size
	, unsigned char* file_image
	, bool& byte_swap_sw
	, igs::tga::header::parameters& param_ref
	, unsigned char*& image_ptr_ref
	, size_t& image_size_ref
	, unsigned char*& cmap_ptr_ref
) {
	/* 始めの3bytesからファイル書式確認と、byte swapするかどうか */
	check_tag( 3, file_image, byte_swap_sw );

	igs::tga::header::parameters_rw *pa_rw_ptr;
	/* ファイルイメージからヘッダ部をポインタへ */
	pa_rw_ptr = igs::resource::pointer_cast<
		igs::tga::header::parameters_rw *>(file_image);
 	rw_to_param(*pa_rw_ptr,param_ref);

	/* fileでのendianとOnMemoryのendianが逆のときswapする */
	if (byte_swap_sw) { byte_swap_header(param_ref); }

	size_t cmap_bytes = 0;
	cmap_ptr_ref = 0;
	if (0 < param_ref.cmap_type) {
		cmap_ptr_ref = file_image +
			sizeof(igs::tga::header::parameters_rw) +
			param_ref.id_length;
		cmap_bytes +=
			param_ref.cmap_count_of_entries *
			igs::tga::read_handle::bytes_from_bits(
				param_ref.cmap_bits_depth
			);
	}

	image_ptr_ref = file_image +
		sizeof(igs::tga::header::parameters_rw) +
		param_ref.id_length +
		cmap_bytes
		;
	image_size_ref = file_size -
		sizeof(igs::tga::header::parameters_rw) -
		param_ref.id_length -
		cmap_bytes
		;
}
