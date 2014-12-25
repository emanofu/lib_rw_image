#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include <iomanip> /* std::setprecision() */
#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_tga_extensions.h"
#include "igs_tga_header.h"
#include "igs_tga_encode.h"
#include "igs_tga_writer.h"

/* -------------------------------------------------------- */

using igs::tga::writer;
writer::writer()
{}

/* File��̾ */
const char *const writer::name_=
	"TGA(Truevision(Advanced Raster) Graphics Adapter) File Format"
	;
const char *const writer::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
writer::ext_count(void) {
	return sizeof(igs::tga::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
writer::ext_lists(void) {
	return igs::tga::extensions;
}

/* ����̾�ꥹ�ȼ¥ǡ��� */
namespace igs {
 namespace tga { /* igs::tga::header::compression�ȹ�碌�뤳�� */
  const char *const compressions[] = {
		"Uncompressed"
		, "RLE"
  };
 }
}
/* ���̼���� */
const unsigned int
writer::compression_count(void) {
	return sizeof(igs::tga::compressions)/sizeof(char *);
}
/* ����̾�ꥹ�� */
const char *const *const
writer::compression_lists(void) {
	return igs::tga::compressions;
}

/* ��¸���Բĥ����å� */
/* .rgb/.tga  RGBA/RGB/Grayscale  8/16bits */
const bool
writer::error_from_properties(
	const igs::image::properties& props
	, const int ext_number
	, std::ostringstream& error_or_warning_msg
) {
	bool error_sw = false;

	if ( igs::tga::extension::tga != static_cast<
	     igs::tga::extension::type>(ext_number)
	) {
		error_or_warning_msg
			<< "Error : extension number(" << ext_number
			<< ") Must be "
			<< static_cast<int>(igs::tga::extension::tga)
			<< "\n";
		error_sw = true;
	}

	if (0 == props.width) {
		error_or_warning_msg
			<< "Error : width(0) Must Not be zero\n";
		error_sw = true;
	}
	if (0 == props.height) {
		error_or_warning_msg
			<< "Error : height(0) Must Not be zero\n";
		error_sw = true;
	}
	if (	(1 != props.channels) &&
		(3 != props.channels) &&
		(4 != props.channels)
	) {
		error_or_warning_msg
			<< "Error : channel(" << props.channels
			<< ") Must be 1 or 3 or 4\n";
		error_sw = true;
	}
	if (	static_cast<int>(props.bits) !=
		std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char)
	) {
		error_or_warning_msg
			<< "Error : sampling bits("
			<< props.bits
			<< ") Must be "
		<< std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char)
			<< "\n";
		error_sw = true;
	}
	/* �����θ���(botleft��topleft�Τ�) */
	if (	(igs::image::orientation::botleft != props.orie_side) &&
		(igs::image::orientation::topleft != props.orie_side)
	) {
		error_or_warning_msg
			<< "Error : orientaion(" << props.orie_side
			<< ") Must be botleft("
		<< static_cast<int>(igs::image::orientation::botleft)
			<< ") or topleft("
		<< static_cast<int>(igs::image::orientation::topleft)
			<< ")\n";
		error_sw = true;
	}
	/* ������ */
	switch (props.reso_unit) {
	case igs::image::resolution::not_defined:
		break;
	case igs::image::resolution::centimeter:
	case igs::image::resolution::millimeter:
	case igs::image::resolution::meter:
	case igs::image::resolution::inch:
	case igs::image::resolution::nothing:
		error_or_warning_msg
			<< "Warning : resolution will not save\n";
		break;
	}
	return error_sw;
}

namespace {
 void param_to_rw(
	const igs::tga::header::parameters &param
	, igs::tga::header::parameters_rw &pa_rw
 ) {
	pa_rw.id_length		= param.id_length;
	pa_rw.cmap_type		= param.cmap_type;
	pa_rw.image_type	= param.image_type;

	pa_rw.cmap_index_of_entry1 = static_cast<unsigned char>(
	(static_cast<int>(param.cmap_index_of_entry) >> 8) & 0xff);
	pa_rw.cmap_index_of_entry0 = static_cast<unsigned char>(
	(static_cast<int>(param.cmap_index_of_entry)) & 0xff);

	pa_rw.cmap_count_of_entries1 = static_cast<unsigned char>(
	(static_cast<int>(param.cmap_count_of_entries) >> 8) & 0xff);
	pa_rw.cmap_count_of_entries0 = static_cast<unsigned char>(
	(static_cast<int>(param.cmap_count_of_entries)) & 0xff);


	pa_rw.cmap_bits_depth	= param.cmap_bits_depth;
	pa_rw.left_term_pos	= param.left_term_pos;
	pa_rw.lower_term_pos	= param.lower_term_pos;
	pa_rw.width		= param.width;
	pa_rw.height		= param.height;
	pa_rw.bits_par_pixel	= param.bits_par_pixel;
	pa_rw.descriptor	= param.descriptor;
 }
}

