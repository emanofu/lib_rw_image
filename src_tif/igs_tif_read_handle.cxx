#include <iostream> /* std::cout */
#include <limits> /* std::numeric_limits */
#include <stdexcept> /* std::domain_error */
#include <sstream> /* std::ostringstream */
#include "igs_tif_error.h"
#include "igs_tif_read_handle.h"

/* libtiffのTIFFヘッダ用スマートクラス定義 */
//------------------------------------------------------------
using igs::tif::read_handle;
read_handle::read_handle(const char *file_path):
	p_(0)
{
	/* スレッドセーフでない */
	TIFFSetErrorHandler( igs::tif::error_handler );

	const char *const mode = "r";
	/* mode "r"	読みこみ
		"w"	書き出し
		"wl"	LittleEndianで書き出し --> 使わない
		"wb"	BigEndianで書き出し --> 使わない
	*/
	this->p_ = TIFFOpen(file_path,mode);
	if (0 == this->p_) {
		throw std::domain_error(this->get_error_msg());
	}
}
char *read_handle::get_error_msg(void)
{ return igs::tif::get_error_msg(); } /* スレッドセーフではない */
void read_handle::close(void)
{ if (0 != this->p_) { TIFFClose(this->p_); this->p_ = 0; } } 
read_handle::~read_handle()
{ try { this->close(); } catch(...) {} } /* destractorの例外を無効化 */
//------------------------------------------------------------
namespace {
 template<class T> int tiff_get_field(TIFF *tif,ttag_t tag,T arg) {
	return TIFFGetField(tif,tag,arg); }
 template<class T> void error_get_field(TIFF *tif,ttag_t tag,const char *name,T arg) {
	int err = tiff_get_field(tif,tag,arg);
	if (1 != err) {
		std::ostringstream os;
		os	<< "TIFFGetField(-) returns " << err << ","
			<< name << " is not define";
		throw std::domain_error(os.str());
	}
 }

 unsigned int get_uint16_throw(TIFF *tif,ttag_t tag,const char *name) {
	uint16 val=0;
	error_get_field(tif,tag,name,&val);
	return static_cast<unsigned int>(val);
 }
 unsigned int get_uint32_throw(TIFF *tif,ttag_t tag,const char *name) {
	uint32 val=0;
	error_get_field(tif,tag,name,&val);
	return static_cast<unsigned int>(val);
 }
 unsigned int get_uint16_no_throw(TIFF *tif,ttag_t tag) {
	uint16 val = 0;
	if (1 != tiff_get_field(tif,tag,&val)) { // not defined
		return std::numeric_limits<unsigned int>::max();
	}
	return static_cast<unsigned int>(val);
 }
 double get_float_no_throw(TIFF *tif,ttag_t tag) {
	float val = 0;
	if (1 != tiff_get_field(tif,tag,&val)) { // not defined
		return 0.0;
	}
	return static_cast<double>(val);
 }
}
//------------------------------------------------------------
unsigned int
read_handle::imagelength(void) { return get_uint32_throw(this->p_,
     TIFFTAG_IMAGELENGTH,
    "TIFFTAG_IMAGELENGTH"); }
unsigned int
read_handle::imagewidth(void) { return get_uint32_throw(this->p_,
     TIFFTAG_IMAGEWIDTH,
    "TIFFTAG_IMAGEWIDTH"); }
unsigned int
read_handle::samplesperpixel(void) { return get_uint16_throw(this->p_,
     TIFFTAG_SAMPLESPERPIXEL,
    "TIFFTAG_SAMPLESPERPIXEL"); }
unsigned int
read_handle::bitspersample(void) { return get_uint16_throw(this->p_,
     TIFFTAG_BITSPERSAMPLE,
    "TIFFTAG_BITSPERSAMPLE"); }
unsigned int
read_handle::photometric(void) { return get_uint16_throw(this->p_,
     TIFFTAG_PHOTOMETRIC,
    "TIFFTAG_PHOTOMETRIC"); }
unsigned int
read_handle::compression(void) { return get_uint16_throw(this->p_,
     TIFFTAG_COMPRESSION,
    "TIFFTAG_COMPRESSION"); }
unsigned int
read_handle::planarconfig(void) { return get_uint16_throw(this->p_,
     TIFFTAG_PLANARCONFIG,
    "TIFFTAG_PLANARCONFIG"); }
unsigned int
read_handle::tilewidth(void) { return get_uint32_throw(this->p_,
     TIFFTAG_TILEWIDTH,
    "TIFFTAG_TILEWIDTH"); }
unsigned int
read_handle::tilelength(void) { return get_uint32_throw(this->p_,
     TIFFTAG_TILELENGTH,
    "TIFFTAG_TILELENGTH"); }
