#include <vector> /* std::vector */
#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include <iomanip> /* std::setprecision() */
#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_sgi_extensions.h"
#include "igs_sgi_header.h"
#include "igs_sgi_rle_write.h"
#include "igs_sgi_verbatim_write.h"
#include "igs_sgi_writer.h"

/* -------------------------------------------------------- */
// using igs::sgi::write::properties;

using igs::sgi::writer;
writer::writer()
{}

/* File��̾ */
const char *const writer::name_ =
	"SGI(Silicon Graphics Image)/RGB File Format"
	;
const char *const writer::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
writer::ext_count(void) {
	return sizeof(igs::sgi::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
writer::ext_lists(void) {
	return igs::sgi::extensions;
}

/* ����̾�ꥹ�ȼ¥ǡ��� */
namespace igs {
 namespace sgi { /* igs::sgi::header::compression�ȹ�碌�뤳�� */
  const char *const compressions[] = {
		"Verbatim",
		"RLE",
  };
 }
}
/* ���̼���� */
const unsigned int
writer::compression_count(void) {
	return sizeof(igs::sgi::compressions)/sizeof(char *);
}
/* ����̾�ꥹ�� */
const char *const *const
writer::compression_lists(void) {
	return igs::sgi::compressions;
}

/* ��¸
���ｪλ���㳰������catch����
����������������ե�������Ĥ��뤳��
*/
namespace {
 template<class T> void _image2minmax(
	const T* pixel,
	const unsigned int ys,
	const unsigned int xs,
	const unsigned int zs,
	long& l_min,
	long& l_max
 )
 {
	l_min = 0xffff;
	l_max = 0;
	for (unsigned int zz=0; zz<zs; ++zz) {
	for (unsigned int yy=0; yy<ys; ++yy) {
	for (unsigned int xx=0; xx<xs; ++xx) {
		if (*pixel < l_min) { l_min = *pixel; }
		if (l_max < *pixel) { l_max = *pixel; }
		++pixel;
	}
	}
	}
 }
}

/* ��¸���Բĥ����å� */
/* .rgb/.sgi  RGBA/RGB/Grayscale  8/16bits */
const bool
writer::error_from_properties(
	const igs::image::properties& props,
	const int ext_number,
	std::ostringstream& error_or_warning_msg
) {
	bool error_sw = false;

	if ((igs::sgi::extension::rgb != static_cast<
	     igs::sgi::extension::type>(ext_number)) && (
	    (igs::sgi::extension::sgi != static_cast<
	     igs::sgi::extension::type>(ext_number))
	    )
	) {
		error_or_warning_msg
			<< "Error : extension number(" << ext_number
			<< ") Must be "
			<< static_cast<int>(igs::sgi::extension::rgb)
			<< " or "
			<< static_cast<int>(igs::sgi::extension::sgi)
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
	if ((
		static_cast<int>(props.bits) !=
		std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char)
	) && (
		static_cast<int>(props.bits) !=
		std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned short)
	)) {
		error_or_warning_msg
			<< "Error : sampling bits("
			<< props.bits
			<< ") Must be "
		<< std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char)
			<< " or "
		<< std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned short)
			<< "\n";
		error_sw = true;
	}
	/* �����θ���(botleft�ξ�����¸���岼ȿž������¸����) */
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

