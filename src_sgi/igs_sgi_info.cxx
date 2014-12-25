#include <sstream> /* std::ostringstream */
#include <iomanip> /* std::setw(-) */
#include "igs_sgi_reader.h"
#include "igs_sgi_read_handle.h"

//-----------------------------------------------------------
using igs::sgi::reader;

void
reader::get_info(const std::string& file_path, std::string& info) {
	std::ostringstream os;

	 os << this->name() << '\n';

	bool byte_swap_sw;
	igs::sgi::header::parameters header_params;

	igs::sgi::read_handle::get_info(
		file_path,
		byte_swap_sw,
		header_params
	);

	 os << " Byte Order : ";
	union { unsigned short us; unsigned char uc[2]; } order_box;
	order_box.us = 1;
	const bool cpu_is_little_endian = (1==order_box.uc[0])?true:false;
	if (( (!cpu_is_little_endian) == (!byte_swap_sw) ) ||
	    (   cpu_is_little_endian  ==   byte_swap_sw  )) {
	     /* CPUがbig    endian(false)でbyte swapしてない(false)か、
		CPUがlittle endian(true )でbyte swapした(true) */
	 os << "Big Endian";
	} else {
	 os << "Little Endian";
	}
	 os << '\n';

	 os << " Compression : ";
	if (
	 igs::sgi::header::compression::verbatim==header_params.c_storage
	) {
	 os << "Verbatim";
	} else if (
	 igs::sgi::header::compression::rle==header_params.c_storage
	) {
	 os << "Run Length Encode";
	} else {
	 os << "Bad_Compression_Number";
	 os << '(' << (int)header_params.c_storage << ')';
	}
	 os << '\n';

	//------------------------------------------------------------

	 os << "Image\n"
	    << " Size : "
	    << header_params.us_xsize
	    << " x "
	    << header_params.us_ysize
	    << "  ";
	    ;
	if      (1 == header_params.us_zsize) { os << "Grayscale"; }
	else if (3 == header_params.us_zsize) { os << "RGB"; }
	else if (4 == header_params.us_zsize) { os << "RGBA"; }
	else { os
		<< "Bad_Channel_Size"
		<< '(' << (int)header_params.us_zsize << ')';
 	}
	 os
	    << ' '
	    << static_cast<int>(header_params.c_bytes_per_ch)
	    << " byte";
	if (1 < header_params.c_bytes_per_ch) {
	 os << 's';
	}
	 os << '\n';

 {
	using namespace igs::image::resolution;
	unit e_reso = not_defined;
	double resox=0.0;
	double resoy=0.0;
	igs::sgi::read_handle::get_reso(
		header_params.ca_imagename, e_reso, resox, resoy);

	 os << " Resolution : ";
	switch (e_reso) {
	case meter:
	case centimeter:
	case millimeter:
			os << resox;
		if (resox != resoy) {
			os << " x " << resoy;
		}
			os << ' ';
		break;
	case inch:
	case not_defined:
	case nothing:
		break;
	}
	switch (e_reso) {
	case meter:       os << "Dot/Meter"; break;
	case centimeter:  os << "Dot/Centimeter"; break;
	case millimeter:  os << "Dot/Millimeter"; break;
	case inch:        os << "Dot/Inch"; break;
	case not_defined: os << "Not Defined"; break;
	case nothing:     os << "Nothing"; break;
	}
	 os << '\n';
 }
	 os << " Orientation(Row Column) : Bottom Left\n";
			/* SGI画像ファイル上の原点位置は左下 */

	info += os.str();
}
//-----------------------------------------------------------
void
reader::get_line_head(std::string& line_head) {
	line_head += "bo  comp xsize ysize z b d  pmin  pmax imatyp";
	line_head += " (imagename) file\n";
}
void
reader::get_line_info(const std::string& file_path,std::string& line_info) {
	bool byte_swap_sw;
	igs::sgi::header::parameters header_params;

	bool b_canuse_sw = true;
 try {
	igs::sgi::read_handle::get_info(
		file_path,
		byte_swap_sw,
		header_params
        );
 }
 //catch (std::exception& e) {
 //	std::cout << e.what() << '\n';
 catch (std::exception) {
	b_canuse_sw = false;
 }

	std::ostringstream os;

	/* ファイルのバイトオーダ ------------------------- */
	union { unsigned short us; unsigned char uc[2]; } order_box;
	order_box.us = 1;
	const bool cpu_is_little_endian = (1==order_box.uc[0])?true:false;
	if (( (!cpu_is_little_endian) == (!byte_swap_sw) ) ||
	    (   cpu_is_little_endian  ==   byte_swap_sw  )) {
	     /*	CPUがbig    endian(false)でbyte swapしてない(false)か、
		CPUがlittle endian(true )でbyte swapした(true) */
		os << "big";
	} else {
		os << "lit";
	}

	/* このライブラリで扱えるか否か ------------------- */
	if (b_canuse_sw) { os << " "; }
	else             { os << "x"; }

	/* 圧縮タイプ ------------------------------------- */
	char *cp_comp = "?";
	switch (header_params.c_storage) {
	case igs::sgi::header::compression::verbatim:
				   cp_comp = "verb"; break;
	case igs::sgi::header::compression::rle:
				   cp_comp = "rle";  break;
	}
	os << std::setw(4) << cp_comp;

	/* 画像幅 ----------------------------------------- */
	os << ' ' << std::setw(5) << header_params.us_xsize;
	/* 画像高さ --------------------------------------- */
	os << ' ' << std::setw(5) << header_params.us_ysize;
	/* チャンネル数 ----------------------------------- */
	os << ' ' << std::setw(1) << header_params.us_zsize;
	/* サンプリングバイト数 --------------------------- */
	os << ' ' << std::setw(1)
		<< (int)(header_params.c_bytes_per_ch);
	/* dimension -------------------------------------- */
	os << ' ' << std::setw(1) << header_params.us_dimension;
	/* Pixel最大値 ------------------------------------ */
	os << ' ' << std::setw(5) << header_params.l_pixmin;
	/* Pixel最小値 ------------------------------------ */
	os << ' ' << std::setw(5) << header_params.l_pixmax;

	/* 画像種類 --------------------------------------- */
	char *cp_cmap = "?";
	switch (header_params.l_colormap) {
	case igs::sgi::header::form::normal:   cp_cmap="normal";break;
	case igs::sgi::header::form::dithered: cp_cmap="dither";break;
	case igs::sgi::header::form::screen:   cp_cmap="screen";break;
	case igs::sgi::header::form::colormap: cp_cmap="colorm";break;
	}
	os << ' ' << std::setw(5) << cp_cmap;

	/* コメント --------------------------------------- */
	if (0 != header_params.ca_imagename[0]) {
		os << " (";
		for (unsigned int ii = 0;
		(ii < sizeof(header_params.ca_imagename)) &&
		(0 !=        header_params.ca_imagename[ii]); ++ii) {
			os << header_params.ca_imagename[ii];
		}
		os << ')';
	}

	/* ファイル名(ディレクトリパスを除く) ------------- */
	std::string::size_type  index = file_path.rfind('/');
	if (std::string::npos == index) {
				index = file_path.rfind('\\');
	}
	if (std::string::npos == index) {
		os << ' ' << file_path;
	} else {
		os << ' ' << file_path.substr(index+1);
	}
	/* 改行コード ------------------------------------- */
	os << "\n";

	line_info += os.str();
}
//-----------------------------------------------------------
#if DEBUG_SGI_INFO
/*
g++ -Wall -O2 -g -DDEBUG_SGI_INFO\
-I. -I${HOME}/utility_tools/rhel4/include\
igs_sgi_info.cxx\
${HOME}/utility_tools/rhel4/lib/libigs_sgi_reader.a\
${HOME}/utility_tools/rhel4/lib/libigs_resource_irw.a\
-o tes_bin_info-rhel4-a

cl -W3 -MD -EHa -O2 -wd4819 -DDEBUG_SGI_INFO -I.\
-I%HOME%/utility_tools/vc2005md/include\
igs_sgi_info.cxx\
%HOME%/utility_tools/vc2005md/lib/libigs_sgi_reader.lib\
%HOME%/utility_tools/vc2005md/lib/libigs_resource_irw.lib\
-Fetes_info-vc2005md-lib
*/
#include <iostream>
#include "igs_resource_irw.h"

namespace {
 void _print_info(int argc, char *argv[]) {
	std::string info;
	igs::sgi::reader sgi_re;
	for (int ii = 0; ii < argc; ++ii) {
		std::ostringstream os;
		igs::resource::file_stat fsta(argv[ii]);
		fsta.form_stat(os);
		info += os.str();
		if (fsta.is_exist) { sgi_re.get_info(argv[ii],info); }
	}
	std::cout << info;
 }
 void _print_column(int argc, char *argv[]) {
	std::string info;
	igs::sgi::reader sgi_re;
	sgi_re.get_line_head(info);
	for (int ii = 0; ii < argc; ++ii) {
		sgi_re.get_line_info(argv[ii],info);
	}
	std::cout << info;
 }
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		std::cout
		<< "Usage : " << argv[0] << " [Options] image ...\n"
		<< "[Options]\n"
		<< "image\n"
		<< "\t.sgi,.rgb\n";
		return 0;
	}
 try {
	_print_column( argc-1, &argv[1] );
	_print_info( argc-1, &argv[1] );
 }
 catch (std::exception& e) {
	std::cout << e.what() << '\n';
 }
	return 0;
}
#endif /* DEBUG_TIF_INFO */