unsigned int
read_handle::orientation(void) { return get_uint16_no_throw(this->p_,
     TIFFTAG_ORIENTATION); }
unsigned int
read_handle::resolutionunit(void) { return get_uint16_no_throw(this->p_,
     TIFFTAG_RESOLUTIONUNIT); }
double
read_handle::xresolution(void) { return get_float_no_throw(this->p_,
     TIFFTAG_XRESOLUTION); }
double
read_handle::yresolution(void) { return get_float_no_throw(this->p_,
     TIFFTAG_YRESOLUTION); }
unsigned int
read_handle::fillorder(void) { return get_uint16_no_throw(this->p_,
     TIFFTAG_FILLORDER); }
int
read_handle::istiled(void) { return TIFFIsTiled(this->p_); }
//------------------------------------------------------------
void
read_handle::check_parameters(void) {
	const unsigned int
		u_phot = this->photometric(),
		bb = this->bitspersample(),
		u_samp = this->samplesperpixel();

/* pixelタイプ
画像種類	pixel channels	sampleing bits
---------------+--------------+---------------
RGBA		4		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
RGB		3		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
grayscale	1		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
B/W		1		1
---------------+--------------+---------------
(※1) by_bi means std::numeric_limits<unsigned char>::digits
*/
	switch (u_phot) {
	/* photometricが単チャンネルの場合 */
	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
		/* サンプル数は一つでなければならない */
		if (1 != u_samp) {
			std::ostringstream os;
			os 	<< "mono must be 1channel,but "
				<< u_samp;
			throw std::domain_error(os.str());
		}
		/* ビット数(1サンプルあたり)が、1,8,16のどれかで
		なければならない */
		switch (bb) {
		case 1:
		case std::numeric_limits<unsigned char>::digits:
		case std::numeric_limits<unsigned short>::digits:
			break;
		default:
			std::ostringstream os;
			os
			<< "mono channel must be "
			<< "1 or "
			<< std::numeric_limits<unsigned char>::digits
			<< " or "
			<< std::numeric_limits<unsigned short>::digits
			<< "bits,but "
			<< bb;
			throw std::domain_error(os.str());
		}
		/*
		MonoBWのとき、MSB(最上位ビット)-->LSB(最下位ビット)
		の並びでなければならない
		Adobe Photoshop CS2 Windowsで保存したMonoBW画像には
		TIFFTAG_FILLORDERがない、ないときはMSB-LSBで扱う
		*/
		if (1 == bb) {
		 const unsigned int  u_fill = this->fillorder();
		 if ((
			std::numeric_limits<unsigned int>::max()!=u_fill
		 ) && (
			FILLORDER_MSB2LSB != u_fill
		 )) {
			std::ostringstream os;
			os	<< "TIFFTAG_FILLORDER(" << u_fill
				<< ") is not support";
			throw std::domain_error(os.str());
		 }
		}
		break;

	/* photometricが複数チャンネル(フルカラー)の場合 */
	case PHOTOMETRIC_RGB:
		/* サンプル数が、3(RGB)か4(RGBA)でなければならない */
		if ((3 != u_samp) && (4 != u_samp)) {
			std::ostringstream os;
			os	<< "rgb(a) must be 3 or 4samples,but "
				<< u_samp;
			throw std::domain_error(os.str());
		}
		/* ビット数(1サンプルあたり)が、8,16のどれかで
		なければならない */
		switch (bb) {
		case std::numeric_limits<unsigned char>::digits:
		case std::numeric_limits<unsigned short>::digits:
			break;
		default:
			std::ostringstream os;
			os
			<< "rgb(a) must be "
			<< std::numeric_limits<unsigned char>::digits
			<< " or "
			<< std::numeric_limits<unsigned short>::digits
			<< "bits,but "
			<< bb;
			throw std::domain_error(os.str());
		}
		/* RGBRGBRGBの並びでなければならない */
		{
			const unsigned int u_plco = this->planarconfig();
			if (PLANARCONFIG_CONTIG != u_plco) {
				std::ostringstream os;
				os << "TIFFTAG_PLANARCONFIG(" << u_plco
				   << ") is not support";
				throw std::domain_error(os.str());
			}
		}

		break;
	default:
		std::ostringstream os;
		os	<< "TIFFTAG_PHOTOMETRIC(" << u_phot
			<< ") is not support";
		throw std::domain_error(os.str());
	}

	/* 原点位置モード */
	/*
		orientation値が規定値
			ORIENTATION_TOPLEFT
			ORIENTATION_BOTLEFT
			ORIENTATION_TOPRIGHT
			ORIENTATION_BOTRIGHT
			ORIENTATION_LEFTTOP
			ORIENTATION_RIGHTTOP
			ORIENTATION_RIGHTBOT
			ORIENTATION_LEFTBOT
		以外の値、
		 1 std::numeric_limits<unsigned int>::max() --> not defined
		 2 0 --> 古いphotoshopで作成したTIFFデータ
		 3 それ以外の規定値 --> not support
		の場合、
		default値のORIENTATION_TOPLEFTであるとして、
		受け入れることにする 2012-12-14
	*/
#if 0//---------------------------------------------------------------
	const unsigned int u_ori = this->orientation();
	if (std::numeric_limits<unsigned int>::max() == u_ori) {
		throw std::domain_error(
			"TIFFTAG_ORIENTATION is not defined"); }
	switch (u_ori) {
	case 0: /* 古いphotoshopで作成したTIFFデータはゼロになっている
		= ORIENTATION_TOPLEFT-1 */
	case ORIENTATION_TOPLEFT:  /* =1 左上原点0度回転 TIFF,SGI */
	case ORIENTATION_BOTLEFT:  /* =4 左下原点0度回転 OpenGL */
	case ORIENTATION_TOPRIGHT: /* =2 */
	case ORIENTATION_BOTRIGHT: /* =3 左上原点180度回転 */
	case ORIENTATION_LEFTTOP:  /* =5 */
	case ORIENTATION_RIGHTTOP: /* =6 左上原点270度回転 */
	case ORIENTATION_RIGHTBOT: /* =7 */
	case ORIENTATION_LEFTBOT:  /* =8 左上原点90度回転 */
		break;
	default:
		std::ostringstream os;
		os	<< "TIFFTAG_ORIENTATION(" << u_ori
			<< ") is not support";
		throw std::domain_error(os.str());
	}
#endif//--------------------------------------------------------------
}
//------------------------------------------------------------
#if DEBUG_LIBTIFF_PTR
/*
cl -MD -EHa tif_read_handle.cxx tif_error.cxx -wd4819 -DDEBUG_LIBTIFF_PTR -I. -I%HOME%/utility_tools/vc2005md/include %HOME%/utility_tools/vc2005md/tiff-v3.5.7/lib/libtiff.a -Fetes-vc2005md
cl -MD -EHa tif_read_handle.cxx tif_error.cxx -DDEBUG_LIBTIFF_PTR -I. -I%HOME%/utility_tools/vc6md/include %HOME%/utility_tools/vc6md/tiff-v3.5.7/lib/libtiff.a -Fetes-vc6md
g++ -Wall -O tif_read_handle.cxx tif_error.cxx -DDEBUG_LIBTIFF_PTR -I. -I${HOME}/utility_tools/rhel4/include ${HOME}/utility_tools/rhel4/tiff-v3.5.7/lib/libtiff.a -o tes-rhel4
g++ -Wall -O tif_read_handle.cxx tif_error.cxx -DDEBUG_LIBTIFF_PTR -I. -I${HOME}/utility_tools/darwin8/include ${HOME}/utility_tools/darwin8/tiff-v3.5.7/lib/libtiff.a -o tes-dw8
*/
#include <iostream>
int main(int argc,char *argv[])
{
	igs::tif::read_handle cl_r_handle;

	std::cout << "file_path " << argv[1] << std::endl;

	cl_r_handle.v_open(argv[1]);

	std::cout
		<< "he " << cl_r_handle.imagelength() << std::endl
		<< "wi " << cl_r_handle.imagewidth() << std::endl
		<< "ch " << cl_r_handle.samplesperpixel() << std::endl
		<< "bi " << cl_r_handle.bitspersample() << std::endl
		<< "photom " << cl_r_handle.photometric() << std::endl
		<< "compre " << cl_r_handle.compression() << std::endl
		<< "planar " << cl_r_handle.planarconfig() << std::endl;
	if (cl_r_handle.istiled()) {
	 std::cout
	 << "tileheight " << cl_r_handle.tilelength() << std::endl
	 << "tilewidth  " << cl_r_handle.tilewidth() << std::endl;
	} else {
	 std::cout
//	 << "scanlsize  " << cl_r_handle.u_size_scanline() << std::endl
	 << "stripsize  " << TIFFStripSize(cl_r_handle) << std::endl
	 << "stripcount " << TIFFNumberOfStrips(cl_r_handle) << std::endl;
	}
	std::cout
		<< "orient " << cl_r_handle.orientation() << std::endl
		<< "resolu " << cl_r_handle.resolutionunit() << std::endl
		<< "reso_x " << cl_r_handle.xresolution() << std::endl
		<< "reso_y " << cl_r_handle.yresolution() << std::endl
		<< "fillor " << cl_r_handle.fillorder() << std::endl;
	return 0;
}
#endif
