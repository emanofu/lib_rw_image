#include <iostream> /* std::cout */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
// #include "igs_image_irw.h"
#include "igs_sgi_decode_rle.h"

//-----------------------------------------------------------
using igs::sgi::decode_rle;
decode_rle::decode_rle(
	  const unsigned char	*sour_top
	, const unsigned long	*sour_seek_set
	, const unsigned long	*sour_bytes_size
	, unsigned char	*dest_top
	, const int	dest_bytes
	, const int	he
	, const int	wi
	, const int	ch
	, const int	sampling_bytes
):
	  sour_top_(sour_top)
	, sour_seek_set_(sour_seek_set)
	, sour_bytes_size_(sour_bytes_size)
	, dest_top_(dest_top)
	, dest_bytes_(dest_bytes)
	, he_(he)
	, ch_(ch)
	, sampling_bytes_(sampling_bytes)
	, scanline_bytes_(wi * ch * sampling_bytes)
{}
//-----------------------------------------------------------
#include <iostream>
namespace {
 /* std::vectorでやっているようなメモリアクセスチェック */
 /* 代入しようとしている部分が容量を越えたらエラーをthrow */
 /* ただし処理重くなる */
 class throw_overflow {
 public:
	explicit throw_overflow(const char*tt, const int size)
	:title_(tt),size_(size),count_(0){}
	void increment(int add=1) {
		if (this->size_ < this->count_) {
	//std::ostringstream os("over run<");/* これだと表示しない */
			std::ostringstream os; os << "over run<";
			os << this->size_ << ">,by sgi decode rle,at "
			<< this->title_;
			throw std::domain_error(os.str());
		}
		this->count_ += add;
	}
 private:
	const char* title_;
	const int size_;
	int count_;
 };
 /* Run Lengthの復号(decode) */
 template<class T> void decode_scanline_run_length(
	  const int sour_size, const T* sour_image
	, const int dest_size,       T* dest_image
	, const int ch
 ) {
	int start,count;
	T pixel_image;
//	throw_overflow	  chk_sour("sour",sour_size)
//			, chk_dest("dest",dest_size);
	while (1) {
//				chk_sour.increment();
		start = static_cast<int>(*sour_image++);/* 値とり次へ */
		count = (start & 0x7f);		/* 個数 */
		if (0 == count) { return; }	/* 個数ゼロは終 */
		if (start & 0x80) {		/* 非圧縮部分 */
			while (0 < count--) {
//				chk_sour.increment();
//				chk_dest.increment(ch);
				*dest_image = *sour_image++;
				dest_image += ch;
			}
		} else {			/* 圧縮部分 */
//				chk_sour.increment();
			pixel_image = (*sour_image++);
			while (0 < count--) {
//				chk_dest.increment(ch);
				*dest_image = pixel_image;
				dest_image += ch;
			}
		}
	}
 }
}

void decode_rle::segment_exec(const int yy, const int zz, const bool rvs_sw)
{
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

	/* 復号 */
	if (1 == this->sampling_bytes_) {
	 decode_scanline_run_length(
		static_cast<int>(
			this->sour_bytes_size_[yy + this->he_ * zz]
		)
		, &(this->sour_top_[
			this->sour_seek_set_[yy + this->he_ * zz]
		])
		, this->dest_bytes_ -
			  yd * this->scanline_bytes_
			- zz * this->sampling_bytes_
		, &this->dest_top_[
			  yd * this->scanline_bytes_
			+ zz * this->sampling_bytes_
		]
		, this->ch_
	 );
	} else { /* 2bytes */
	 decode_scanline_run_length(
		static_cast<int>(
			this->sour_bytes_size_[yy + this->he_ * zz] /
			this->sampling_bytes_
		)
		, igs::resource::const_pointer_cast<
			const unsigned short *
		>(&(this->sour_top_[
			this->sour_seek_set_[yy + this->he_ * zz]
		]))
		, this->dest_bytes_ -
			  yd * this->scanline_bytes_
			- zz * this->sampling_bytes_
		, igs::resource::pointer_cast<unsigned short *>(
		&this->dest_top_[
			  yd * this->scanline_bytes_
			+ zz * this->sampling_bytes_
		])
		, this->ch_
	 );
	}
}
//-----------------------------------------------------------
