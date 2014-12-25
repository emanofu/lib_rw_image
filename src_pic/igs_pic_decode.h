#ifndef igs_pic_decode_h
#define igs_pic_decode_h

#include <vector>
#include "igs_pic_header.h"

namespace igs {
 namespace pic {
  class decode {
  public:
	decode(
/************
		const unsigned char *sour_top,
		const unsigned int he,
		const unsigned int wi,
		const unsigned int ch,
***********/
		unsigned char *sour_top,
		unsigned int he,
		unsigned int wi,
		unsigned int ch,
		unsigned char *dest_top
	);

	unsigned int count(void) const; /* segment_exec()実行回数 */

	void segment_exec(
	 const unsigned int packet_size,
	 const igs::pic::header::channel_packet *packet_array,
	 const bool byte_swap_sw
	); /* 部分処理(呼出毎順次処理) */
   private:
	decode();

#if 0
	const unsigned char *sour_current_;	/* 外部メモリ参照 */
	const unsigned int he_;
	const unsigned int wi_;
	const unsigned int ch_;
	const unsigned int scanline_bytes_;
#endif
	unsigned char *sour_current_;	/* 外部メモリ参照 */
	unsigned int he_;
	unsigned int wi_;
	unsigned int ch_;
	unsigned int scanline_bytes_;

	unsigned char *dest_current_;	/* 外部メモリ参照 */
	unsigned int y_;
  };
 }
}

#endif	/* !igs_pic_decode_h */
