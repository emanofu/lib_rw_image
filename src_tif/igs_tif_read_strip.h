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

	unsigned int count(void) const; /* segment_exec()実行回数 */

	void segment_exec(
		igs::tif::read_handle& handle_ref
	); /* 呼出毎順次(部分)処理 */
  private:
	read_strip();

	const tsize_t  strip_size_;
	const tstrip_t number_of_strip_;
	unsigned int   strip_counter_;

	const size_t dest_size_;
	unsigned char *dest_current_; /* 外部メモリ参照 */

	size_t read_encoded_strip_bytes_;
  };
 }
}

#endif /* !igs_tif_read_strip_h */
