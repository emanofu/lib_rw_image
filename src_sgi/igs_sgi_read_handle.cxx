#include <vector> /* std::vector */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_sgi_read_handle.h"

const bool
igs::sgi::read_handle::istag(
	const unsigned int bytes_count
	, const unsigned char* const tag_array
	, bool& byte_swap_sw
) {
	if (bytes_count < 2) { return false; }

	union { unsigned char uca[2]; unsigned short us; } ut;
	ut.us = static_cast<unsigned short>(
		igs::sgi::header::tag::s_magic);

	if (	(ut.uca[0] == tag_array[0]) &&
		(ut.uca[1] == tag_array[1])
	) {
		/* cpu��byte order��file��byte order��Ʊ�� */
		byte_swap_sw = false;
		return true; /* OK */
	}
	if (	(ut.uca[1] == tag_array[0]) &&
		(ut.uca[0] == tag_array[1])
	) {
		/* cpu��byte order��file��byte order���դΤȤ� */
		byte_swap_sw =true;/* byte swap���ʤ����ɤ� */
		return true; /* OK */
	}
	return false; /* Error */
}
namespace {
 /* �ե�����񼰤���äƤ����throw */
 void check_tag(
	const unsigned int bytes_count
	, const unsigned char* const tag_array
	, bool& byte_swap_sw
 ) {
	if (igs::sgi::read_handle::istag(
		bytes_count, tag_array, byte_swap_sw)) { return; }

	std::ostringstream os;
	  os
		<< std::hex
		<< "file tag(";
	for (unsigned int ii=0; ii < bytes_count; ++ii) {
	 if (0 < ii) {
	  os
		<< ' ';
	 }
	  os
		<< (int)tag_array[ii];
	}
	  os
		<< ")"
		<< "is not sgi image tag("
		<< igs::sgi::header::tag::s_magic
		<< ")"
		<< std::dec;
	throw std::domain_error(os.str());
 }
 void byte_swap_header(igs::sgi::header::parameters *header_ptr) {
	igs::resource::swap_endian( header_ptr->s_magic );

	/* sgi�����ե�������¸�Ǥ�
		header_ptr->c_storage��
		header_ptr->c_bytes_per_ch��
		2bytes(ushort)�Ȥ��Ƥ��Ĥ��� */
	char cc = header_ptr->c_storage;
	header_ptr->c_storage = header_ptr->c_bytes_per_ch;
	header_ptr->c_bytes_per_ch = cc;

	igs::resource::swap_endian( header_ptr->us_dimension );
	igs::resource::swap_endian( header_ptr->us_xsize );
	igs::resource::swap_endian( header_ptr->us_ysize );
	igs::resource::swap_endian( header_ptr->us_zsize );
	igs::resource::swap_endian( header_ptr->l_pixmin );
	igs::resource::swap_endian( header_ptr->l_pixmax );
	igs::resource::swap_endian( header_ptr->l_dummy );
	igs::resource::swap_endian( header_ptr->l_colormap );
 }
 void byte_swap_sto_ch(igs::sgi::header::parameters *header_ptr) {
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
	/* little endian��CPU�Ǥϡ�2bytesñ�̤Υǡ�����1byte�Ǹ����
	�դλ������򤷤Ƥ���ΤǤ�ɤ� */
	if (1 == buf.uc[0]) { /* Little Endian CPU */
		char cc = header_ptr->c_storage;
		header_ptr->c_storage = header_ptr->c_bytes_per_ch;
		header_ptr->c_bytes_per_ch = cc;
	}
 }
}
/* �ե����뤫�顢��ǽ�ʤ��٤ƤΥե������������� */
void
igs::sgi::read_handle::get_info(
	const std::string& file_path
	, bool& byte_swap_sw
	, igs::sgi::header::parameters& header_param_ref
) {
	/* �ե����볫�� */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* �ե����뤫��إå����Τ��ɤ� */
	cl_fp.read(
	  igs::resource::pointer_cast<unsigned char *>(&header_param_ref)
		,sizeof(igs::sgi::header::parameters)
	);

	/* �ե������Ĥ��� */
	cl_fp.close();

	/* �Ϥ��2bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
		igs::resource::pointer_cast<unsigned char *>(&header_param_ref),
		byte_swap_sw);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(&header_param_ref); }

	/* little endian��CPU�Ǥϡ�2bytesñ�̤Υǡ�����1byte�Ǹ����
	�դλ������򤷤Ƥ���ΤǤ�ɤ� */
 	byte_swap_sto_ch(&header_param_ref);
}
void
igs::sgi::read_handle::get_reso(
	const char imagename[80]
	, igs::image::resolution::unit& reso_unit
	, double& reso_x
	, double& reso_y
) {
	/* (custom��ǽ)�����٤μ����� */
	std::string str = imagename;
	std::string tag;
	std::istringstream is(str);
	is >> tag >> reso_x >> reso_y;

	using namespace igs::sgi::header::resolution;
	using namespace igs::image::resolution;
	if      (ca2_meter     ==tag) { reso_unit = meter; }
	else if (ca2_centimeter==tag) { reso_unit = centimeter; }
	else if (ca2_millimeter==tag) { reso_unit = millimeter; }
	else if (ca2_inch      ==tag) { reso_unit = inch; }
	else			      { reso_unit = nothing; }
}

