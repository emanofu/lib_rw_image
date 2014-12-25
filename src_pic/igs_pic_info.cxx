// #include <limits> /* std::numeric_limits */
// #include <cctype> /* tolower() */
// #include <fstream> /* std::ifstream */
// #include <iostream> /* std::cerr std::cout */
// #include <stdexcept> /* std::domain_error */
// #include "pic_extensions.h"
// #include "pic_decode.h"

#include <sstream> /* std::ostringstream */
#include <iomanip> /* std::setw(-) */
#include "igs_pic_read_handle.h"
#include "igs_pic_reader.h"

//-----------------------------------------------------------
using igs::pic::reader;

void
reader::get_info(const std::string& file_path, std::string& info) {
	std::ostringstream os;

	 os << this->name() << '\n';

	bool byte_swap_sw;
	igs::pic::header::parameters header_params;
	std::vector<igs::pic::header::channel_packet> packet_array;
	unsigned char channel_flag;
	unsigned int channel_size;
	igs::pic::read_handle::get_info(
		file_path,
		byte_swap_sw,
		header_params,
		packet_array,
		channel_flag,
		channel_size
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
	 os << "\n";

	 os << " Version : "
	    << header_params.f_version
	    << "\n";

	if (0 != header_params.ca80_comment[0]) {
	 os << " Comment : "
	    << "\"";
		for (unsigned int ii = 0;
		(ii < sizeof(header_params.ca80_comment)) &&
		(0 != header_params.ca80_comment[ii]); ++ii) {
	 os << header_params.ca80_comment[ii];
		}
	 os << "\"\n";
	}
	
 {
	 os << " Channel Packet :\n";
	using namespace igs::pic::header::compress_type;
	using namespace igs::pic::header::channel;
	for (std::vector<igs::pic::header::channel_packet>::iterator
	it  = packet_array.begin();
	it != packet_array.end(); ++it) {
		//if (it != packet_array.begin()) { os << "  "; }
		os << "\t";
		if (uc_red_bit & it->uc_channels) { os << 'R'; }
		else				  { os << '-'; }
		if (uc_gre_bit & it->uc_channels) { os << 'G'; }
		else				  { os << '-'; }
		if (uc_blu_bit & it->uc_channels) { os << 'B'; }
		else				  { os << '-'; }
		if (uc_alp_bit & it->uc_channels) { os << 'A'; }
		else				  { os << '-'; }
		os << ' ' << static_cast<int>(it->uc_size) << " bits  ";
		/***os << std::hex << std::showbase << "("
			<< static_cast<int>(it->uc_channels)
			<< ")" << std::dec;
		***/
		switch (it->uc_type) {
		case uc_uncompres: os << "Uncompress"; break;
		case uc_pure_rle:  os << "PureRLE";    break;
		case uc_mixed_rle: os << "MixedRLE";   break;
		default: { os << "Bad_Compression_Type"
			 << '(' << static_cast<int>(it->uc_type) << ')';
			}
			break;
		}
		os << "\n";
	}
 }
 {
	 os << " Field : ";
	using namespace igs::pic::header::field_type;
	switch (header_params.us_fields) {
	case us_no_field:   os << "No Field";   break;
	case us_odd_field:  os << "Odd Field";  break;
	case us_even_field: os << "Even Field"; break;
	case us_full_frame: os << "Full Frame"; break;
	default: { os	<< "Bad_Field_Type"
			<< '(' << (int)(header_params.us_fields) << ')';
		}
		break;
	}
	 os << "\n";
 }
	 os << " Ratio : " << header_params.f_ratio << "\n";

	//------------------------------------------------------------

	 os << "Image\n"
	    << " Size : "
	    << header_params.us_width
	    << " x "
	    << header_params.us_height
	    ;
 {
	using namespace igs::pic::header::channel;
	for (std::vector<igs::pic::header::channel_packet>::iterator
	it  = packet_array.begin();
	it != packet_array.end(); ++it) {
		os << "  ";
		if (uc_red_bit & it->uc_channels) { os << 'R'; }
		if (uc_gre_bit & it->uc_channels) { os << 'G'; }
		if (uc_blu_bit & it->uc_channels) { os << 'B'; }
		if (uc_alp_bit & it->uc_channels) { os << 'A'; }
		os << ' ' << static_cast<int>(it->uc_size) << " bits";
	}
	 os << "\n";
 }

	 os << " Resolution : Nothing\n";
	 os << " Orientation(Row Column) : Top Left\n";

	info += os.str();
}
//-----------------------------------------------------------
void
reader::get_line_head(std::string& line_head) {
	line_head += "bo  vers   id width heigh pixr field";
	line_head += " (s colo compress)...";
	line_head += " (comment) filename\n";
}
void
reader::get_line_info(const std::string& file_path,std::string& line_info) {
	// using namespace igs::pic::header;

	bool byte_swap_sw = false;
	igs::pic::header::parameters header_params;
	std::vector<igs::pic::header::channel_packet> packet_array;
	unsigned char channel_flag;
	unsigned int channel_size;

	bool b_canuse_sw = true;
 try {
	/* ファイル開く。ヘッダー読む(読んでファイル閉じている) */
	igs::pic::read_handle::get_info(
		file_path,
		byte_swap_sw,
		header_params,
		packet_array,
		channel_flag,
		channel_size
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

	/* 各パラメータ------------------------------------ */
	os.precision(2); /* 1.23 */
	os << std::fixed << std::setw(4)
		<< header_params.f_version; /* 1.23 */
	os << ' ';
	os << header_params.ca4_id[0];
	os << header_params.ca4_id[1];
	os << header_params.ca4_id[2];
	os << header_params.ca4_id[3];
	os << std::setw(6) << header_params.us_width;
	os << std::setw(6) << header_params.us_height;
	os << ' ';
	os << std::setw(4) << header_params.f_ratio; /* 1.23 */
	os << ' ';
 {
	// os << header_params.us_fields << ' ';
	using namespace igs::pic::header::field_type;
	switch (header_params.us_fields) {
	case us_no_field:   os << "nothi"; break;
	case us_odd_field:  os << "oddfr"; break;
	case us_even_field: os << "evenf"; break;
	case us_full_frame: os << "fullf"; break;
	default:            os << "unkno"; break;
	}
	// os << std::setw(6) << header_params.us_pad;
 }

	/* 色チャンネルパケット情報 ----------------------- */
 {
	os << " ";
	using namespace igs::pic::header::compress_type;
	using namespace igs::pic::header::channel;
	for (std::vector<igs::pic::header::channel_packet>::iterator
	it  = packet_array.begin();
	it != packet_array.end(); ++it) {
		os << " ";
		os << (int)(it->uc_size);
		os << ' ';
		if (uc_red_bit & it->uc_channels) { os << 'r'; }
		else				  { os << '-'; }
		if (uc_gre_bit & it->uc_channels) { os << 'g'; }
		else				  { os << '-'; }
		if (uc_blu_bit & it->uc_channels) { os << 'b'; }
		else				  { os << '-'; }
		if (uc_alp_bit & it->uc_channels) { os << 'a'; }
		else				  { os << '-'; }
		os << ' ';
		switch (it->uc_type) {
		case uc_uncompres: os << "Uncompre";break;
		case uc_pure_rle:  os << "Pure_RLE";break;
		case uc_mixed_rle: os << "MixedRLE";break;
		default:           os << "Unknown ";break;
		}
	}
 }

	/* コメント --------------------------------------- */
	if (0 != header_params.ca80_comment[0]) {
		os << " \"";
		for (unsigned int ii = 0;
		(ii < sizeof(header_params.ca80_comment)) &&
		(0 != header_params.ca80_comment[ii]); ++ii) {
			os << header_params.ca80_comment[ii];
		}
		os << '\"';
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

	os.precision(6); /* reset */

	line_info += os.str();
}
//-----------------------------------------------------------
#if DEBUG_PIC_INFO
/*
g++ -Wall -O2 -g -DDEBUG_PIC_INFO\
-I. -I${HOME}/utility_tools/rhel4/include\
igs_pic_info.cxx\
${HOME}/utility_tools/rhel4/lib/libigs_pic_reader.a\
${HOME}/utility_tools/rhel4/lib/libigs_resource_irw.a\
-o tes_bin_info-rhel4-a

cl -W3 -MD -EHa -O2 -wd4819\
-DDEBUG_PIC_INFO\
-I. -I%HOME%/utility_tools/vc2005md/include\
igs_pic_info.cxx\
%HOME%/utility_tools/vc2005md/lib/libigs_pic_reader.lib\
-Fetes_info-vc2005md-lib
*/
#include <iostream>
#include "igs_resource_irw.h"

namespace {
 void _print_info(int argc, char *argv[]) {
	std::string info;
	igs::pic::reader pic_re;
	for (int ii = 0; ii < argc; ++ii) {
		std::ostringstream os;
		igs::resource::file_stat fsta(argv[ii]);
		fsta.form_stat(os);
		info += os.str();
		if (fsta.is_exist) { pic_re.get_info(argv[ii],info); }
	}
	std::cout << info;
 }
 void _print_column(int argc, char *argv[]) {
	std::string info;
	igs::pic::reader pic_re;
	pic_re.get_line_head(info);
	for (int ii = 0; ii < argc; ++ii) {
		pic_re.get_line_info(argv[ii],info);
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
		<< "\t.pic\n";
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
#endif /* DEBUG_PIC_INFO */
