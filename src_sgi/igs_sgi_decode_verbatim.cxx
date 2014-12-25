#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
// #include "igs_image_irw.h"
#include "igs_sgi_decode_verbatim.h"

//-----------------------------------------------------------
using igs::sgi::decode_verbatim;
decode_verbatim::decode_verbatim(
	const unsigned char *sour_top
	, unsigned char	*dest_top
	, const unsigned int	he
	, const unsigned int	wi
	, const unsigned int	ch
	, const unsigned int	sampling_bytes
):
	sour_top_(sour_top)
	, dest_top_(dest_top)
	, he_(he)
	, wi_(wi)
	, ch_(ch)
	, sampling_bytes_(sampling_bytes)
	, scanline_bytes_(wi * ch * sampling_bytes)
{}
//-----------------------------------------------------------
namespace {
 template<class T> void translate_scanline_verbatim(
	int sour_size
	, const T* sour_image
	, T* dest_image
	, const int ch
 ) {
	while (0 < sour_size--) {
		(*dest_image) = (*sour_image);
		++sour_image; dest_image += ch;
	}
 }
}

void decode_verbatim::segment_exec(
	const int yy
	, const int zz
	, const bool rvs_sw
) {
	const int yd = rvs_sw? (this->he_ - 1 - yy): yy;

	/* 呼び出し範囲チェック */
	if (yy < 0) {
		throw std::domain_error("yy is Minus");
	}
	if (this->he_ <= yy) {
		std::ostringstream os("yy equal over ");os << this->he_;
		throw std::domain_error(os.str());
	}
	if (zz < 0) {
		throw std::domain_error("zz is Minus");
	}
	if (this->ch_ <= zz) {
		std::ostringstream os("zz equal over ");os << this->ch_;
		throw std::domain_error(os.str());
	}

	if (1 == this->sampling_bytes_) {
	 translate_scanline_verbatim(
		this->wi_
		, &this->sour_top_[
			zz* this->he_* this->wi_* this->sampling_bytes_
			+ yy * this->wi_* this->sampling_bytes_
		]
		, &this->dest_top_[
			  yd * this->scanline_bytes_
			+ zz * this->sampling_bytes_
		]
		, this->ch_
	 );
	} else { /* 2bytes */
	 translate_scanline_verbatim(
		this->wi_
		, igs::resource::const_pointer_cast<
					const unsigned short *>(
		&this->sour_top_[
			zz* this->he_* this->wi_* this->sampling_bytes_
			+ yy * this->wi_* this->sampling_bytes_
		])
		, igs::resource::pointer_cast<unsigned short *>(
		&this->dest_top_[
			  yd * this->scanline_bytes_
			+ zz * this->sampling_bytes_
		])
		, this->ch_
	 );
	}
}
/*--------------------------------------------------------*/
