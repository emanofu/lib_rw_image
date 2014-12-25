#include <iostream> /* std::cout */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_image_function_invert.h"
#include "igs_tif_error.h"
#include "igs_tif_read_strip.h"

/* strip�ɤ߹��� */
//------------------------------------------------------------
using igs::tif::read_strip;
/*
 handle_ref.i_strip_size()
	tsize_t TIFFStripSize(TIFF* tif)
	typedef int32 tsize_t;
 handle_ref.u_strip_count()
	tstrip_t TIFFNumberOfStrips(TIFF* tif)
	typedef uint32 tstrip_t;
*/
read_strip::read_strip(
	igs::tif::read_handle& handle_ref,
	unsigned char *dest_top,
	const size_t dest_size
):
	strip_size_(TIFFStripSize(handle_ref)),
	number_of_strip_(TIFFNumberOfStrips(handle_ref)),
	strip_counter_(0),
	dest_size_(dest_size),
	dest_current_(dest_top),
	read_encoded_strip_bytes_(0)
{}

unsigned int read_strip::count(void) const
{ return this->number_of_strip_; }

void read_strip::segment_exec(igs::tif::read_handle& handle_ref) {
	/* �ƽв�������å� */
	if (this->number_of_strip_ <= this->strip_counter_) {
		std::ostringstream os;
		os	<< "over call TIFFReadEncodedStrip(,"
			<< this->strip_counter_
			<< ",p_data,"
			<< this->strip_size_
			<< ")";
		throw std::domain_error(os.str());
	}

	/*
	�ե����뤫��ñ��(?)�ɤ߹���
	tsize_t TIFFReadEncodedStrip(
		TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size);
	typedef uint32 tstrip_t;
	typedef void* tdata_t;
	typedef int32 tsize_t;
	typedef unsigned int uint32;
	typedef int int32;
	--> buf���֤��줿�ǡ����μºݤΥХ��ȿ����֤�.
	*/
	const tsize_t t_ret = TIFFReadEncodedStrip(
		handle_ref,
		this->strip_counter_,
		this->dest_current_,
		this->strip_size_
	);
	if (t_ret < 0) {
		throw std::domain_error( handle_ref.get_error_msg() );
	}

	/* �ƽв��������ȥ��å� */
	++ this->strip_counter_;

	/* ���β������֤���ذ�ư */
	this->dest_current_ += t_ret;

	/* memory over�����å���
	!!!�Ǵ���strip��this->strip_size_���狼��ʤ��Τǡ�
	����äƥ����������å����Ǥ��ʤ�!!! */
	this->read_encoded_strip_bytes_ += t_ret;
	if (this->dest_size_ < this->read_encoded_strip_bytes_) {
		std::ostringstream os;
		os	<< "overd memory<"
			<< this->dest_size_
			<< "> TIFFReadEncodedStrip(,"
			<< this->strip_counter_
			<< ",,"
			<< this->strip_size_
			<< ")";
		throw std::domain_error(os.str());
	}
}
//------------------------------------------------------------
