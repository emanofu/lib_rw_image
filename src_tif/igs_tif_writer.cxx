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

/* File��̾ */
const char *const writer::name_ =
	"TIFF(Tagged Image File Format)"
	;
const char *const writer::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
writer::ext_count(void) {
	return sizeof(igs::tif::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
writer::ext_lists(void) {
	return igs::tif::extensions;
}

/* ����̾�ꥹ�ȼ¥ǡ��� */
namespace igs {
 namespace tif { /* tiff.h�ȹ�碌�뤳�� */
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
/* ���̼���� */
const unsigned int
writer::compression_count(void) {
	return sizeof(igs::tif::compressions)/sizeof(char *);
}
/* ����̾�ꥹ�� */
const char *const *const
writer::compression_lists(void) {
	return igs::tif::compressions;
}

/* ��¸���Բĥ����å� */
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
	/* �����θ��� */
	/*
	TIFF 6.0 Specification��ꡢ
	TIFFTAG_ORIENTATION�ϡ�
		"Support for orientations other than 1 is not
		a Baseline TIFF requirement."
		"1�ʳ��������Υ��ݡ��Ȥϡ�Baseline TIFF���׷�ǤϤʤ�"
		Default is 1(TL(topleft)). ���ξ���������
		OpenGL�β����� 4(BL).
	��������TIFF�Ϥ��٤Ƥθ����򥵥ݡ��Ȥ���
	*/

	/* ������ */
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

/* ��¸
���ｪλ���㳰������catch����
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
	/* �إå����� --------------------------------- */
	/* http://bb/watch.impress.co.jp/cda/bbword/15612.html
		RGB�ե륫�顼�ξ�硢Tag�Ȥ������ꤹ�٤����ܤ�
		�ʲ���12���ܤ����ǡ�
		����ʳ���Tag�ϥ��ץ���󰷤��ˤʤ�ޤ���
		��ImageLength
		��ImageWidth
		��SamplesPerPixel
		��BitsPerSample

		��Compression
		��PhotometricInterpretation
		��XResolution
		��YResolution
		��ResolutionUnit

		��StripOffsets
		��RowsPerStrip
		��StripByteCounts
	*/
	igs::tif::write_handle w_handler(file_path.c_str());

	/* --- �礭�� --------------------------------- */
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

	/* --- �����θ��� ----------------------------- */
	/*
	TIFF 6.0 Specification��ꡢ
	"Support for orientations other than 1 is not
		a Baseline TIFF requirement."
	TIFFTAG_ORIENTATION��1(topleft)��default�ǡ����ξ���������???
	*/
	unsigned short us_orient=0;
	switch (props.orie_side) {
	case orientation::not_defined:
	/* �������ؼ����Ƥʤ���硢������default���ꤹ�� 2012-12-14 */
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
	/* default����⤢������¸���� 2012-12-14 */
	 w_handler.set_uint16_throw( us_orient,
			 TIFFTAG_ORIENTATION,
			"TIFFTAG_ORIENTATION" );
	/***}***/

	/* --- �������¤� ----------------------------- */
	/*
	TIFFTAG_PLANARCONFIG��PLANARCONFIG_CONTIG��default
	Write���ϻ��꤬ɬ��
	*/
	w_handler.set_uint16_throw(  PLANARCONFIG_CONTIG,
			 TIFFTAG_PLANARCONFIG,
			"TIFFTAG_PLANARCONFIG");

	/* --- �����ο���� --------------------------- */
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW���� */
		/*
	��Υ�����Ĵ�����ե�����Ǥϡ�
	Photoshop 8.0.1��������Ͳ���������¸����ȡ�
	�������1���(PHOTOMETRIC_MINISWHITE)�Ȥʤ�
	2007-12

	��������1����(PHOTOMETRIC_MINISBLACK)�Ȥ�����¸����ȡ�
	Photoshop CS4(11.0.1)��Tag���Τ��������ȿž���Ƥ��ޤ��Τǡ�
	PHOTOMETRIC_MINISWHITE����¸���뤳�Ȥ��ѹ�����
	2010-2-8
		*/
		/* ��β����ϥ������Ȥ����ݻ�->������Ȥ�����¸ */
		w_handler.set_uint16_throw( PHOTOMETRIC_MINISWHITE,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC");
	} else
	if ((1==props.channels) && (1<props.bits)) {/* Grayscale */
		w_handler.set_uint16_throw( PHOTOMETRIC_MINISBLACK,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC");
	} else
	if (1 < props.channels) {/* �ե륫��(RGB,RGBA)���� */
		w_handler.set_uint16_throw( PHOTOMETRIC_RGB,
			 TIFFTAG_PHOTOMETRIC,
			"TIFFTAG_PHOTOMETRIC" );
	}

	/* --- ɽ�������� ----------------------------- */
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

	/* --- ��¸���̤���� ------------------------- */
	/* ���̼��������
		monoBW�λ��Ȱʳ��ǰ��̥����פ��㤦�ΤǼ�ưŪ���ѹ� */
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
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW���� */
		/*
			monoBW�ΰ�����¸�ǡ�
				COMPRESSION_PACKBITS or
				COMPRESSION_LZW
			�ȤʤäƤ�����硢
				COMPRESSION_CCITTRLE
			�Ȥ�����¸���롣
		*/
		if ((COMPRESSION_PACKBITS == comp_num)
		||  (COMPRESSION_LZW == comp_num)) {
			comp_num = COMPRESSION_CCITTRLE;
		}
		w_handler.set_uint16_throw( comp_num,
			 TIFFTAG_COMPRESSION,
			"TIFFTAG_COMPRESSION" );
	} else {/* Grayscale, �ե륫�顼(RGB, RGBA)���� */
		/*
			Grayscale or RGB(A)�ΰ�����¸�ǡ�
				COMPRESSION_CCITTRLE
			�ȤʤäƤ�����硢
				COMPRESSION_LZW
			�Ȥ�����¸���롣
		*/
		if (COMPRESSION_CCITTRLE == comp_num) {
			comp_num = COMPRESSION_LZW;
		}
		w_handler.set_uint16_throw( comp_num,
			 TIFFTAG_COMPRESSION,
			"TIFFTAG_COMPRESSION" );
	}

/* -------------------------------------------------
	���ƻ���1 TIFFTAG_PREDICTOR
	TIFFSetField(p_tif,TIFFTAG_COMPRESSION,COMPRESSION_LZW);
	TIFFSetField(p_tif,TIFFTAG_COMPRESSION,COMPRESSION_DEFLATE);
	��ͽ¬ư��(���̸�Ψ�򤢤���)?
		TIFFSetField(w_handler,TIFFTAG_PREDICTOR,1);�ʤ�?
		TIFFSetField(w_handler,TIFFTAG_PREDICTOR,2);����?
		default is 1???
	TIFFTAG_PREDICTOR��������(2004.09.14 tue)
	-----------------------+---------------+---------------
	TIFF RGB��16btis sampling�����ΤȤ�����¸�����ξɾ�
				lzw prediction scheme
				1		2
	-----------------------+---------------+---------------
	libtiff version		��ֿ�	������	��ֿ�	������
	-----------------------+-------+-------+-------+-------
	libtiff-v3.5.7(rh9)	����	����	����	�Ǥ�
	libtiff-v3.6.1(rhel4)	�Ǥʤ�	����	�Ǥʤ�	�Ǥ�
	-----------------------+-------+-------+-------+-------
	"lzw_prediction_scheme = 2;"��16bits�����ǻȤ��ʤ���


	���ƻ���2 TIFFTAG_FILLORDER
	MonoBW�ˤ�
	TIFFSetField(p_tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	�ϻ��ꤤ��Τ�?


	���ƻ���3 TIFFTAG_ROWSPERSTRIP
	if (1 == props.bits) {
		TIFFSetField( w_handler, TIFFTAG_ROWSPERSTRIP,
			static_cast<uint32>(props.height) );
	}
	else {
		uint32 rowsperstrip = (uint32)-1;

		���줬�ʤ��ȡ�
		���ｪλ����Τˡ��ɤ�ʤ��ե�����ˤʤ�
		(2000.04.05)
		TIFFSetField( w_handler,
			TIFFTAG_ROWSPERSTRIP,
			TIFFDefaultStripSize(
				w_handler, rowsperstrip
			)
		);
	}


	���ƻ���4 TIFFTAG_SUBFILETYPE
	Example about imginfo : Page Size (x,y,z,c):    256, ?, 1, 3
	TIFFSetField( w_handler, TIFFTAG_SUBFILETYPE, 0);
	For same to photoshop_v3.0.1(IRIX)
------------------------------------------------- */

	/* --- ������¸ ------------------------------- */
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW���� */
		/* �����->������(PHOTOMETRIC_MINISWHITE)�Ȥ�����¸ */
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
	if ((1==props.channels) && (1==props.bits)) {/* MonoBW���� */
		/* ������(PHOTOMETRIC_MINISWHITE)->��������᤹ */
		unsigned char *uchar_array =
			const_cast<unsigned char *>(image_array);
		for (size_t ii=0; ii<image_bytes; ++ii) {
			uchar_array[ii] = ~image_array[ii];
		}
	}
}
