#include <algorithm> /* std::swap(-) */
#include <stdexcept> /* std::domain_error(-) */
#include <sstream> /* std::ostringstream */
#include "igs_pic_decode.h"

using igs::pic::decode;
//------------------------------------------------------------
decode::decode(
/******
const unsigned char *sour_top,
const unsigned int he,
const unsigned int wi,
const unsigned int ch,
******/
unsigned char *sour_top,
unsigned int he,
unsigned int wi,
unsigned int ch,
unsigned char *dest_top
):
	sour_current_(sour_top),
	he_(he),
	wi_(wi),
	ch_(ch),
	scanline_bytes_(wi * ch),
	dest_current_(dest_top),
	y_(0)
{}
unsigned int decode::count(void) const { return this->he_; }
//------------------------------------------------------------
namespace {
 unsigned int decode_raw(
 const unsigned char sour_flag,
 const unsigned int sour_ch,
 const unsigned char* sour_sl_top,
 const unsigned int dest_len,
 const unsigned int dest_ch,
 unsigned char* dest_sl_top
 ) {
	using namespace igs::pic::header::channel;
	unsigned int xx;
	unsigned int zz=0;
	unsigned int sour_incr=0;
	if (uc_red_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 0] =
			sour_sl_top[xx * sour_ch + zz];
		}
		sour_incr += dest_len;		 ++zz;
	}
	if (uc_gre_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 1] =
			sour_sl_top[xx * sour_ch + zz];
		}
		sour_incr += dest_len;		 ++zz;
	}
	if (uc_blu_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 2] =
			sour_sl_top[xx * sour_ch + zz];
		}
		sour_incr += dest_len;		 ++zz;
	}
	if (uc_alp_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 3] =
			sour_sl_top[xx * sour_ch + zz];
		}
		sour_incr += dest_len;		 ++zz;
	}
	return sour_incr;
 }
 unsigned int decode_run_length(
 const unsigned char sour_flag,
 const unsigned char* sour_sl_top,
 const unsigned int dest_len,
 const unsigned int dest_ch,
 unsigned char* dest_sl_top
 ) {
	using namespace igs::pic::header::channel;
	unsigned int xx;
	unsigned int zz=0;
	if (uc_red_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 0] =
			sour_sl_top[zz];
		}		  ++zz;
	}
	if (uc_gre_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 1] =
			sour_sl_top[zz];
		}		  ++zz;
	}
	if (uc_blu_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 2] =
			sour_sl_top[zz];
		}		  ++zz;
	}
	if (uc_alp_bit & sour_flag) {
		for (xx=0; xx < dest_len; ++xx) {
			dest_sl_top[xx * dest_ch + 3] =
			sour_sl_top[zz];
		}		  ++zz;
	}
	return zz;
 }
}
//------------------------------
namespace {
 /*
	rgbrgbrgb	-->	rgbrgbrgb
	aaa		-->	aaa
	rgbargbargba	-->	rgbargbargba
 */
 unsigned int decode_scanline_raw(
 const unsigned char sour_flag,
 const unsigned int sour_ch,
 const unsigned char* sour_sl_top,
 const unsigned int dest_wi,
 const unsigned int dest_ch,
 unsigned char* dest_sl_top
 ) {
	return decode_raw(
		sour_flag, sour_ch, sour_sl_top,
		dest_wi, dest_ch, dest_sl_top
	);
 }

 /*
	3rgb	-->	rgbrgbrgb
	3a	-->	aaa
	3rgba	-->	rgbargbargba
	1rgb	-->	rgb
	1a	-->	a
	1rgba	-->	rgba
 */
 unsigned int decode_scanline_pure(
 const unsigned char sour_flag,
 /*const unsigned int sour_ch,*/
 const unsigned char* sour_sl_top,
 const unsigned int dest_wi,
 const unsigned int dest_ch,
 unsigned char* dest_sl_top
 ) {
	unsigned int sour_incr=0;
	unsigned int dest_len=0;
	for (unsigned int xx = 0; xx < dest_wi; ++xx) {
		dest_len = static_cast<unsigned int>(
			sour_sl_top[sour_incr++]
		);
		if (dest_wi < (xx + dest_len)) {
		/* "dest_wi - xx"はマイナスにならない  */
			dest_len = dest_wi - xx;
		}
		if (0 < dest_len) {
			sour_incr += decode_run_length(
				sour_flag,
				&sour_sl_top[sour_incr],
				dest_len,
				dest_ch,
				&dest_sl_top[xx * dest_ch]
 			);
			xx += dest_len; /* 次の位置へ */
		}
	}
	return sour_incr;
 }

