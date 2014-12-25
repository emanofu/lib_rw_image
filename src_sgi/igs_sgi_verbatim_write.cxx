#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_sgi_verbatim_write.h"

//-----------------------------------------------------------
using igs::sgi::verbatim_write;
verbatim_write::verbatim_write(
	const unsigned char *sour_top
	, const unsigned int he
	, const unsigned int wi
	, const unsigned int ch
	, const unsigned int sampling_bytes
	, const bool byte_swap_sw
	, const bool orient_is_topleft_sw
):
	sour_channel_top_(sour_top)
	, sour_current_(sour_top)
	, he_(he)
	, wi_(wi)
	, ch_(ch)
	, sampling_bytes_(sampling_bytes)
	, scanline_bytes_(wi * ch * sampling_bytes)
	, vertical_count_(he * ch)
	, vertical_current_(0)
	, y_(0)
	, byte_swap_sw_(byte_swap_sw)
	, orient_is_topleft_sw_(orient_is_topleft_sw)
{
	if (this->orient_is_topleft_sw_) {
		this->sour_current_ +=
			(this->he_ - 1) * this->scanline_bytes_;
	}
}

unsigned int verbatim_write::count(void) const
{ return this->vertical_count_; }
//-----------------------------------------------------------
namespace {
 void ofs_write(
 const unsigned char* uchar_array,
 const unsigned int byte_size,
 std::ofstream& ofs
 ) {
	ofs.write(
		igs::resource::pointer_cast<char *>(
			const_cast<unsigned char *>(uchar_array)
		),
		byte_size
	);
	if (!ofs) {
		throw std::domain_error(
			"file write<std::ofstream> error");
	}
 }
 void write_uchar(
 const unsigned char* uchar_ptr,
 const unsigned int wi,
 const unsigned int ch,
 std::ofstream& ofs
 ) {
	for (unsigned int xx = 0; xx < wi; ++xx, uchar_ptr += ch) {
		ofs_write(uchar_ptr, sizeof(unsigned char), ofs);
	}
 }
 void write_ushort(
 const unsigned short* ushort_ptr,
 const unsigned int wi,
 const unsigned int ch,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	if (byte_swap_sw) {
		union { unsigned char uca[2]; unsigned short us; } pix;
		for (unsigned int xx=0; xx<wi; ++xx, ushort_ptr+=ch) {
			pix.us = *ushort_ptr;
			std::swap( pix.uca[0], pix.uca[1] );
			ofs_write(pix.uca,sizeof(unsigned short),ofs);
		}
	} else {
		for (unsigned int xx=0; xx<wi; ++xx, ushort_ptr+=ch) {
			ofs_write(
		igs::resource::const_pointer_cast<const unsigned char *>(ushort_ptr),
				sizeof(unsigned short), ofs);
		}
	}
 }
}

/* 符号化した数を返す、bytes数ではない */
void verbatim_write::segment_exec(std::ofstream& ofs) {
	/* 呼出回数チェック */
	if (this->vertical_count_ <= this->vertical_current_) {
		std::ostringstream os;
		os	<< "over call vertical_count<"
			<< this->vertical_count_ << ">";
		throw std::domain_error(os.str());
	}

	if (1 == this->sampling_bytes_) {
		write_uchar(
			this->sour_current_, this->wi_, this->ch_, ofs
		);
	} else { /* 2bytes */
		write_ushort(
			igs::resource::const_pointer_cast<const unsigned short *>(
				this->sour_current_
			),
			this->wi_, this->ch_, this->byte_swap_sw_, ofs
		);
	}

	/* 呼出回数カウントアップ */
	/* 画像を次へ1スキャンライン(pixel)単位進めとく */
	++ this->vertical_current_;
	++ this->y_;
	if (this->y_ < this->he_) {
		if (this->orient_is_topleft_sw_) {
			this->sour_current_ -= this->scanline_bytes_;
		} else {
			this->sour_current_ += this->scanline_bytes_;
		}
	} else {
		this->y_ = 0;
		this->sour_channel_top_ += this->sampling_bytes_;
		this->sour_current_ = this->sour_channel_top_;
		if (this->orient_is_topleft_sw_) {
			this->sour_current_ +=
				(this->he_ - 1) * this->scanline_bytes_;
		}
	}
}
//-----------------------------------------------------------