namespace {
 unsigned long
 get_file_base_bytes(
	size_t table_size
	, unsigned long* seek_table
	, unsigned long* size_table
 ) {
	unsigned long	seek_set_max=0,
			bytes_size_max=0;
	for (unsigned int ii = 0; ii < table_size; ++ii) {
		if (seek_set_max < seek_table[ii]) {
			seek_set_max  =seek_table[ii];
			bytes_size_max=size_table[ii];
		}
	}
	return seek_set_max + bytes_size_max;
 }
}
void
igs::sgi::read_handle::get_size(
	const std::string& file_path
	, size_t& image_size
	, size_t& history_size
) {
	/* �ե����볫�� */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* �ե����뤫��إå����Τ��ɤ� */
	igs::sgi::header::parameters header_params;
	cl_fp.read(
	  igs::resource::pointer_cast<unsigned char *>(&header_params)
		,sizeof(igs::sgi::header::parameters)
	);

	/* �Ϥ��2bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	bool byte_swap_sw=false;
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
	 igs::resource::pointer_cast<unsigned char *>(&header_params),
	 byte_swap_sw);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(&header_params); }

	/* little endian��CPU�Ǥϡ�2bytesñ�̤Υǡ�����1byte�Ǹ����
	�դλ������򤷤Ƥ���ΤǤ�ɤ� */
 	byte_swap_sto_ch(&header_params);

	/* ���������� */
	image_size = static_cast<size_t>(header_params.us_xsize)
		* header_params.us_ysize
		* header_params.us_zsize
		* header_params.c_bytes_per_ch;

	/* �����ե������(��ĥ��ޤޤʤ�history�ʳ���)������������ */
	unsigned int file_base_bytes=0;
	if (igs::sgi::header::compression::rle ==
	header_params.c_storage) {
		/* rle�ξ��table���ɤ߹��߲����κ��祵���������� */
		/* table�ѥХåե������ */
		size_t table_size =
			static_cast<size_t>(header_params.us_ysize)
			* header_params.us_zsize;
		std::vector<unsigned long> cl_table;
		cl_table.resize(table_size * 2);
		/* table���ɤ߹��� */
		cl_fp.read(
	igs::resource::pointer_cast<unsigned char *>(&cl_table.at(0))
			,cl_table.size()*sizeof(unsigned long)
		);
		/* table��Х��ȥ���å� */
		if (byte_swap_sw) {
			igs::resource::swap_array_endian(
				&cl_table.at(0), cl_table.size()
			);
		}
		/* �����ǡ����Υ饹��seek���֤����� */
		file_base_bytes = get_file_base_bytes(
			table_size,
			&cl_table.at(0),
			&cl_table.at(0) + table_size);
	} else {
		file_base_bytes =
			sizeof(igs::sgi::header::parameters) +
			header_params.us_xsize *
			header_params.us_ysize *
			header_params.us_zsize *
			header_params.c_bytes_per_ch;
	}

	/* �ե������Ĥ��� */
	cl_fp.close();

	/* history�����������롣ʸ����ν�ü'\0'��ޤ� */
	size_t file_size = igs::resource::bytes_of_file(file_path.c_str());
	if (file_size < file_base_bytes) {
		throw std::domain_error(
			"file size is too small then image size");
	}
	history_size = file_size - file_base_bytes;
}

