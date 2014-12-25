// #include <iomanip> /* std::setw */
// #include <iostream> /* std::cout */
#include <vector> /* std::vector */
#include <sstream> /* std::ostringstream */
#include <limits> /* std::numeric_limits */
#include <stdexcept> /* std::domain_error(-) */

#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_sgi_extensions.h"
#include "igs_sgi_decode_rle.h"
#include "igs_sgi_decode_verbatim.h"
#include "igs_sgi_read_handle.h"
#include "igs_sgi_reader.h"

/* pixel������
��������	pixel channels	sampleing bits
---------------+---------------+------------------------------------
RGBA		4		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
RGB		3		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
grayscale	1		by_bi * sizeof(unsigned char)
				by_bi * sizeof(unsigned short)
---------------+---------------+------------------------------------
(��1) by_bi means std::numeric_limits<unsigned char>::digits
*/
/*----------------------------------------------------------*/
using igs::sgi::reader;

/* File��̾ */
const char *const reader::name_ =
	"SGI(Silicon Graphics Image)/RGB File Format"
	;
const char *const reader::name(void) const { return this->name_; }

/* ��ĥ�ҿ� */
const unsigned int
reader::ext_count(void) {
	return sizeof(igs::sgi::extensions)/sizeof(char *);
}
/* ��ĥ��̾�ꥹ�� */
const char *const *const
reader::ext_lists(void) {
	return igs::sgi::extensions;
}

/* �ե�����إå�������ǧ */
const bool
reader::istag(
	const size_t bytes_count
	, const unsigned char *const tag_array
) {
	bool byte_swap_sw;
	return igs::sgi::read_handle::istag(
		bytes_count,tag_array,byte_swap_sw);
}

/* �Ѱդ��٤������Ⱥ�Ⱦ���Υ����������� */
void
reader::get_size(
	const std::string& file_path
	, size_t& image_bytes
	, size_t& history_bytes
) {
	igs::sgi::read_handle::get_size(
		file_path,image_bytes,history_bytes
	);
}

/* �ɤ�
	���ｪλ���㳰����throw��(�ǥ��ȥ饯��)����
	����������������ե�������Ĥ��뤳��
*/
namespace {
 void header_to_prop(
	igs::sgi::header::parameters *header_params_ptr,
	igs::image::properties &props
 ) {
	props.height  = header_params_ptr->us_ysize;
	props.width   = header_params_ptr->us_xsize;
	props.channels= header_params_ptr->us_zsize;
	props.bits    = header_params_ptr->c_bytes_per_ch *
			std::numeric_limits<unsigned char>::digits;
	if (igs::image::orientation::topleft != props.orie_side) {
		props.orie_side = igs::image::orientation::botleft;
			/* SGI�����ե������θ������֤Ϻ��� */
	}
	igs::sgi::read_handle::get_reso(
		header_params_ptr->ca_imagename,
		props.reso_unit,
		props.reso_x,
		props.reso_y
	);
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
	 //cl_fp.read(&file_image.at(0),file_image.size());
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
	igs::sgi::header::parameters *header_params_ptr=0;
	unsigned long *seek_set_ptr=0;
	unsigned long *bytes_size_ptr=0;
	unsigned char *file_image_ptr=0;
	char *history_ptr=0;
	igs::sgi::read_handle::get_data(
		file_image.size()
		, &file_image.at(0)
		, byte_swap_sw
		, header_params_ptr
		, seek_set_ptr
		, bytes_size_ptr
		, file_image_ptr
		, history_ptr
	);

	/* �ɤ߹��������������å� */
	igs::sgi::read_handle::check_parameters(header_params_ptr);

	/* ������������ */
 	header_to_prop( header_params_ptr, props);

	/* ��������ǧ */
	if (image_bytes < (
		static_cast<size_t>(header_params_ptr->us_ysize)
		* header_params_ptr->us_xsize
		* header_params_ptr->us_zsize
		* header_params_ptr->c_bytes_per_ch
	)) {
		std::ostringstream os;
		os	<< "memory size<"
			<< image_bytes
			<< "> is less than h<"
			<< static_cast<int>(header_params_ptr->us_ysize)
			<< "> x w<"
			<< static_cast<int>(header_params_ptr->us_xsize)
			<< "> x ch<"
			<< static_cast<int>(header_params_ptr->us_zsize)
			<< "> x by<"
			<< static_cast<int>(header_params_ptr->c_bytes_per_ch)
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
	using namespace igs::sgi::header::compression;
	if (verbatim == header_params_ptr->c_storage) {
		igs::sgi::decode_verbatim decoder(
			file_image_ptr,	// source
			image_array,	// destonation
	static_cast<unsigned int>(header_params_ptr->us_ysize),
	static_cast<unsigned int>(header_params_ptr->us_xsize),
	static_cast<unsigned int>(header_params_ptr->us_zsize),
	static_cast<unsigned int>(header_params_ptr->c_bytes_per_ch)
		);
		bool rvs_sw = false;
		if (igs::image::orientation::topleft==props.orie_side) {
			rvs_sw = true;
		}
		for (int zz = 0; zz < props.channels; ++zz) {
		 for (int yy = 0; yy < props.height; ++yy) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			decoder.segment_exec(yy,zz, rvs_sw);
		 }
		}
	} else {
		igs::sgi::decode_rle decoder(
			&file_image.at(0),
			seek_set_ptr,
			bytes_size_ptr,
			image_array, // destonation
	static_cast<int>(image_bytes),  // destonation
	static_cast<int>(header_params_ptr->us_ysize),
	static_cast<int>(header_params_ptr->us_xsize),
	static_cast<int>(header_params_ptr->us_zsize),
	static_cast<int>(header_params_ptr->c_bytes_per_ch)
		);
		bool rvs_sw = false;
		if (igs::image::orientation::topleft==props.orie_side) {
			rvs_sw = true;
		}
		for (int zz = 0; zz < props.channels; ++zz) {
		 for (int yy = 0; yy < props.height; ++yy) {
			if (true == break_sw) { // Cancel
				break_sw = false;
				return;
			}
			decoder.segment_exec(yy,zz, rvs_sw);
		 }
		}
	}
	if (0 != history_ptr) {
		history_str += history_ptr;
	}
#if defined TES_SPEED
igs::test::speed::stop();
#endif
}