/* ��¸
���ｪλ�⡢�㳰������catch�ΤȤ��⡢
����������������ե�������Ĥ��뤳��
*/
void
writer::put_data(
	const igs::image::properties& props
	, const size_t image_bytes
	, const unsigned char *image_array
	, const int compression_number
	, const std::string& history_str
	, const std::string& file_path
) {
	/* --- �إå����� --------------------------------------*/
	/* ����Τʤ���ΤϽ�����ˤ�ꥼ���ͤȤʤ� */
	igs::tga::header::parameters param;

	if (igs::tga::header::compression::rle == compression_number) {
		switch (props.channels) {
		case 4: param.image_type = 10;
			param.descriptor = 8;
			break;
		case 3: param.image_type = 10;
			break;
		case 1: param.image_type = 11;
			break;
		}
	}
	else {
		switch (props.channels) {
		case 4: param.image_type = 2;
			param.descriptor = 8;
			break;
		case 3: param.image_type = 2;
			break;
		case 1: param.image_type = 3;
			break;
		}
	}
	param.bits_par_pixel =
			std::numeric_limits<unsigned char>::digits *
			props.channels;

	if (igs::image::orientation::topleft == props.orie_side) {
		param.descriptor |= 0x20;
	}

	/* �������礭�� */
	param.width  = static_cast<short>(props.width);
	param.height = static_cast<short>(props.height);

	/* --- �إå���Х��ȥ���åפ��� ----------------------*/
	/* file��¸Endian��little�˸���,cpu��big�λ�swap���ʤ�����¸ */
	const bool byte_swap_sw =
		(false == igs::resource::cpu_is_little_endian());

	igs::tga::header::parameters pa_sa = param;
	if ( byte_swap_sw ) {
	 igs::resource::swap_endian( pa_sa.cmap_index_of_entry );
	 igs::resource::swap_endian( pa_sa.cmap_count_of_entries);
	 igs::resource::swap_endian( pa_sa.left_term_pos );
	 igs::resource::swap_endian( pa_sa.lower_term_pos );
	 igs::resource::swap_endian( pa_sa.width );
	 igs::resource::swap_endian( pa_sa.height );
	}
	igs::tga::header::parameters_rw pa_rw;
	param_to_rw(pa_sa,pa_rw);

	/* --- �ե����볫�� ------------------------------------*/
 	std::ofstream ofs(file_path.c_str(), std::ios::binary);
	if (!ofs) {
		std::string str("file<"); str += file_path;
		str += "> open<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* --- �إå���¸ --------------------------------------*/
	ofs.write(
		igs::resource::pointer_cast<char *>( &pa_rw ),
		sizeof(igs::tga::header::parameters_rw)
	);
	if (!ofs) {
		std::string str("file<"); str += file_path;
		str += "> write<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* --- ID��¸ ------------------------------------------*/
	/* Nothing */

	/* --- Cmap��¸ ----------------------------------------*/
	/* Nothing */

	/* --- ������¸ ----------------------------------------*/

	const igs::image::pixel::rgba32 *b32 =
		igs::resource::const_pointer_cast<
			const igs::image::pixel::rgba32 *>(image_array);
	const igs::image::pixel::rgb24 *b24 =
		igs::resource::const_pointer_cast<
			const igs::image::pixel::rgb24 *>(image_array);
	const igs::image::pixel::gray8 *gr8 =
		igs::resource::const_pointer_cast<
			const igs::image::pixel::gray8 *>(image_array);

	igs::tga::pixel::bgra32 t32;
	igs::tga::pixel::bgr24 t24;
	igs::tga::pixel::bw8 tg8;

	using igs::tga::encode::run_length;
	using igs::tga::encode::uncompressed;

	igs::tga::encode::pixel edr(byte_swap_sw);

	const size_t iby = image_bytes;

	if (igs::tga::header::compression::rle == compression_number) {
	 switch (props.channels) {
	 case 4: run_length(b32,iby,t32,edr,file_path,ofs); break;
	 case 3: run_length(b24,iby,t24,edr,file_path,ofs); break;
	 case 1: run_length(gr8,iby,tg8,edr,file_path,ofs); break;
	 }
	}
	else {
	 switch (props.channels) {
	 case 4: uncompressed(b32,iby,t32,edr,file_path,ofs); break;
	 case 3: uncompressed(b24,iby,t24,edr,file_path,ofs); break;
	 case 1: uncompressed(gr8,iby,tg8,edr,file_path,ofs); break;
	 }
	}

	/* --- history_str�������¸ -------------*/
	/* nothing */

	/* --- �ե������Ĥ��� ----------------------------------*/
	ofs.close();
	if (!ofs) {
		std::string str("file<"); str += file_path;
		str += "> close<std::ofstream> error";
		throw std::domain_error(str);
	}
}
