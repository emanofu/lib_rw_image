#include <sstream> /* std::ostringstream */
#include <iomanip> /* std::setw(-) */
#include "igs_tga_reader.h"
#include "igs_tga_read_handle.h"

//-----------------------------------------------------------
using igs::tga::reader;

void reader::get_info(
	const std::string& file_path
	, std::string& info
) {
	std::ostringstream os;

	os << this->name() << '\n';

	bool byte_swap_sw;
	igs::tga::header::parameters header_params;

	igs::tga::read_handle::get_info(
		file_path, byte_swap_sw, header_params
	);

	os << " Byte Order : Little Endian(Format Spec.)\n";

	os << " ID Field : ";
	if (0 < (int)header_params.id_length) {
	 os << "Exist " << (int)header_params.id_length << " byte";
	 if (1 < (int)header_params.id_length) { 
	  os << 's';
	 }
	} else {
	 os << "Nothing";
	}
	os << '\n';

	os << " ColorMap Field : ";
	if (0 < (int)header_params.cmap_type) {
	 os << "Exist";
	} else {
	 os << "Nothing";
	}
	os << '\n';

	os << " Image Field : ";
	if (0 < (int)header_params.image_type) {
	 switch ((int)header_params.image_type & 0x07) {
	 case 1: os << "ColorMap"; break;
	 case 2: os << "FullColor"; break;
	 case 3: os << "Grayscale"; break;
	 default: os	<< "unknown<"
			<< ((int)header_params.image_type & 0x07)
			<< '>';
		break;
	 }
	 os << "  Compression is ";
	 if ((int)header_params.image_type & 0x08) {
	  os << "RLE";
	 } else {
	  os << "Uncompress";
	 }
	} else {
	 os << "Nothing";
	}
	os << '\n';

	os	<< " ColorMap index of entry : "
		<< (int)header_params.cmap_index_of_entry
		<< '\n';

	os	<< " ColorMap count of entries : "
		<< (int)header_params.cmap_count_of_entries
		<< '\n';

	os	<< " ColorMap bits depth : "
		<< (int)header_params.cmap_bits_depth
		<< '\n';

	os	<< " Left Lower Position : "
		<< (int)header_params.left_term_pos
		<< " "
		<< (int)header_params.lower_term_pos
		<< '\n';

	os	<< " Orientation : ";
	if ((int)header_params.descriptor & 0x10) {
	 os	<< "Right to Left";
	} else {
	 os	<< "Left to Right";
	}
	 os	<< "  ";
	if ((int)header_params.descriptor & 0x20) {
	 os	<< "Top to Bottom";
	} else {
	 os	<< "Bottom to Top";
	}
	os	<< '\n';

	os	<< " Interleave : ";
	int interleave = static_cast<int>(
		(header_params.descriptor&0xc0) >> 6
	);
	switch (interleave) {
	case 0: os << "non-interleaved"; break;
	case 1: os << "two-way(even/odd)interleaving"; break;
	case 2: os << "four way interleaving"; break;
	case 3: os << "reserved"; break;
	}
	 os	<< '\n';
	//------------------------------------------------------------

	os << "Image\n"

	    << " Size : "
	    << (int)header_params.width
	    << " x "
	    << (int)header_params.height
	    << "  "
	    ;
	int image_type = (int)header_params.image_type & 0x07;
	if      (1 == image_type) { os << "ColorMap"; }
	else if (2 == image_type) { os << "FullColor"; }
	else if (3 == image_type) { os << "Grayscale"; }
	else {
	os	<< "Bad_image_type"
		<< '(' << (int)image_type << ')';
 	}
	os	<< ' '
		<< (int)header_params.bits_par_pixel
		<< " bits"
		<< "  Attribute "
		<< (int)(header_params.descriptor & 0x0f)
		<< " bits\n";

	os << " Resolution : Not Defined\n";

	os << " Orientation(Row Column) : ";
	if ((int)header_params.descriptor & 0x20) {
	 os	<< "Top";
	} else {
	 os	<< "Bottom";
	}
	os	<< ' ';
	if ((int)header_params.descriptor & 0x10) {
	 os	<< "Right";
	} else {
	 os	<< "Left";
	}
	os	<< '\n';
			/* TGA画像ファイル上の原点位置は左下 */

	info += os.str();
}
//-----------------------------------------------------------
void
reader::get_line_head(std::string& line_head) {
	line_head += "bo  id c imag cmp c_i c_c cb leftp lowep width heigh bpp db dh dv di file\n";
}
void
reader::get_line_info(const std::string& file_path,std::string& line_info) {
	bool byte_swap_sw;
	igs::tga::header::parameters header_params;

	bool b_canuse_sw = false;
 try {
	igs::tga::read_handle::get_info(
		file_path,
		byte_swap_sw,
		header_params
        );
 }
 //catch (std::exception& e) {
 //	std::cout << e.what() << '\n';
 catch (std::exception) {
	b_canuse_sw = true;
 }

	std::ostringstream os;

	os << "lit";

	/* このライブラリで扱えるか否か ------------------- */
	if (b_canuse_sw) { os << 'x'; }
	else             { os << ' '; }

	os << std::setw(2) << (int)header_params.id_length;
	os << std::setw(2) << (int)header_params.cmap_type;
	int image_type = (int)header_params.image_type & 0x07;
	os << std::setw(5);
	switch (image_type) {
	case 0: os << "None"; break;
	case 1: os << "CMap"; break;
	case 2: os << "Full"; break;
	case 3: os << "Gray"; break;
	}
	os << std::setw(4);
	if ((int)header_params.image_type & 0x08) {
		os << "RLE";
	} else {
		os << "not";
	}
	os << std::setw(4) << (int)header_params.cmap_index_of_entry;
	os << std::setw(4) << (int)header_params.cmap_count_of_entries;
	os << std::setw(3) << (int)header_params.cmap_bits_depth;
	os << std::setw(6) << (int)header_params.left_term_pos;
	os << std::setw(6) << (int)header_params.lower_term_pos;
	os << std::setw(6) << (int)header_params.width;
	os << std::setw(6) << (int)header_params.height;
	os << std::setw(4) << (int)header_params.bits_par_pixel;
	os << std::setw(3) << ((int)header_params.descriptor&0x0f);
	os << std::setw(3) << ((header_params.descriptor&0x10)?"RI":"LE");
	os << std::setw(3) << ((header_params.descriptor&0x20)?"TO":"BO");
	os << std::setw(3) << ((header_params.descriptor&0xc0)>>10);

	/* ファイル名(ディレクトリパスを除く) ------------- */
	std::string::size_type index = file_path.find_last_of("/\\");
	if (std::string::npos == index) {
		os << ' ' << file_path;
	} else {
		os << ' ' << file_path.substr(index+1);
	}
	/* 改行コード ------------------------------------- */
	os << '\n';

	line_info += os.str();
}
//-----------------------------------------------------------
#if DEBUG_TGA_INFO
/*
g++ -Wall -O2 -g -DDEBUG_TGA_INFO\
-I. -I${HOME}/utility_tools/rhel4/include\
igs_tga_info.cxx\
${HOME}/utility_tools/rhel4/lib/libigs_tga_reader.a\
${HOME}/utility_tools/rhel4/lib/libigs_resource_irw.a\
-o tes_bin_info-rhel4-a

cl /W3 /MD /EHa /O2 /wd4819 /DDEBUG_TGA_INFO /I.\
/I%HOME%/utility_tools/vc2005md/include\
igs_tga_info.cxx\
%HOME%/utility_tools/vc2005md/lib/libigs_tga_reader.lib\
%HOME%/utility_tools/vc2005md/lib/libigs_resource_irw.lib\
/Fetes_info-vc2005md-lib
*/
#include <iostream>
#include "igs_resource_irw.h"

namespace {
 void _print_info(int argc, char *argv[]) {
	std::string info;
	igs::tga::reader tga_re;
	for (int ii = 0; ii < argc; ++ii) {
		std::ostringstream os;
		igs::resource::file_stat fsta(argv[ii]);
		fsta.form_stat(os);
		info += os.str();
		if (fsta.is_exist) { tga_re.get_info(argv[ii],info); }
	}
	std::cout << info;
 }
 void _print_column(int argc, char *argv[]) {
	std::string info;
	igs::tga::reader tga_re;
	tga_re.get_line_head(info);
	for (int ii = 0; ii < argc; ++ii) {
		tga_re.get_line_info(argv[ii],info);
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
		<< "\t.tga\n";
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
#endif /* DEBUG_TGA_INFO */