void
writer::put_data(
	const igs::image::properties& props
	, const size_t image_bytes
	, const unsigned char *image_array
	, const int compression_number
	, const std::string& history_str
	, const std::string& file_path
) {
	/* �إå����� --------------------------------------*/
	igs::sgi::header::parameters head_params;
	/* ����̾����� */
	size_t ii;
	for (ii=0;ii<sizeof(head_params.ca_imagename);++ii){
		head_params.ca_imagename[ii]=0;
	}
	/* ����̾�˲�������¸�Τ�����ȼ���ĥ 2008-03-08 */
	if ((
		igs::image::resolution::not_defined !=
		props.reso_unit
	) && (
		igs::image::resolution::nothing !=
		props.reso_unit
	)) {
		std::ostringstream os;
		using namespace igs::image::resolution;
		using namespace igs::sgi::header::resolution;
		switch (props.reso_unit) { /* ɽ�������� */
		case not_defined:
		case nothing:				break;
		case inch:	 os << ca2_inch;	break;
		case centimeter: os << ca2_centimeter;	break;
		case millimeter: os << ca2_millimeter;	break;
		case meter:	 os << ca2_meter;	break;
		}
		os
			// << std::setprecision(16)
			<< std::setprecision(
				std::numeric_limits<double>::digits10)
					/* double�κ�������(15) */
			<< ' ' << props.reso_x
			<< ' ' << props.reso_y
			<< std::setprecision(6); /* default���᤹ */
		std::string str = os.str();
		size_t u_len=(str.length()<80)? str.length():80-1;
		for (ii=0; ii < u_len; ++ii) {
			head_params.ca_imagename[ii] = str[ii];
		}
	}
	/* SGI�ե�����إå� */
	head_params.s_magic = igs::sgi::header::tag::s_magic;

	/* ��¸���β������̥⡼�� (0/1) */
	head_params.c_storage = static_cast<char>(compression_number);

	/* ��channel��byte�� (1/2) */
	head_params.c_bytes_per_ch =
			static_cast<char>(props.bits) /
			std::numeric_limits<unsigned char>::digits;

	/* �������礭�� */
	head_params.us_xsize =
		static_cast<unsigned short>(props.width);
	head_params.us_ysize =
		static_cast<unsigned short>(props.height);
	head_params.us_zsize =
		static_cast<unsigned short>(props.channels);

	if      (1 < props.channels) {
		head_params.us_dimension = 3; }
	else if (1 < props.height) {
		head_params.us_dimension = 2; }
	else {	head_params.us_dimension = 1; }

	/* pixel�κǾ��ͤȺ����� */
	if (2 == head_params.c_bytes_per_ch) {
		_image2minmax(
			igs::resource::const_pointer_cast<const unsigned short *>(
				image_array
			),
			static_cast<unsigned int>(head_params.us_ysize),
			static_cast<unsigned int>(head_params.us_xsize),
			static_cast<unsigned int>(head_params.us_zsize),
			head_params.l_pixmin,
			head_params.l_pixmax);
	} else {
		_image2minmax(
			image_array,
			static_cast<unsigned int>(head_params.us_ysize),
			static_cast<unsigned int>(head_params.us_xsize),
			static_cast<unsigned int>(head_params.us_zsize),
			head_params.l_pixmin,
			head_params.l_pixmax);
	}
	/* dummy */
	head_params.l_dummy = 0L;
	/* �����μ���򼨤� */
	head_params.l_colormap = static_cast<long>(
		igs::sgi::header::form::normal
	);
	/* dummy */
	for (ii=0;ii<sizeof(head_params.ca_dummy);++ii){
		head_params.ca_dummy[ii]=0;
	}

	/* CPU����ǥ�����
	intel(x86)��ǡ����Τޤ���¸������� byte-swapped�β����ˤʤ�
	mips (68k)��ǡ����Τޤ���¸������� byte-ordered�β����ˤʤ�
	intel(x86)(little endian)��CPU�Ǥϡ�byte swap������¸����
	----------------------------------------------------*/
	union { unsigned short us; unsigned char uc[2]; } buf;
	buf.us = 1;
	/*
	CPU Byte Order������
	Big Endian CPU�Ǥ�buf.us��1������buf.uc[1]��1�Ȥʤ�
			+---------------+
	unsigned short	|      us       |
			|           1   |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   0   |   1   |
			+-------+-------+
	Little Endian CPU�Ǥ�buf.us��1������buf.uc[0]��1�Ȥʤ�
			+---------------+
	unsigned short	|      us       |
			|   1           |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   1   |   0   |
			+-------+-------+
	*/
	bool byte_swap_sw = false;
	if (1 == buf.uc[0]) { /* Little Endian CPU */
		byte_swap_sw = true; /* then save with swapping */
	}

	/* �إå���Х��ȥ���åפ��� ----------------------*/
	igs::sgi::header::parameters save_params = head_params;
	if ( byte_swap_sw ) {
		igs::resource::swap_endian( save_params.s_magic );
		igs::resource::swap_endian( save_params.us_dimension );
		igs::resource::swap_endian( save_params.us_xsize );
		igs::resource::swap_endian( save_params.us_ysize );
		igs::resource::swap_endian( save_params.us_zsize );
		igs::resource::swap_endian( save_params.l_pixmin );
		igs::resource::swap_endian( save_params.l_pixmax );
		igs::resource::swap_endian( save_params.l_dummy );
		igs::resource::swap_endian( save_params.l_colormap );
	}

	/* �ե����볫�� ------------------------------------*/
 	std::ofstream ofs(file_path.c_str(), std::ios::binary);
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> open<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* �إå���¸ --------------------------------------*/
	ofs.write(
		igs::resource::pointer_cast<char *>( &save_params ),
		sizeof(igs::sgi::header::parameters)
	);
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> write<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* ������¸ ----------------------------------------*/
	if (igs::sgi::header::compression::rle ==
	head_params.c_storage) {
		/* table������� ---------------*/
		std::vector<unsigned long> cl_rle_start;
		std::vector<unsigned long> cl_rle_size;
		cl_rle_start.resize(
			head_params.us_ysize * head_params.us_zsize);
		cl_rle_size.resize(
			head_params.us_ysize * head_params.us_zsize);

		/* run length������¸�ν���� ----*/
		igs::sgi::rle_write encode_writer(
			// &cl_image.at(0)
			image_array
		, static_cast<unsigned int>(head_params.us_ysize)
		, static_cast<unsigned int>(head_params.us_xsize)
		, static_cast<unsigned int>(head_params.us_zsize)
		, static_cast<unsigned int>(head_params.c_bytes_per_ch)
			, byte_swap_sw
			, &cl_rle_start.at(0)
			, &cl_rle_size.at(0)
			, (igs::image::orientation::topleft == props.orie_side)
		);
		/* ��������¸����file���֤ذ�ư --*/
		ofs.seekp( static_cast<std::streamoff>(512
			+ cl_rle_start.size()* sizeof(unsigned long)
			+ cl_rle_size.size() * sizeof(unsigned long)
		) );
		if (!ofs) {
			std::string str;
			str = "file<";
			str += file_path;
			str += "> seekp<std::ofstream> error";
			throw std::domain_error(str);
		}
		/* ��������¸ --------------------*/
		for (unsigned int ii=0;ii<encode_writer.count();++ii) {
			encode_writer.segment_exec(ofs);
		}
		/* history_str�������¸ -------------*/
		if (0 < history_str.size()) {
			/* history_strʸ����Ͻ�ü'\0'��ޤޤ��� */
			ofs.write(
				const_cast<char *>(history_str.c_str()),
				static_cast<std::streamsize>(
					history_str.size()+1
				)
			);
		}
		/* table�Х��ȥ���å� -----------*/
		if ( byte_swap_sw ) {
			igs::resource::swap_array_endian(
				&cl_rle_start.at(0),
				cl_rle_start.size()
			);
			igs::resource::swap_array_endian(
				&cl_rle_size.at(0),
				cl_rle_size.size()
			);
		}
		/* table��¸���� ---------------*/
		ofs.seekp(512L);
		if (!ofs) {
			std::string str;
			str = "file<";
			str += file_path;
			str += "> seekp<std::ofstream> error";
			throw std::domain_error(str);
		}
		/* table��¸(������¸��ɬ��) -----*/
		ofs.write(
			igs::resource::pointer_cast<char *>(
				&cl_rle_start.at(0)
			),
			static_cast<std::streamsize>(
			 cl_rle_start.size() * sizeof(unsigned long)
			)
		);
		if (!ofs) {
			std::string str;
			str = "file<";
			str += file_path;
			str += "> write<std::ofstream> error";
			throw std::domain_error(str);
		}
		ofs.write(
			igs::resource::pointer_cast<char *>(
				&cl_rle_size.at(0)
			),
			static_cast<std::streamsize>(
			 cl_rle_size.size() * sizeof(unsigned long)
			)
		);
		if (!ofs) {
			std::string str;
			str = "file<";
			str += file_path;
			str += "> write<std::ofstream> error";
			throw std::domain_error(str);
		}
	} else {
		/* verbatim��¸�ν���� ----------*/
		igs::sgi::verbatim_write encode_writer(
			//&cl_image.at(0)
			image_array
		, static_cast<unsigned int>(head_params.us_ysize)
		, static_cast<unsigned int>(head_params.us_xsize)
		, static_cast<unsigned int>(head_params.us_zsize)
		, static_cast<unsigned int>(head_params.c_bytes_per_ch)
			, byte_swap_sw
			, (igs::image::orientation::topleft == props.orie_side)
		);
		/* ��������¸ --------------------*/
		for (unsigned int ii=0;ii<encode_writer.count();++ii) {
			encode_writer.segment_exec(ofs);
		}
		/* history_str�������¸ -------------*/
		if (0 < history_str.size()) {
			/* history_strʸ����Ͻ�ü'\0'��ޤޤ��� */
			ofs.write(
				const_cast<char *>(history_str.c_str()),
				static_cast<std::streamsize>(
					history_str.size() + 1
				)
			);
		}
	}
	/* �ե������Ĥ��� ----------------------------------*/
	ofs.close();
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> close<std::ofstream> error";
		throw std::domain_error(str);
	}
}
