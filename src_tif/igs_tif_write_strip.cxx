#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_tif_error.h"
#include "igs_tif_write_strip.h"

/* strip�񤭽Ф� */
//------------------------------------------------------------
using igs::tif::write_strip;

write_strip::write_strip(
	igs::tif::write_handle& handle_ref,
	const unsigned char *sour_top,
	const size_t sour_size
):
	number_of_strip_(1), /* ������1�ΤޤޤǤ����Τ�����??? */
	strip_counter_(0),
	sour_size_(sour_size),
	sour_current_(sour_top),
	write_encoded_strip_bytes_(0)
{}

unsigned int write_strip::count(void) const
{ return this->number_of_strip_; }

void write_strip::segment_exec(igs::tif::write_handle& handle_ref)
{
	/* �ƽв�������å� */
	if (this->number_of_strip_ <= this->strip_counter_) {
		std::ostringstream os;
		os	<< "over call TIFFWriteEncodedStrip(,"
			<< this->strip_counter_
			<< ",p_data,"
			<< this->sour_size_
			<< ")";
		throw std::domain_error(os.str());
	}
	/* memory over�����å� */
	if (this->sour_size_ <= this->write_encoded_strip_bytes_) {
		std::ostringstream os;
		os	<< "over memory<"
			<< "> TIFFWriteEncodedStrip(,"
			<< this->strip_counter_
			<< ",,"
			<< this->sour_size_ 
			<< ")";
		throw std::domain_error(os.str());
	}
/*
	tsize_t TIFFWriteEncodedStrip(
		TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size);
	typedef void* tdata_t;

	size is
		h*(w/std::numeric_limits<unsigned char>::digits+(w%std::numeric_limits<unsigned char>::digits?1:0))	--> bw		
		h*w*b*ch			--> rgb/rgba	

	TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, ...);
		�򤷤Ƥ������Ȥ�ɬ��???

	size is returned, when error then -1 is returned.
*/
	tsize_t t_ret = TIFFWriteEncodedStrip(
		handle_ref,
		this->strip_counter_,
		const_cast<unsigned char *>(this->sour_current_ ),
		static_cast<tsize_t>(this->sour_size_)
	);
	if (t_ret < 0) {
		throw std::domain_error(handle_ref.get_error_msg());
	}

	/* �ƽв��������ȥ��å� */
	++ this->strip_counter_;

	/* ���β������֤���ذ�ư */
	this->sour_current_ += t_ret;
	this->write_encoded_strip_bytes_ += t_ret;
}
//------------------------------------------------------------
