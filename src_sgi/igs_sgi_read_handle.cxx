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
		/* cpuのbyte orderとfileのbyte orderが同じ */
		byte_swap_sw = false;
		return true; /* OK */
	}
	if (	(ut.uca[1] == tag_array[0]) &&
		(ut.uca[0] == tag_array[1])
	) {
		/* cpuのbyte orderとfileのbyte orderが逆のとき */
		byte_swap_sw =true;/* byte swapしながら読む */
		return true; /* OK */
	}
	return false; /* Error */
}
namespace {
 /* ファイル書式が違っていればthrow */
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

	/* sgi画像ファイル保存では
		header_ptr->c_storageと
		header_ptr->c_bytes_per_chを
		2bytes(ushort)としてあつかう */
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
	CPU Byte Orderの説明
	Big Endian CPUではbuf.usに1いれるとbuf.uc[1]が1となる
			+---------------+
	unsigned short	|      us       |
			|           1   |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   0   |   1   |
			+-------+-------+
	Little Endian CPUではbuf.usに1いれるとbuf.uc[0]が1となる
			+---------------+
	unsigned short	|      us       |
			|   1           |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   1   |   0   |
			+-------+-------+
	*/
	/* little endianのCPUでは、2bytes単位のデータを1byteで見ると
	逆の持ち方をしているのでもどす */
	if (1 == buf.uc[0]) { /* Little Endian CPU */
		char cc = header_ptr->c_storage;
		header_ptr->c_storage = header_ptr->c_bytes_per_ch;
		header_ptr->c_bytes_per_ch = cc;
	}
 }
}
/* ファイルから、可能なすべてのファイル情報を得る */
void
igs::sgi::read_handle::get_info(
	const std::string& file_path
	, bool& byte_swap_sw
	, igs::sgi::header::parameters& header_param_ref
) {
	/* ファイル開く */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* ファイルからヘッダ部のみ読む */
	cl_fp.read(
	  igs::resource::pointer_cast<unsigned char *>(&header_param_ref)
		,sizeof(igs::sgi::header::parameters)
	);

	/* ファイル閉じる */
	cl_fp.close();

	/* 始めの2bytesからファイル書式確認と、byte swapするかどうか */
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
		igs::resource::pointer_cast<unsigned char *>(&header_param_ref),
		byte_swap_sw);

	/* fileでのendianとOnMemoryのendianが逆のときswapする */
	if (byte_swap_sw) { byte_swap_header(&header_param_ref); }

	/* little endianのCPUでは、2bytes単位のデータを1byteで見ると
	逆の持ち方をしているのでもどす */
 	byte_swap_sto_ch(&header_param_ref);
}
void
igs::sgi::read_handle::get_reso(
	const char imagename[80]
	, igs::image::resolution::unit& reso_unit
	, double& reso_x
	, double& reso_y
) {
	/* (custom機能)解像度の取りだし */
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
	/* ファイル開く */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* ファイルからヘッダ部のみ読む */
	igs::sgi::header::parameters header_params;
	cl_fp.read(
	  igs::resource::pointer_cast<unsigned char *>(&header_params)
		,sizeof(igs::sgi::header::parameters)
	);

	/* 始めの2bytesからファイル書式確認と、byte swapするかどうか */
	bool byte_swap_sw=false;
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
	 igs::resource::pointer_cast<unsigned char *>(&header_params),
	 byte_swap_sw);

	/* fileでのendianとOnMemoryのendianが逆のときswapする */
	if (byte_swap_sw) { byte_swap_header(&header_params); }

	/* little endianのCPUでは、2bytes単位のデータを1byteで見ると
	逆の持ち方をしているのでもどす */
 	byte_swap_sto_ch(&header_params);

	/* 画像サイズ */
	image_size = static_cast<size_t>(header_params.us_xsize)
		* header_params.us_ysize
		* header_params.us_zsize
		* header_params.c_bytes_per_ch;

	/* 画像ファイルの(拡張を含まないhistory以外の)サイズを得る */
	unsigned int file_base_bytes=0;
	if (igs::sgi::header::compression::rle ==
	header_params.c_storage) {
		/* rleの場合tableを読み込み画像の最大サイズを得る */
		/* table用バッファを確保 */
		size_t table_size =
			static_cast<size_t>(header_params.us_ysize)
			* header_params.us_zsize;
		std::vector<unsigned long> cl_table;
		cl_table.resize(table_size * 2);
		/* tableを読み込む */
		cl_fp.read(
	igs::resource::pointer_cast<unsigned char *>(&cl_table.at(0))
			,cl_table.size()*sizeof(unsigned long)
		);
		/* tableをバイトスワップ */
		if (byte_swap_sw) {
			igs::resource::swap_array_endian(
				&cl_table.at(0), cl_table.size()
			);
		}
		/* 画像データのラストseek位置を得る */
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

	/* ファイル閉じる */
	cl_fp.close();

	/* historyサイズを得る。文字列の終端'\0'を含む */
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
	/* 圧縮モード */
	switch (header_ptr->c_storage) {
	case igs::sgi::header::compression::verbatim: break;
	case igs::sgi::header::compression::rle:      break;
	default:
		std::ostringstream os;
		os	<< "bad compress type("
			<< (int)header_ptr->c_storage << ")";
		throw std::domain_error(os.str());
	}

	/* 各channelのbyte数 */
	switch (header_ptr->c_bytes_per_ch) {
	case 1: break;
	case 2: break;
	default:
		std::ostringstream os;
		os	<< "bad sampling bytes("
			<< (int)header_ptr->c_bytes_per_ch << ")";
		throw std::domain_error(os.str());
	}
	/* xyz次元数 */
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
	/* 次元数とサイズのチェック */
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
	/* 画像モード
		zsizeが1のときだけ、
			igs::sgi::header::form::dithered
			igs::sgi::header::form::screen
			igs::sgi::header::form::colormap
		は意味がある
			igs::sgi::header::form::normal
		は、zsizeは1,3,4となる。

		古いsgi画像(mononoke)で、
			zsize	   -->	3 or 4
			l_colormap -->	igs::sgi::header::form::dithered
		となっている画像があった。
		l_colormapは現在ほぼ使われていないので無視することで、
		これも読み取れるようにした
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
	/* 始めの4bytesからファイル書式確認と、byte swapするかどうか */
	check_tag( sizeof(igs::sgi::header::tag::s_magic),
		file_image, byte_swap_sw );

	/* ファイルイメージからヘッダ部をポインタへ */
	header_ptr_ref = igs::resource::pointer_cast<
		igs::sgi::header::parameters *>(file_image);

	/* fileでのendianとOnMemoryのendianが逆のときswapする */
	if (byte_swap_sw) { byte_swap_header(header_ptr_ref); }

	/* little endianのCPUでは、2bytes単位のデータを1byteで見ると
	逆の持ち方をしているのでもどす */
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
		/* tableをバイトスワップ */
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
	/* 16bits sampling時のbyte swap */
	if (byte_swap_sw && (2 == header_ptr_ref->c_bytes_per_ch)) {
		igs::resource::swap_array_endian(
			igs::resource::pointer_cast<unsigned short *>(
				image_ptr_ref
			),
			file_image_part_bytes/2
		);
	}
}