 /*
	uc(...127) -- rgb -->	dump (uc+1)pixel分のrgb
	uc(128)    us rgb -->	rle  rgbを(us)回くり返す
	uc(129...) -- rgb -->	rle  rgbを(uc-127)回くり返す
 */
 unsigned int decode_scanline_mixed(
 const unsigned char sour_flag,
 const unsigned int sour_ch,
 const unsigned char* sour_sl_top,
 const unsigned int dest_wi,
 const unsigned int dest_ch,
 unsigned char* dest_sl_top,
 const bool byte_swap_sw
 ) {
	unsigned int sour_incr=0;
	unsigned int dest_len=0;
	bool isrle;
	unsigned int xx;
	for (xx = 0; xx < dest_wi; xx += dest_len) {
		/* 1. run lengthかdumpかの判断と、その長さ */
		dest_len = static_cast<unsigned int>(
			sour_sl_top[sour_incr++]
		);
		if (dest_len < 128) { /* dump */
			isrle = false;
			++dest_len;
		} else {
			isrle = true;
			if (128 == dest_len) { /* big run length */
				union tag {
					unsigned char uca[2];
					unsigned short us;
				} ut;
				ut.uca[0] = sour_sl_top[sour_incr++];
				ut.uca[1] = sour_sl_top[sour_incr++];
				if (byte_swap_sw) {
					std::swap(ut.uca[0],ut.uca[1]);
				}
				dest_len = static_cast<unsigned int>(
					ut.us
				);
			} else { /* run length */
				dest_len -= 127;
			}
		}
		/* 2. 長さのlimit */
		if ((dest_wi - xx) < dest_len) {
		/*  "dest_wi - xx"の結果はマイナスにならない */
			std::ostringstream os;
			os << "hit garbage<" << dest_len << ">";
			throw std::domain_error(os.str());
		}
		if (0 == dest_len) {
			throw std::domain_error(
				"zero length mixed image part");
		}
		/* 3. 展開 */
		if (isrle) {
			sour_incr += decode_run_length(
			 sour_flag, &sour_sl_top[sour_incr],
			 dest_len, dest_ch, &dest_sl_top[xx * dest_ch]
 			);
		} else {
			sour_incr += decode_raw(
			 sour_flag, sour_ch, &sour_sl_top[sour_incr],
			 dest_len, dest_ch, &dest_sl_top[xx * dest_ch]
			);
						/* 次の位置へ */
		}
	}
	return sour_incr;
 }
}
//------------------------------
namespace {
 unsigned int decode_scanline_y(
	const unsigned int packet_size,
	const igs::pic::header::channel_packet *packet_array,
	const bool byte_swap_sw,
	const unsigned char* sour_sl_top,
	const unsigned int dest_wi,
	const unsigned int dest_ch,
	unsigned char* dest_sl_top
 ) {
	using namespace igs::pic::header::channel;
	using namespace igs::pic::header::compress_type;
	unsigned int sour_incr = 0;
	unsigned int sour_ch;
	/* sourから、packet毎に各channel画像をdecodeし、destへ */
	for (unsigned int ii=0; ii<packet_size; ++ii,++packet_array) {
		sour_ch = 0;
		if (uc_red_bit&packet_array->uc_channels) {++sour_ch;}
		if (uc_gre_bit&packet_array->uc_channels) {++sour_ch;}
		if (uc_blu_bit&packet_array->uc_channels) {++sour_ch;}
		if (uc_alp_bit&packet_array->uc_channels) {++sour_ch;}

		/* 圧縮タイプごとに読み込む */
		switch (packet_array->uc_type) {
		case uc_uncompres:
			sour_incr += decode_scanline_raw(
				packet_array->uc_channels,
				sour_ch, &sour_sl_top[sour_incr],
				dest_wi, dest_ch, dest_sl_top);
		case uc_pure_rle:
			sour_incr += decode_scanline_pure(
				packet_array->uc_channels,
				/*sour_ch,*/ &sour_sl_top[sour_incr],
				dest_wi, dest_ch, dest_sl_top);
		case uc_mixed_rle:
			sour_incr += decode_scanline_mixed(
				packet_array->uc_channels,
				sour_ch, &sour_sl_top[sour_incr],
				dest_wi, dest_ch, dest_sl_top,
				byte_swap_sw);
		}
	}
	return sour_incr;
 }
}
//------------------------------------------------------------
void decode::segment_exec(
	const unsigned int packet_size,
	const igs::pic::header::channel_packet *packet_array,
	const bool byte_swap_sw
) {
	/* 呼出回数チェック */
	if (this->he_ <= this->y_) {
		std::ostringstream os;
		os << "over call scanline<" << this->y_ << ">";
		throw std::domain_error(os.str());
	}
	/* sourから、scanline毎に画像をdecodeし、destへ */
	unsigned int sour_incr;
	sour_incr = decode_scanline_y(
		packet_size,
		packet_array,
		byte_swap_sw,
		this->sour_current_,
		this->wi_,
		this->ch_,
		this->dest_current_
	);

	/* 呼出回数カウントアップ */
	++ this->y_;

	/* 圧縮画像上のscanline位置を次へ移動しとく */
	this->sour_current_ += sour_incr;

	/* decode画像上のscanline位置を次へ移動しとく */
	//this->dest_current_ += this->wi_ * this->ch_;
	this->dest_current_ += this->scanline_bytes_;
}
