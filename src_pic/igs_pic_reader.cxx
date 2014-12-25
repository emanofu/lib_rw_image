#include <iomanip> /* std::setw */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include <limits> /* std::numeric_limits */
#include "igs_resource_irw.h"
#include "igs_pic_extensions.h"
#include "igs_pic_decode.h"
#include "igs_pic_read_handle.h"
#include "igs_pic_reader.h"

/* pixel������
��������	pixel channels	sampleing bits
---------------+---------------+--------------
RGBA		4		by_bi * sizeof(unsigned char)
RGB		3		by_bi * sizeof(unsigned char)
grayscale	1		by_bi * sizeof(unsigned char)
---------------+---------------+--------------
(��1) by_bi means std::numeric_limits<unsigned char>::digits
*/
/*----------------------------------------------------------*/
using igs::pic::reader;

/* File��̾ */
const char *const reader::name_ = "Softimage File Format";
const char *const reader::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
reader::ext_count(void) {
	return sizeof(igs::pic::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
reader::ext_lists(void) {
	return igs::pic::extensions;
}

/* �ե�����إå�������ǧ */
const bool
reader::istag(
	const size_t bytes_count
	,const unsigned char *const tag_array
) {
	bool byte_swap_sw;
	return igs::pic::read_handle::istag(
		static_cast<unsigned int>(bytes_count)
		,tag_array,byte_swap_sw
	);
}

/* �Ѱդ��٤������Ⱥ�Ⱦ���Υ����������� */
void
reader::get_size(
	const std::string& file_path,
	size_t& image_bytes,
	size_t& history_bytes
) {
	unsigned int imagebytes=0;
	igs::pic::read_handle::get_size(file_path,imagebytes);
	history_bytes = 0;
	image_bytes = imagebytes;
}

/* �ɤ�
	���ｪλ���㳰������throw��(�ǥ��ȥ饯��)����
	����������������ե�������Ĥ���褦�߷פ���
*/
namespace {
 void header_to_prop(
	igs::pic::header::parameters *header_ptr,
	unsigned int channel_size,
	igs::image::properties &props
 ) {
	props.height    = header_ptr->us_height;
	props.width     = header_ptr->us_width;
	props.channels  = channel_size;
	props.bits      = std::numeric_limits<unsigned char>::digits;
	props.orie_side = igs::image::orientation::topleft;
			/* pic�����ե������θ������֤Ϻ��� Fix */

	props.reso_unit = igs::image::resolution::not_defined;/* Fix */
	props.reso_x    = 0.0;
	props.reso_y    = 0.0;
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
	history_str; /* for stopping warning C4100 */
#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �ե����륤�᡼���ѥ������ */
	std::vector<unsigned char> file_image;
	file_image.resize(igs::resource::bytes_of_file(file_path.c_str()));
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �ե����뤽�Τޤ��ɤ� */
	{
	 igs::resource::ifbinary cl_fp(file_path.c_str());
	 // cl_fp.read(&file_image.at(0),file_image.size());
	 const size_t fsiz = file_image.size();
	 size_t siz = segment_bytes;
	 for (size_t pos = 0; pos < fsiz; pos += segment_bytes) {
	  if (true == break_sw) { // Cancel
		break_sw = false;
		return;
	  }
	  if ((fsiz-pos) < segment_bytes) { siz=fsiz-pos; } // last
	  cl_fp.read(&file_image.at(pos),siz);
	 }
	 cl_fp.close();
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �����Ѵ����� */
	bool byte_swap_sw;
	igs::pic::header::parameters *header_ptr;
	unsigned int packet_size;
	igs::pic::header::channel_packet *packet_ptr;
	unsigned int channel_size;
	unsigned char *fimage_ptr;
	igs::pic::read_handle::get_data(
		&file_image.at(0),
		byte_swap_sw,
		header_ptr,
		packet_ptr,
		packet_size,
		channel_size,
		fimage_ptr
	);

	/* ������������ */
 	header_to_prop( header_ptr, channel_size, props );

	/* ��������ǧ */
	if (image_bytes < (static_cast<size_t>(header_ptr->us_height)
		* header_ptr->us_width * channel_size
	)) {
		std::ostringstream os;
		os	<< "memory size<"
			<< image_bytes
			<< "> is less than h<"
			<< header_ptr->us_height
			<< "> x w<"
			<< header_ptr->us_width
			<< "> x ch<"
			<< channel_size
			<< ">"
			;
		throw std::domain_error(os.str());
	}

#if defined TES_SPEED
igs::test::speed::stop();
#endif

#if defined TES_SPEED
igs::test::speed::start();
#endif
	/* �ɤ߹�����ǡ�����OpenGL�������ǡ������Ѵ����� */
	igs::pic::decode cl_decode(
		fimage_ptr,
		header_ptr->us_height,
		header_ptr->us_width,
		channel_size,
		image_array
	);
	for (unsigned int ii = 0; ii < cl_decode.count();++ii) {
		if (true == break_sw) { // Cancel
			break_sw = false;
			return;
		}
		cl_decode.segment_exec(
			packet_size,
			packet_ptr,
			byte_swap_sw
		);
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif
}

