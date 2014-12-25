#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_tif_error.h"
#include "igs_tif_write_strip.h"

/* strip書き出し */
//------------------------------------------------------------
using igs::tif::write_strip;

write_strip::write_strip(
	igs::tif::write_handle& handle_ref,
	const unsigned char *sour_top,
	const size_t sour_size
):
	number_of_strip_(1), /* ここは1のままでいいのか不明??? */
	strip_counter_(0),
	sour_size_(sour_size),
	sour_current_(sour_top),
	write_encoded_strip_bytes_(0)
{}

unsigned int write_strip::count(void) const
{ return this->number_of_strip_; }

void write_strip::segment_exec(igs::tif::write_handle& handle_ref)
{
	/* 呼出回数チェック */
	if (this->number_of_strip_ <= this->strip_counter_) {
		std::ostringstream os;
		os	<< "over call TIFFWriteEncodedStrip(,"
			<< this->strip_counter_
			<< ",p_data,"
			<< this->sour_size_
			<< ")";
		throw std::domain_error(os.str());
	}
	/* memory overチェック */
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
		をしておくことが必要???

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

	/* 呼出回数カウントアップ */
	++ this->strip_counter_;

	/* 次の画像の置き場へ移動 */
	this->sour_current_ += t_ret;
	this->write_encoded_strip_bytes_ += t_ret;
}
//------------------------------------------------------------
