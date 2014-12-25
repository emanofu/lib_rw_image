#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_tif_extensions.h"
#include "igs_tif_write_handle.h"
#include "igs_tif_write_strip.h"
#include "igs_tif_writer.h"

//------------------------------------------------------------
// using igs::tif::write::properties;

using igs::tif::writer;
writer::writer()
{}

/* File書式名 */
const char *const writer::name_ =
	"TIFF(Tagged Image File Format)"
	;
const char *const writer::name(void) const { return this->name_; }

/* 拡張子数 */
const unsigned int
writer::ext_count(void) {
	return sizeof(igs::tif::extensions)/sizeof(char *);
}
/* 拡張子名リスト */
const char *const *const
writer::ext_lists(void) {
	return igs::tif::extensions;
}

/* 圧縮名リスト実データ */
namespace igs {
 namespace tif { /* tiff.hと合わせること */
  const char *const compressions[] = {
	"NONE(dump mode)",
	"LZW(Lempel-Ziv & Welch)",
	"CCITTRLE(CCITT modified Huffman RLE)",
	"PACKBITS(Macintosh RLE)",
  };
/*
0 -> COMPRESSION_NONE          1  dump mode
1 -> COMPRESSION_LZW           5  Lempel-Ziv & Welch        ,Not  monoBW
2 -> COMPRESSION_CCITTRLE      2  CCITT modified Huffman RLE,Only monoBW
3 -> COMPRESSION_PACKBITS  32773  Macintosh RLE             ,Not  monoBW
*/
 }
}
/* 圧縮種類数 */
const unsigned int
writer::compression_count(void) {
	return sizeof(igs::tif::compressions)/sizeof(char *);
}
/* 圧縮名リスト */
const char *const *const
writer::compression_lists(void) {
	return igs::tif::compressions;
}

/* 保存可不可チェック */
/*
	.tif  RGBA/RGB/Grayscale  16/8/1(MonoBW)bits
*/
const bool writer::error_from_properties(
	const igs::image::properties& props,
	const int ext_number,
	std::ostringstream& error_or_warning_msg
) {

	bool error_sw = false;

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
		const unsigned int u_uc_bi =
			std::numeric_limits<unsigned char>::digits;
		if ( (props.bits != 1)
		&&   (props.bits != u_uc_bi*sizeof(unsigned char))
		&&   (props.bits != u_uc_bi*sizeof(unsigned short))
		) {
			error_or_warning_msg
				<< "Error : sampling bits("
				<< props.bits
				<< ") Must be 1 or "
				<< u_uc_bi*sizeof(unsigned char)
				<< " or "
				<< u_uc_bi*sizeof(unsigned short)
				<< "\n";
			error_sw = true;
		}
	/* 画像の向き */
	/*
	TIFF 6.0 Specificationより、
	TIFFTAG_ORIENTATIONは、
		"Support for orientations other than 1 is not
		a Baseline TIFF requirement."
		"1以外の方向のサポートは、Baseline TIFFの要件ではない"
		Default is 1(TL(topleft)). この場合指定不用
		OpenGLの画像は 4(BL).
	しかし、TIFFはすべての向きをサポートする
	*/

	/* 解像度 */
	switch (props.reso_unit) {
	case igs::image::resolution::not_defined:
		error_or_warning_msg
		<< "Warning : resolution unit is not defined\n";
		break;
	case igs::image::resolution::centimeter:
		error_or_warning_msg
		<< "Error : resolution unit is defined centimeter\n";
		error_sw = true;
		break;
	case igs::image::resolution::millimeter:
		error_or_warning_msg
		<< "Error : resolution unit is defined milliimeter\n";
		error_sw = true;
		break;
	case igs::image::resolution::meter:
	case igs::image::resolution::inch:
	case igs::image::resolution::nothing:
		break;
	}
	return error_sw;
}

