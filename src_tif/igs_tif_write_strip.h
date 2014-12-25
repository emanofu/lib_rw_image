#ifndef igs_tif_write_strip_h
#define igs_tif_write_strip_h

#include "igs_tif_write_handle.h"

namespace igs {
 namespace tif {
  class write_strip {
  public:
	write_strip(
		igs::tif::write_handle& handle_ref,
		const unsigned char *sour_top,
		const size_t sour_size
	);

	unsigned int count(void) const; /* segment_exec()�¹Բ�� */
	void segment_exec(
		igs::tif::write_handle& handle_ref
	); /* �ƽ���缡(��ʬ)���� */
  private:
	write_strip();

	const unsigned int number_of_strip_;
	unsigned int strip_counter_;
	const size_t sour_size_;
	const unsigned char *sour_current_; /* �������� */
	size_t write_encoded_strip_bytes_;
  };
 }
}

#endif /* !igs_tif_write_strip_h */
