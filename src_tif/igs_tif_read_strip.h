#ifndef igs_tif_read_strip_h
#define igs_tif_read_strip_h

#include "igs_tif_read_handle.h"

namespace igs {
 namespace tif {
  class read_strip {
  public:
	read_strip(
		igs::tif::read_handle& handle_ref,
		unsigned char *dest_top,
		const size_t dest_size
	);

	unsigned int count(void) const; /* segment_exec()�¹Բ�� */

	void segment_exec(
		igs::tif::read_handle& handle_ref
	); /* �ƽ���缡(��ʬ)���� */
  private:
	read_strip();

	const tsize_t  strip_size_;
	const tstrip_t number_of_strip_;
	unsigned int   strip_counter_;

	const size_t dest_size_;
	unsigned char *dest_current_; /* �������껲�� */

	size_t read_encoded_strip_bytes_;
  };
 }
}

#endif /* !igs_tif_read_strip_h */