/* 保存
正常終了、例外処理のcatch、で
内部メモリ解放し、ファイルを閉じること
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
	/* ヘッダ設定 --------------------------------- */
	/* http://bb/watch.impress.co.jp/cda/bbword/15612.html
		RGBフルカラーの場合、Tagとして設定すべき項目は
		以下の12項目だけで、
		それ以外のTagはオプション扱いになります。
		・ImageLength
		・ImageWidth
		・SamplesPerPixel
		・BitsPerSample

		・Compression
		・PhotometricInterpretation
		・XResolution
		・YResolution
		・ResolutionUnit

		・StripOffsets
		・RowsPerStrip
		・StripByteCounts
	*/
	igs::tif::write_handle w_handler(file_path.c_str());

	/* --- 大きさ --------------------------------- */
	w_handler.set_uint32_throw( props.height,
			 TIFFTAG_IMAGELENGTH,
			"TIFFTAG_IMAGELENGTH" );
	w_handler.set_uint32_throw( props.width,
			 TIFFTAG_IMAGEWIDTH,
			"TIFFTAG_IMAGEWIDTH" );
	w_handler.set_uint16_throw( props.channels,
			 TIFFTAG_SAMPLESPERPIXEL,
			"TIFFTAG_SAMPLESPERPIXEL" );
	w_handler.set_uint16_throw( props.bits,
			 TIFFTAG_BITSPERSAMPLE,
			"TIFFTAG_BITSPERSAMPLE" );

	using namespace igs::image;

	/* --- 画像の向き ----------------------------- */
	/*
	TIFF 6.0 Specificationより、
	"Support for orientations other than 1 is not
		a Baseline TIFF requirement."
	TIFFTAG_ORIENTATIONは1(topleft)がdefaultで、その場合指定不用???
	*/
	unsigned short us_orient=0;
	switch (props.orie_side) {
	case orientation::not_defined:
	/* 方向が指示してない場合、あえてdefault指定する 2012-12-14 */
				   us_orient=ORIENTATION_TOPLEFT; break;
	case orientation::topleft: us_orient=ORIENTATION_TOPLEFT; break;
	case orientation::toprigh: us_orient=ORIENTATION_TOPRIGHT;break;
	case orientation::botrigh: us_orient=ORIENTATION_BOTRIGHT;break;
	case orientation::botleft: us_orient=ORIENTATION_BOTLEFT; break;
	case orientation::lefttop: us_orient=ORIENTATION_LEFTTOP; break;
	case orientation::rightop: us_orient=ORIENTATION_RIGHTTOP;break;
	case orientation::righbot: us_orient=ORIENTATION_RIGHTBOT;break;
	case orientation::leftbot: us_orient=ORIENTATION_LEFTBOT; break;
	}
	/***if ((orientation::not_defined != props.orie_side) &&
	    (orientation::topleft     != props.orie_side)) {***/
	/* default指定もあえて保存する 2012-12-14 */
	 w_handler.set_uint16_throw( us_orient,
			 TIFFTAG_ORIENTATION,
			"TIFFTAG_ORIENTATION" );
	/***}***/

	/* --- 画像の並び ----------------------------- */
	/*
	TIFFTAG_PLANARCONFIGはPLANARCONFIG_CONTIGがdefault
	Write時は指定が必要
	*/
	w_handler.set_uint16_throw(  PLANARCONFIG_CONTIG,
			 TIFFTAG_PLANARCONFIG,
			"TIFFTAG_PLANARCONFIG");

	/* --- 画像の色定義 --------------------------- */
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW画像 */
		/*
	モノクロ２階調画像ファイルでは、
	Photoshop 8.0.1で白黒２値化画像を保存すると、
	ゼロを白、1を黒(PHOTOMETRIC_MINISWHITE)となる
	2007-12

	ゼロを黒、1を白(PHOTOMETRIC_MINISBLACK)として保存すると、
	Photoshop CS4(11.0.1)がTagを感知せず、白黒反転してしまうので、
	PHOTOMETRIC_MINISWHITEで保存することに変更する
	2010-2-8
		*/
		/* モノ画像はゼロが黒として保持->ゼロが白として保存 */
		w_handler.set_uint16_throw( PHOTOMETRIC_MINISWHITE,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC");
	} else
	if ((1==props.channels) && (1<props.bits)) {/* Grayscale */
		w_handler.set_uint16_throw( PHOTOMETRIC_MINISBLACK,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC");
	} else
	if (1 < props.channels) {/* フルカラ(RGB,RGBA)画像 */
		w_handler.set_uint16_throw( PHOTOMETRIC_RGB,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC" );
	}

	/* --- 表示解像度 ----------------------------- */
	uint16 ui16_un = RESUNIT_NONE;
	switch (props.reso_unit) {
	case resolution::not_defined: break;
	case resolution::nothing:     break;
	case resolution::inch:       ui16_un=RESUNIT_INCH;       break;
	case resolution::centimeter: ui16_un=RESUNIT_CENTIMETER; break;
	case resolution::millimeter:  break;
	case resolution::meter:       break;
	}
	if ( resolution::not_defined != props.reso_unit) {
		w_handler.set_uint16_throw( ui16_un,
			 TIFFTAG_RESOLUTIONUNIT,
			"TIFFTAG_RESOLUTIONUNIT" );
		w_handler.set_float_throw( props.reso_x,
			 TIFFTAG_XRESOLUTION,
			"TIFFTAG_XRESOLUTION" );
		w_handler.set_float_throw( props.reso_y,
			 TIFFTAG_YRESOLUTION,
			"TIFFTAG_YRESOLUTION" );
	}

	/* --- 保存圧縮の定義 ------------------------- */
	/* 圧縮種類を設定
		monoBWの時と以外で圧縮タイプが違うので自動的に変更 */
	unsigned int comp_num = 0;
	switch (compression_number) {
	case 0: comp_num = COMPRESSION_NONE;     break;
	case 1: comp_num = COMPRESSION_LZW;      break;
	case 2: comp_num = COMPRESSION_CCITTRLE; break;
	case 3: comp_num = COMPRESSION_PACKBITS; break;
	}
/*
0 -> COMPRESSION_NONE          1  dump mode
1 -> COMPRESSION_LZW           5  Lempel-Ziv & Welch        ,Not  monoBW
2 -> COMPRESSION_CCITTRLE      2  CCITT modified Huffman RLE,Only monoBW
3 -> COMPRESSION_PACKBITS  32773  Macintosh RLE             ,Not  monoBW
*/
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW画像 */
		/*
			monoBWの圧縮保存で、
				COMPRESSION_PACKBITS or
				COMPRESSION_LZW
			となっていた場合、
				COMPRESSION_CCITTRLE
			として保存する。
		*/
		if ((COMPRESSION_PACKBITS == comp_num)
		||  (COMPRESSION_LZW == comp_num)) {
			comp_num = COMPRESSION_CCITTRLE;
		}
		w_handler.set_uint16_throw( comp_num,
			 TIFFTAG_COMPRESSION,
			"TIFFTAG_COMPRESSION" );
	} else {/* Grayscale, フルカラー(RGB, RGBA)画像 */
		/*
			Grayscale or RGB(A)の圧縮保存で、
				COMPRESSION_CCITTRLE
			となっていた場合、
				COMPRESSION_LZW
			として保存する。
		*/
		if (COMPRESSION_CCITTRLE == comp_num) {
			comp_num = COMPRESSION_LZW;
		}
		w_handler.set_uint16_throw( comp_num,
			 TIFFTAG_COMPRESSION,
			"TIFFTAG_COMPRESSION" );
	}

/* -------------------------------------------------
	懸案事項1 TIFFTAG_PREDICTOR
	TIFFSetField(p_tif,TIFFTAG_COMPRESSION,COMPRESSION_LZW);
	TIFFSetField(p_tif,TIFFTAG_COMPRESSION,COMPRESSION_DEFLATE);
	で予測動作(圧縮効率をあげる)?
		TIFFSetField(w_handler,TIFFTAG_PREDICTOR,1);なし?
		TIFFSetField(w_handler,TIFFTAG_PREDICTOR,2);あり?
		default is 1???
	TIFFTAG_PREDICTORの問題点(2004.09.14 tue)
	-----------------------+---------------+---------------
	TIFF RGB各16btis sampling画像のときの保存画像の症状
				lzw prediction scheme
				1		2
	-----------------------+---------------+---------------
	libtiff version		中間色	一部黒	中間色	一部黒
	-----------------------+-------+-------+-------+-------
	libtiff-v3.5.7(rh9)	正常	正常	正常	でる
	libtiff-v3.6.1(rhel4)	でない	正常	でない	でる
	-----------------------+-------+-------+-------+-------
	"lzw_prediction_scheme = 2;"は16bits画像で使えない。


	懸案事項2 TIFFTAG_FILLORDER
	MonoBWにて
	TIFFSetField(p_tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	は指定いるのか?


	懸案事項3 TIFFTAG_ROWSPERSTRIP
	if (1 == props.bits) {
		TIFFSetField( w_handler, TIFFTAG_ROWSPERSTRIP,
			static_cast<uint32>(props.height) );
	}
	else {
		uint32 rowsperstrip = (uint32)-1;

		これがないと、
		正常終了するのに、読めないファイルになる
		(2000.04.05)
		TIFFSetField( w_handler,
			TIFFTAG_ROWSPERSTRIP,
			TIFFDefaultStripSize(
				w_handler, rowsperstrip
			)
		);
	}


	懸案事項4 TIFFTAG_SUBFILETYPE
	Example about imginfo : Page Size (x,y,z,c):    256, ?, 1, 3
	TIFFSetField( w_handler, TIFFTAG_SUBFILETYPE, 0);
	For same to photoshop_v3.0.1(IRIX)
------------------------------------------------- */

	/* --- 画像保存 ------------------------------- */
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW画像 */
		/* ゼロ黒->ゼロ白(PHOTOMETRIC_MINISWHITE)として保存 */
		unsigned char *uchar_array =
			const_cast<unsigned char *>(image_array);
		for (size_t ii = 0; ii < image_bytes; ++ii) {
			uchar_array[ii] = ~image_array[ii];
		}
	}
	igs::tif::write_strip w_strip(
		w_handler, image_array, image_bytes
	);
	for (unsigned int ii=0; ii < w_strip.count(); ++ii) {
		w_strip.segment_exec(w_handler);
	}
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW画像 */
		/* ゼロ白(PHOTOMETRIC_MINISWHITE)->ゼロ黒に戻す */
		unsigned char *uchar_array =
			const_cast<unsigned char *>(image_array);
		for (size_t ii=0; ii<image_bytes; ++ii) {
			uchar_array[ii] = ~image_array[ii];
		}
	}
}
