#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h" /* swap_endian() */
#include "igs_sgi_header.h"
#include "igs_sgi_rle_write.h"

//-----------------------------------------------------------
using igs::sgi::rle_write;
rle_write::rle_write(
	const unsigned char *sour_top
	, const unsigned int he
	, const unsigned int wi
	, const unsigned int ch
	, const unsigned int sampling_bytes
	, const bool byte_swap_sw
	, unsigned long *sour_seek_set
	, unsigned long *sour_bytes_size
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
	, sour_seek_set_(sour_seek_set)
	, sour_bytes_size_(sour_bytes_size)
	, orient_is_topleft_sw_(orient_is_topleft_sw)
{
	if (this->orient_is_topleft_sw_) {
		this->sour_current_ +=
			(this->he_ - 1) * this->scanline_bytes_;
	}
}

unsigned int rle_write::count(void) const
{ return this->vertical_count_; }
//-----------------------------------------------------------
namespace {
 template<class T> unsigned int check_raw(
 const T* t_top,
 const unsigned int wi,
 const unsigned int ch
 ) {
	for (unsigned int xx = 0, x2 = 1; x2 < wi; ++xx, ++x2) {
		if (t_top[xx*ch] == t_top[x2*ch]) { return xx; }
	}
	return wi;
 }
 template<class T> unsigned int check_rle(
 const T* t_top,
 const unsigned int wi,
 const unsigned int ch
 ) {
	for (unsigned int xx = 0, x2 = 1; x2 < wi; ++xx, ++x2) {
		if (t_top[0] != t_top[x2*ch]) { return xx; }
	}
	return wi;
 }
 template<class T> void write_unit(
 T value,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	if (2 == sizeof(T)) {
 	 T val2 = value;
	 if (byte_swap_sw) {
	  igs::resource::swap_endian(val2);
	 }
	 ofs.write( igs::resource::pointer_cast<char *>(&val2), sizeof(T) );
	} else if (1 == sizeof(T)) {
	 ofs.write( igs::resource::pointer_cast<char *>(&value), sizeof(T) );
	} else {
	 throw std::domain_error("templete write_unit(-) bad type size.");
	}
	if (!ofs) {
		throw std::domain_error(
			"file write<std::ofstream> error");
	}
 }
 template<class T> unsigned int write_raw(
 const T* t_top,
 const unsigned int wi,
 const unsigned int ch,
 const unsigned int maxlen,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	unsigned int x2;
	unsigned int length = 0;
	unsigned int w_count = 0;
	for (unsigned int xx = 0; xx < wi; xx += length) {
		length = (maxlen < (wi-xx))? maxlen: (wi-xx);
		write_unit( static_cast<T>(0x80|length),
			byte_swap_sw, ofs ); ++w_count;
		for (x2 = 0; x2 < length; ++x2) {
		 write_unit( t_top[(xx+x2)*ch],
			byte_swap_sw, ofs ); ++w_count;
		}
	}
	return w_count;
 }
 template<class T> unsigned int write_rle(
 const T* t_top,
 const unsigned int wi,
 const unsigned int ch,
 const unsigned int maxlen,
 const bool byte_swap_sw,
 std::ofstream& ofs
 ) {
	unsigned int length = 0;
	unsigned int w_count = 0;
	for (unsigned int xx = 0; xx < wi; xx += length) {
		length = (maxlen < (wi-xx))? maxlen: (wi-xx);
		write_unit( static_cast<T>(length),
			byte_swap_sw, ofs ); ++w_count;
		write_unit( t_top[0],
			byte_swap_sw, ofs ); ++w_count;
	}
	return w_count;
 }
 /* Run Lengthの符号化(encode)した大きさを返す(bytes数でない) */
 template<class T> unsigned int rle_scanline(
	const T* t_top,
	const unsigned int wi,
	const unsigned int ch,
	const bool byte_swap_sw,
	std::ofstream& ofs
 ) {
	/* unsigned int maxlen =
(1<<(std::numeric_limits<unsigned char>::digits*sizeof(T) - 1)) - 1;
		T is unsigned char, then maxlen is 127
			(= std::numeric_limits<char>::max())
		T is unsigned short,then maxlen is 32767
			(= std::numeric_limits<short>::max())
		but not support about <unsigned short> size!
	*/
	const unsigned int maxlen = 0xff - 0x80;
	unsigned int w_count = 0;
	unsigned int length = 0;
	for (unsigned int xx = 0; xx < wi; xx += length) {
		/* 不連続値をセット(maxでmaxlen個づつ) */
		length = check_raw( &t_top[xx*ch],wi-xx,ch );
		if (0 < length) {
		 w_count += write_raw( &t_top[xx*ch],
			length, ch, maxlen, byte_swap_sw, ofs );
		} else {
		 /* 連続する同値を圧縮(maxでmaxlen個づつ) */
		 length = check_rle( &t_top[xx*ch],wi-xx,ch );
		 if (0 < length) {
		  w_count += write_rle( &t_top[xx*ch],
			length, ch, maxlen, byte_swap_sw, ofs );
		 } else {
			throw std::domain_error(
		  "sgi check_raw(-) and check_rle(-) returns zero"
			);
		 }
		}
	}
	/* 終端を示すカウント値 */
	write_unit( (T)0, byte_swap_sw, ofs ); ++w_count;

	return w_count;
 }
}
//-----------------------------------------------------------
void rle_write::segment_exec(std::ofstream& ofs) {
	/* 呼出回数チェック */
	if (this->vertical_count_ <= this->vertical_current_) {
		std::ostringstream os;
		os	<< "over call vertical_count<"
			<< this->vertical_count_ << ">";
		throw std::domain_error(os.str());
	}

	unsigned int u_size;
	if (1 == this->sampling_bytes_) {
		/* 符号化した数を返す、bytes数ではない */
		u_size = rle_scanline(
			this->sour_current_,
			this->wi_, this->ch_,
			this->byte_swap_sw_, ofs
		);
	} else { /* 2bytes */
		u_size = rle_scanline(
			igs::resource::const_pointer_cast<const unsigned short *>(
				this->sour_current_
			),
			this->wi_, this->ch_,
			this->byte_swap_sw_, ofs
		);
	}

	/* 保存したbyteサイズ */
	this->sour_bytes_size_[
	 this->vertical_current_
	] = static_cast<unsigned long>(u_size * this->sampling_bytes_);

	/* seek位置 */
	if (this->vertical_current_ <= 0) {
		this->sour_seek_set_[this->vertical_current_] =
			sizeof(igs::sgi::header::parameters) +
			this->vertical_count_*sizeof(unsigned long)*2;
	} else {
		this->sour_seek_set_[this->vertical_current_] =
		 this->sour_seek_set_[this->vertical_current_-1] +
		 this->sour_bytes_size_[this->vertical_current_-1];
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
