#ifndef igs_tga_decode_h
#define igs_tga_decode_h

#include "igs_resource_irw.h"
#include "igs_tga_pixel.h"

namespace igs {
 namespace tga {
  namespace decode {
   /* --- Pixel単位の処理 ------------------------------- */
   class pixel {
   public:
	pixel( const unsigned char* cmap
		, const int cmap_index_of_entry
		, const int cmap_count_of_entries
		, const int cmap_bits_depth
		, const bool byte_swap_sw
	);
	void exec(    const igs::tga::pixel::bgra32&  in
			, igs::image::pixel::rgba32&out);
	void exec(    const igs::tga::pixel::bgr24&   in
			, igs::image::pixel::rgb24& out);
	void exec(    const igs::tga::pixel::argb1555&in	
			, igs::image::pixel::rgba32&out);
	void exec(    const igs::tga::pixel::argb0555&in
			, igs::image::pixel::rgb24& out);
	void exec(    const igs::tga::pixel::cmap8&   in
			, igs::image::pixel::rgba32&out);
	void exec(    const igs::tga::pixel::cmap8&   in
			, igs::image::pixel::rgb24& out);
	void exec(    const igs::tga::pixel::bw8&     in
			, igs::image::pixel::gray8& out);
   private:
	//pixel(){}
	const unsigned char* cmap_;
	const int cmap_index_of_entry_;
	const int cmap_count_of_entries_;
	const int cmap_bits_depth_;
	const bool byte_swap_sw_;
   };
   /* --- メモリアクセスチェックにもなるloop endチェック  */
   class check_end {
   public:
	explicit check_end(const size_t size):size_(size),count_(0){}
	const bool incr(size_t add=1) { /* 容量越えたらtrueを返す */
		this->count_ += add;
		if (this->size_ < this->count_) { return true; }
		return false;
	}
   private:
	check_end();
	size_t size_, count_;
   };
   /* --- 1byte分読み取り、アドレスを進める ------------- */
   template<class SOU> void incr_byte(
	const SOU*& sou_image
	,int& start
   ) {
	const unsigned char* tmpp = igs::resource::const_pointer_cast<
		const unsigned char *
	>(sou_image);
	start = static_cast<int>(*tmpp++);
	sou_image=igs::resource::const_pointer_cast<const SOU *>(tmpp);
   }
   /* --- Run Lengthの復号(decode) ---------------------- */
   template<class SOU, class DES> void run_length(
	  const SOU* sou_image, const size_t sou_bytes
	,       DES* des_image, const size_t des_bytes
	, igs::tga::decode::pixel& decoder
	, bool& break_sw
   ) {
 	igs::tga::decode::check_end
			  sour_chk(sou_bytes)
			, dest_chk(des_bytes);
	int start = 0;
	int count = 0;
	while (1) {
		if (true == break_sw) { break_sw = false; return; }
		if (sour_chk.incr()) {return;}
		igs::tga::decode::incr_byte(sou_image, start);
		count = (start&0x7f)+1;/* 個数(0は1,1は2,2は3...) */
		if (start & 0x80) {	/* RLE圧縮部分 */
			if (sour_chk.incr(sizeof(SOU))) {return;}
			if (dest_chk.incr(sizeof(DES)*count)) {return;}
			while (0 < count--) {
			  decoder.exec(*sou_image,*des_image++);
			}
				++sou_image;
		} else {		/* 非圧縮部分 */
			if (sour_chk.incr(sizeof(SOU)*count)) {return;}
			if (dest_chk.incr(sizeof(DES)*count)) {return;}
			while (0 < count--) {
			  decoder.exec(*sou_image++,*des_image++);
			}
		}
	}
   }

   /* --- Uncompressedの移し変え------------------------- */
   template<class SOU, class DES> void uncompressed(
	  const SOU* sou_image, const size_t sou_bytes
	,       DES* des_image,       size_t des_bytes
	, igs::tga::decode::pixel& decoder
	, bool& break_sw
   ) {
	size_t des_pixel = des_bytes/sizeof(DES);
	size_t sou_pixel = sou_bytes/sizeof(SOU);
	/* 受け側(DES)でPixel数を見るため、送り側(SOU)が少ないとだめ */
	if (sou_pixel < des_pixel) {
		return;
	}
	for (size_t ii = 0; ii < des_pixel; ++ii) {
		if (true == break_sw) { break_sw = false; return; }
		decoder.exec(*sou_image++ ,*des_image++);
	}
   }
  }
 }
}
#endif	/* !igs_tga_decode_h */
