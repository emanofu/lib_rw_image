#include <iostream> /* std::cout */
#include <sstream> /* std::ostringstream(-) */
#include <stdexcept> /* std::domain_error(-) */
#include <algorithm> /* swap(-) */
#include "igs_resource_irw.h"
#include "igs_pic_encode_write.h"

using igs::pic::encode_write;
//------------------------------------------------------------
encode_write::encode_write(
	unsigned char *sour_top
	,unsigned int he
	,unsigned int wi
	,unsigned int ch
	,bool orient_is_botleft_sw
):
	sour_current_(sour_top),
	he_(he),
	wi_(wi),
	ch_(ch),
	y_(0),
	orient_is_botleft_sw_(orient_is_botleft_sw)
{
	if (this->orient_is_botleft_sw_) {
		this->sour_current_ +=
			(this->he_ - 1) * this->wi_ * this->ch_;
	}
}
unsigned int encode_write::count(void) const { return this->he_; } 
//------------------------------------------------------------
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
 void encode_write_raw(
	const unsigned char* sour_top,
	const unsigned int len,
	const unsigned int ch,
	const unsigned char flag,
	std::ofstream& ofs
 ) {
	/* raw数はここでは書き出さない */
	using namespace igs::pic::header::channel;
	for (unsigned int xx = 0; xx < len; ++xx) {
	 if (uc_red_bit & flag) {ofs_write(&sour_top[xx*ch+0],1,ofs);}
	 if (uc_gre_bit & flag) {ofs_write(&sour_top[xx*ch+1],1,ofs);}
	 if (uc_blu_bit & flag) {ofs_write(&sour_top[xx*ch+2],1,ofs);}
	 if (uc_alp_bit & flag) {ofs_write(&sour_top[xx*ch+3],1,ofs);}
	}
 }
 unsigned int encode_check_raw(
 const unsigned char* sour_top,
 const unsigned int len,
 const unsigned int ch,
 const unsigned char flag
 ) {
	if (len <= 1) { return len;}  /* 0 or 1 */
	bool b_raw_sw;
	using namespace igs::pic::header::channel;
	for (unsigned int xx = 0, x2 = 1; x2 < len;
	++xx, ++x2) {
	 b_raw_sw = false;
	 if      ((uc_red_bit & flag) &&
	 (sour_top[x2*ch+0] != sour_top[xx*ch+0])) {b_raw_sw=true;}
	 else if ((uc_gre_bit & flag) &&
	 (sour_top[x2*ch+1] != sour_top[xx*ch+1])) {b_raw_sw=true;}
	 else if ((uc_blu_bit & flag) &&
	 (sour_top[x2*ch+2] != sour_top[xx*ch+2])) {b_raw_sw=true;}
	 else if ((uc_alp_bit & flag) &&
	 (sour_top[x2*ch+3] != sour_top[xx*ch+3])) {b_raw_sw=true;}
	 if (false == b_raw_sw) { return xx; } /* 0... */
	}
	return len;
 }
 void encode_write_run_length(
 const unsigned char* sour_top,
 const unsigned char flag,
 std::ofstream& ofs
 ) {
	/* run length数はここでは書き出さない */
	using namespace igs::pic::header::channel;
	if (uc_red_bit & flag) {ofs_write(&sour_top[0],1,ofs);}
	if (uc_gre_bit & flag) {ofs_write(&sour_top[1],1,ofs);}
	if (uc_blu_bit & flag) {ofs_write(&sour_top[2],1,ofs);}
	if (uc_alp_bit & flag) {ofs_write(&sour_top[3],1,ofs);}
 }
 unsigned int encode_check_run_length(
 const unsigned char* sour_top,
 const unsigned int len,
 const unsigned int ch,
 const unsigned char flag
 ) {
	if (len <= 1) { return len;}  /* 0 or 1 */
	using namespace igs::pic::header::channel;
	unsigned int xx;
	for (xx = 1; xx < len; ++xx) {
 if ((uc_red_bit&flag)&&(sour_top[0]!=sour_top[xx*ch+0])){break;}
 if ((uc_gre_bit&flag)&&(sour_top[1]!=sour_top[xx*ch+1])){break;}
 if ((uc_blu_bit&flag)&&(sour_top[2]!=sour_top[xx*ch+2])){break;}
 if ((uc_alp_bit&flag)&&(sour_top[3]!=sour_top[xx*ch+3])){break;}
	}
	return xx;
 }
 void encode_write_big_run_length(
 const unsigned char* sour_top,
 const unsigned int len,
 const unsigned char flag,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	unsigned char uca[1];
	union _tag { unsigned char uca[2]; unsigned short us; } ut;

	uca[0] = 128;
	ofs_write(uca,1,ofs);

	ut.us = static_cast<const unsigned short>(len);
	if (byte_swap_sw) { std::swap(ut.uca[0],ut.uca[1]); }
	ofs_write(ut.uca,2,ofs);

	using namespace igs::pic::header::channel;
	if (uc_red_bit & flag) {ofs_write(&sour_top[0],1,ofs);}
	if (uc_gre_bit & flag) {ofs_write(&sour_top[1],1,ofs);}
	if (uc_blu_bit & flag) {ofs_write(&sour_top[2],1,ofs);}
	if (uc_alp_bit & flag) {ofs_write(&sour_top[3],1,ofs);}
 }
}
//------------------------------
namespace {
 /*
	rgbrgbrgb	-->	3rgb
	aaa		-->	3a
	rgbargbargba	-->	3rgba
 */
 void encode_write_scanline_pure(
 const unsigned char* sour_sl_top,
 const unsigned int wi,
 const unsigned int ch,
 const unsigned char flag,
 std::ofstream& ofs
 ) {
	unsigned int x2, len1=0, len2=0;
	unsigned char uca[1];

	for (unsigned int xx = 0; xx < wi; xx += len1) {
		len1 = encode_check_run_length(
			&sour_sl_top[xx*ch], wi-xx, ch, flag);
		if (0 == len1) { break; } /* ありえないが念のため */
		for (x2 = 0; x2 < len1; x2 += len2) {
			len2 = (0xffU < (len1-x2))? 0xffU:(len1-x2);
			uca[0] = static_cast<unsigned char>(len2);
			ofs_write(uca,1,ofs);
			encode_write_run_length(
				&sour_sl_top[xx*ch], flag, ofs);
		}
	}
 }
 /*
	dump (uc+1)pixel分のrgb      --> uc(...127) -- rgb
	rle  rgbを(us)回くり返す     --> uc(128)    us rgb
	rle  rgbを(uc-127)回くり返す --> uc(129...) -- rgb
 */
 void encode_write_scanline_mixed(
 const unsigned char* sour_sl_top,
 const unsigned int wi,
 const unsigned int ch,
 const unsigned char flag,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	unsigned int x2, len1=0, len2;
	unsigned char uca[1];

	for (unsigned int xx = 0; xx < wi; xx += len1) {
	 len1 = encode_check_raw(
		&sour_sl_top[xx*ch], wi-xx, ch, flag);
	 if (1 <= len1) {
		for (x2=0,len2=0; x2 < len1; x2 += len2) {
			len2 = (128 < (len1-x2))? 128:(len1-x2);
			uca[0] = static_cast<unsigned char>(len2 - 1);
			ofs_write(uca,1,ofs);
			encode_write_raw(
				&sour_sl_top[(xx+x2)*ch],
				len2, ch, flag, ofs);
		}
	 } else {
		len1 = encode_check_run_length(
			&sour_sl_top[xx*ch], wi-xx, ch, flag);
		if (len1 <= 1) {
			std::ostringstream os;
			os << "encode_check_run_length() returns "
			   << len1;
			throw std::domain_error(os.str());
		}
		for (x2=0,len2=0; x2 < len1; x2 += len2) {
		 if ((len1-x2) <= 128) {
			len2 = len1-x2;
			uca[0] = static_cast<unsigned char>(len2+127);
			ofs_write(uca,1,ofs);
			encode_write_run_length(
				&sour_sl_top[xx*ch], flag, ofs);
		 } else {
			len2=(0xffffU<(len1-x2))? 0xffffU:(len1-x2);
			encode_write_big_run_length(
				&sour_sl_top[xx*ch], len2, flag,
				byte_swap_sw, ofs);
		 }
		}
	 }
	}
 }
}
//------------------------------------------------------------
void encode_write::segment_exec(
std::vector<igs::pic::header::channel_packet>& packet_vector,
bool byte_swap_sw,
std::ofstream& ofs
)
{
	/* 呼出回数チェック */
	if (this->he_ <= this->y_) {
		std::ostringstream os;
		os << "over call scanline<" << this->y_ << ">";
		throw std::domain_error(os.str());
	}
	/*
	チャンネルパケットリストをサーチして
	データ読み込みながら
	channelscanline毎run length復号
	*/
	using namespace igs::pic::header::compress_type;
	for (std::vector<igs::pic::header::channel_packet>::iterator
	it =  packet_vector.begin();
	it != packet_vector.end(); ++it) {
		/* 圧縮タイプごとに読み込む */
		switch (it->uc_type) {
		case uc_uncompres:
			encode_write_raw(
				this->sour_current_,
				this->wi_, this->ch_, it->uc_channels,
				ofs);
			break;
		case uc_pure_rle:
			encode_write_scanline_pure(
				this->sour_current_,
				this->wi_, this->ch_, it->uc_channels,
				ofs);
			break;
		case uc_mixed_rle:
			encode_write_scanline_mixed(
				this->sour_current_,
				this->wi_, this->ch_, it->uc_channels,
				byte_swap_sw,
				ofs
			);
			break;
		}
	}

	/* 呼出回数カウントアップ */
	++ this->y_;

	/* 保存画像を次へ移動しとく */
	if (this->orient_is_botleft_sw_) {
		this->sour_current_ -= this->wi_ * this->ch_;
	} else {
		this->sour_current_ += this->wi_ * this->ch_;
	}
}
