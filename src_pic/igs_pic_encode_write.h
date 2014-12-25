#ifndef igs_pic_encode_write_h
#define igs_pic_encode_write_h

#include <vector>
#include <fstream>
#include "igs_pic_header.h"

namespace igs {
 namespace pic {
  class encode_write {
  public:
	encode_write(
		unsigned char *sour_top,
		unsigned int u_he,
		unsigned int u_wi,
		unsigned int u_ch,
		bool orient_is_botleft_sw
	);

	unsigned int count(void) const; /* segment_exec()実行回数 */

	void segment_exec(
		std::vector<igs::pic::header::channel_packet>&
			packet_vector,	/* 外部メモリ参照 */
		bool byte_swap_sw,
		std::ofstream& ofs
	); /* 部分処理(呼出毎順次処理) */
   private:
	encode_write();

#if 0
	const unsigned char *sour_current_;	/* 外部メモリ参照 */
	const unsigned int he_;
	const unsigned int wi_;
	const unsigned int ch_;
#endif
	unsigned char *sour_current_;	/* 外部メモリ参照 */
	unsigned int he_;
	unsigned int wi_;
	unsigned int ch_;
	unsigned int y_;
	bool orient_is_botleft_sw_;
  };
 }
}

#endif	/* !igs_pic_encode_write_h */
