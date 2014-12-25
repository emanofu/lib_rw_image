#ifndef igs_sgi_verbatim_write_h
#define igs_sgi_verbatim_write_h

#include <fstream>

namespace igs {
 namespace sgi { /* SiliconGraphic�����ե����� */
  class verbatim_write {
  public:
	verbatim_write(
		const unsigned char *sour_top
		, const unsigned int he
		, const unsigned int wi
		, const unsigned int ch
		, const unsigned int sampling_bytes
		, const bool byte_swap_sw
		, const bool orient_is_topleft_sw
	);

	unsigned int count(void) const; /* v_segment()�¹Բ�� */

	void segment_exec(std::ofstream& ofs);/* �ƽ���缡���� */
  private:
	verbatim_write();

	const unsigned char *sour_channel_top_; /* �������껲�� */
	const unsigned char *sour_current_;     /* �������껲�� */
	const unsigned int	he_, wi_, ch_, sampling_bytes_;
	const unsigned int	scanline_bytes_, vertical_count_;
	unsigned int	vertical_current_;
	unsigned int	y_;
	const bool byte_swap_sw_;
	const bool orient_is_topleft_sw_;
  };
 }
}

#endif	/* !igs_sgi_verbatim_write_h */
