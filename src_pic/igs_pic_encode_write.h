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

	unsigned int count(void) const; /* segment_exec()�¹Բ�� */

	void segment_exec(
		std::vector<igs::pic::header::channel_packet>&
			packet_vector,	/* �������껲�� */
		bool byte_swap_sw,
		std::ofstream& ofs
	); /* ��ʬ����(�ƽ���缡����) */
   private:
	encode_write();

#if 0
	const unsigned char *sour_current_;	/* �������껲�� */
	const unsigned int he_;
	const unsigned int wi_;
	const unsigned int ch_;
#endif
	unsigned char *sour_current_;	/* �������껲�� */
	unsigned int he_;
	unsigned int wi_;
	unsigned int ch_;
	unsigned int y_;
	bool orient_is_botleft_sw_;
  };
 }
}

#endif	/* !igs_pic_encode_write_h */
