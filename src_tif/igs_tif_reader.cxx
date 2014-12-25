#include <iomanip> /* std::setw */
#include <limits> /* std::numeric_limits */
#include <iostream> /* std::cerr std::cout */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_image_function_invert.h"
#include "igs_tif_extensions.h"
#include "igs_tif_read_handle.h"
#include "igs_tif_read_strip.h"
#include "igs_tif_read_tile.h"
#include "igs_tif_reader.h"

//------------------------------------------------------------
using igs::tif::reader;

/* File��̾ */
const char *const reader::name_ =
	"TIFF(Tagged Image File Format)"
	;
const char *const reader::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
reader::ext_count(void) {
	return sizeof(igs::tif::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
reader::ext_lists(void) {
	return igs::tif::extensions;
}

/* �ե�����إå�������ǧ */
const bool
reader::istag(
	const size_t bytes_count
	,const unsigned char *const tag_array
) {
	if (	(2 <= bytes_count) && (
		(('I' == tag_array[0]) && ('I' == tag_array[1])) ||
		(('M' == tag_array[0]) && ('M' == tag_array[1]))
	)) { return true; }
	return false;
}

/* �Ѱդ��٤������Ⱥ�Ⱦ���Υ����������� */
void
reader::get_size(
	const std::string& file_path,
	size_t& image_bytes,
	size_t& history_bytes
) {
	igs::tif::read_handle r_handler(file_path.c_str());

	image_bytes = r_handler.imagelength() *
		igs::image::byte_of_scanline(
			r_handler.imagewidth(),
			r_handler.samplesperpixel(),
			r_handler.bitspersample()
		);

	history_bytes = 0;
}

/* �ɤ�
	���ｪλ���㳰����throw��(�ǥ��ȥ饯��)����
	����������������ե�������Ĥ���褦�߷פ���
*/
namespace {
 void _v_properties(
	igs::tif::read_handle& handle_ref,
	igs::image::properties& props
 ) {
	props.height	= handle_ref.imagelength();
	props.width	= handle_ref.imagewidth();
	props.channels	= handle_ref.samplesperpixel();
	props.bits	= handle_ref.bitspersample();

	using namespace igs::image;
	orientation::side or_si;
	const unsigned int u_or = handle_ref.orientation();
	if (std::numeric_limits<unsigned int>::max() == u_or) {
				  or_si = orientation::not_defined; }
	else {
	 switch (u_or) {
	 case 0:
	 case ORIENTATION_TOPLEFT: or_si = orientation::topleft; break;
	 case ORIENTATION_TOPRIGHT:or_si = orientation::toprigh; break;
	 case ORIENTATION_BOTRIGHT:or_si = orientation::botrigh; break;
	 case ORIENTATION_BOTLEFT: or_si = orientation::botleft; break;
	 case ORIENTATION_LEFTTOP: or_si = orientation::lefttop; break;
	 case ORIENTATION_RIGHTTOP:or_si = orientation::rightop; break;
	 case ORIENTATION_RIGHTBOT:or_si = orientation::righbot; break;
	 case ORIENTATION_LEFTBOT: or_si = orientation::leftbot; break;
	 default:
		/*
		orientation�ͤ�������
			ORIENTATION_TOPLEFT
			ORIENTATION_BOTLEFT
			ORIENTATION_TOPRIGHT
			ORIENTATION_BOTRIGHT
			ORIENTATION_LEFTTOP
			ORIENTATION_RIGHTTOP
			ORIENTATION_RIGHTBOT
			ORIENTATION_LEFTBOT
		�ʳ����͡�
		 1 std::numeric_limits<unsigned int>::max() --> not defined
		 2 0 --> �Ť�photoshop�Ǻ�������TIFF�ǡ���
		 3 ����ʳ��ε����� --> not support
		�ξ�硢
		default�ͤ�ORIENTATION_TOPLEFT�Ǥ���Ȥ��ơ�
		��������뤳�Ȥˤ��� 2012-12-14
		*/
		/******std::ostringstream os;
		os	<< "TIFFTAG_ORIENTATION(" << u_or
			<< ") is not support";
		throw std::domain_error(os.str());******/

		or_si = orientation::topleft;
	 }
	}
	props.orie_side = or_si;

	resolution::unit re_un;
	const unsigned int u_re = handle_ref.resolutionunit();
	if (std::numeric_limits<unsigned int>::max() == u_re) {
				  re_un = resolution::not_defined; }
	else {
	 switch (u_re) {
	 case RESUNIT_NONE:       re_un = resolution::nothing;    break;
	 case RESUNIT_INCH:       re_un = resolution::inch;       break;
	 case RESUNIT_CENTIMETER: re_un = resolution::centimeter; break;
	 default:
		std::ostringstream os;
		os	<< "TIFFTAG_RESOLUTIONUNIT(" << u_re
			<< ") is not support";
		throw std::domain_error(os.str());
	 }
	}
	props.reso_unit = re_un;

	props.reso_x = handle_ref.xresolution();
	props.reso_y = handle_ref.yresolution();
 }
 void _v_history(
	igs::tif::read_handle& handle_ref,
	std::string& history_str
 ) {
 }
}

#if defined TES_SPEED
#include "tes_rs.h"
#endif

void
reader::get_data(
	const std::string& file_path
	, igs::image::properties& props
	, const size_t image_bytes
	, unsigned char *image_array
	, std::string& history_str
	, bool& break_sw /* ����Switch��������ture������ */
	, const size_t segment_bytes
			/* ʬ���ɹ��θġ�������(tif�ʳ��ǻ��Ѥ���) */
) {
#if defined TES_SPEED
// igs::test::speed::... is in tes_rs_main.cxx
igs::test::speed::start();
#endif
	/* �ե����볫�� */
	igs::tif::read_handle r_handler(file_path.c_str());
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �إå������å�������Υ��å� */
	r_handler.check_parameters();
	_v_properties(r_handler,props);
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* history�ǡ������ɤ� */
	if (0 < history_str.size()) {
		_v_history(r_handler,history_str);
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �����ǡ������ɤ� */
    if (0 < image_bytes) {
	/* �ե����뤫���ɤߡ�OpenGL�������ǡ������Ѵ����� */
	if (r_handler.istiled()) {
		igs::tif::read_tile cl_r_tile(
			r_handler, image_array, image_bytes
		);
		for (unsigned int ii=0; ii<cl_r_tile.count(); ++ii) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			cl_r_tile.segment_exec(r_handler);
		}
	} else {
		igs::tif::read_strip cl_r_strip(
			r_handler, image_array, image_bytes
		);
		for (unsigned int ii=0; ii<cl_r_strip.count(); ++ii) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			cl_r_strip.segment_exec(r_handler);
		}
	}
	/* �ե�����奼��(MIN)����ʤ顢���ȿž��������(MIN)����� */
	if (PHOTOMETRIC_MINISWHITE==r_handler.photometric()) {
		igs::image::function::invert(
			r_handler.bitspersample(),
			image_bytes,
			image_array
		);
	}
    }
#if defined TES_SPEED
igs::test::speed::stop();
#endif
}