void
igs::sgi::read_handle::check_parameters(
	igs::sgi::header::parameters *header_ptr
) {
	/* ���̥⡼�� */
	switch (header_ptr->c_storage) {
	case igs::sgi::header::compression::verbatim: break;
	case igs::sgi::header::compression::rle:      break;
	default:
		std::ostringstream os;
		os	<< "bad compress type("
			<< (int)header_ptr->c_storage << ")";
		throw std::domain_error(os.str());
	}

	/* ��channel��byte�� */
	switch (header_ptr->c_bytes_per_ch) {
	case 1: break;
	case 2: break;
	default:
		std::ostringstream os;
		os	<< "bad sampling bytes("
			<< (int)header_ptr->c_bytes_per_ch << ")";
		throw std::domain_error(os.str());
	}
	/* xyz������ */
	switch ((header_ptr->us_dimension)) {
	case 1: break;
	case 2: break;
	case 3: break;
	default:
		std::ostringstream os;
		os	<< "bad dimension number("
			<< header_ptr->us_dimension << ")";
		throw std::domain_error(os.str());
	}
	/* �������ȥ������Υ����å� */
	if (	((header_ptr->us_dimension) < 3) &&
		(1 < (header_ptr->us_zsize))
	) {
		std::ostringstream os;
		os 	<< "dimention number("
			<< header_ptr->us_dimension
			<< "), but zsize(" << header_ptr->us_zsize
			<< ")";
		throw std::domain_error(os.str());
	}
	if (	((header_ptr->us_dimension) < 2) &&
		(1 < (header_ptr->us_ysize))
	) {
		std::ostringstream os;
		os 	<< "dimention number("
			<< header_ptr->us_dimension
			<< "), but ysize(" << header_ptr->us_ysize
			<< ")";
		throw std::domain_error(os.str());
	}
	/* �����⡼��
		zsize��1�ΤȤ�������
			igs::sgi::header::form::dithered
			igs::sgi::header::form::screen
			igs::sgi::header::form::colormap
		�ϰ�̣������
			igs::sgi::header::form::normal
		�ϡ�zsize��1,3,4�Ȥʤ롣

		�Ť�sgi����(mononoke)�ǡ�
			zsize	   -->	3 or 4
			l_colormap -->	igs::sgi::header::form::dithered
		�ȤʤäƤ�����������ä���
		l_colormap�ϸ��ߤܻۤȤ��Ƥ��ʤ��Τ�̵�뤹�뤳�Ȥǡ�
		������ɤ߼���褦�ˤ���
		2012-12-14
	*/
	/***********
	if (1 != header_ptr->us_zsize) {
		switch ((header_ptr->l_colormap)) {
		case igs::sgi::header::form::normal:
			break;
		case igs::sgi::header::form::dithered:
		case igs::sgi::header::form::screen:
		case igs::sgi::header::form::colormap:
			std::ostringstream os;
			os	<< "bad colormap number("
				<< header_ptr->l_colormap << ")";
			throw std::domain_error(os.str());
		}
	}
	************/
}

void
igs::sgi::read_handle::get_data(
	const size_t file_size
	, unsigned char* file_image
	, bool& byte_swap_sw
	, igs::sgi::header::parameters*& header_ptr_ref
	, unsigned long*& seek_set_ptr_ref
	, unsigned long*& bytes_size_ptr_ref
	, unsigned char*& image_ptr_ref
	, char*& history_ptr_ref
) {
	/* �Ϥ��4bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
		file_image, byte_swap_sw );

	/* �ե����륤�᡼������إå�����ݥ��󥿤� */
	header_ptr_ref = igs::resource::pointer_cast<
		igs::sgi::header::parameters *>(file_image);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(header_ptr_ref); }

	/* little endian��CPU�Ǥϡ�2bytesñ�̤Υǡ�����1byte�Ǹ����
	�դλ������򤷤Ƥ���ΤǤ�ɤ� */
 	byte_swap_sto_ch(header_ptr_ref);

	/* table,image,history */
	size_t file_base_bytes = 0;
	size_t file_image_part_bytes = 0;
	if (	igs::sgi::header::compression::rle ==
		header_ptr_ref->c_storage) {
		int table_size=	header_ptr_ref->us_ysize *
				header_ptr_ref->us_zsize;
		seek_set_ptr_ref = igs::resource::pointer_cast<unsigned long *>(
			file_image +
			sizeof(igs::sgi::header::parameters)
		);
		bytes_size_ptr_ref = seek_set_ptr_ref + table_size;
		/* table��Х��ȥ���å� */
		if (byte_swap_sw) {
			igs::resource::swap_array_endian(
 				seek_set_ptr_ref, table_size * 2
			);
		}
		image_ptr_ref = igs::resource::pointer_cast<unsigned char *>(
			bytes_size_ptr_ref + table_size);
		file_base_bytes = static_cast<size_t>(
			get_file_base_bytes(
				table_size,
				seek_set_ptr_ref, bytes_size_ptr_ref
			)
		);
		file_image_part_bytes = file_base_bytes -
			sizeof(igs::sgi::header::parameters) -
			table_size * sizeof(unsigned long) * 2;
	} else {
		image_ptr_ref = 
			file_image +
			sizeof(igs::sgi::header::parameters);
		file_image_part_bytes = static_cast<size_t>(
			header_ptr_ref->us_xsize)
			* header_ptr_ref->us_ysize
			* header_ptr_ref->us_zsize
			* header_ptr_ref->c_bytes_per_ch;
		file_base_bytes =
			sizeof(igs::sgi::header::parameters) +
			file_image_part_bytes;
	}
	if (file_size < file_base_bytes) {
		history_ptr_ref = igs::resource::pointer_cast<char *>(
			file_image + file_base_bytes);
	}
	/* 16bits sampling����byte swap */
	if (byte_swap_sw && (2 == header_ptr_ref->c_bytes_per_ch)) {
		igs::resource::swap_array_endian(
			igs::resource::pointer_cast<unsigned short *>(
				image_ptr_ref
			),
			file_image_part_bytes/2
		);
	}
}
