#ifndef igs_pic_writer_h
#define igs_pic_writer_h

#include "igs_image_irw.h"

namespace igs {
 namespace pic {
#if 0 //-------------------------------
  namespace write {
   class properties {
   public:
	properties();
	unsigned char compression;
/*
igs::pic::header::uc_uncompres
igs::pic::header::uc_pure_rle
igs::pic::header::uc_mixed_rle
*/
   };
  }
#endif //------------------------------
  class writer : public igs::image::writer {
  public:
	writer();

	const char *const name(void) const; /* ファイル書式名 */

	const unsigned int ext_count(void);
	const char *const *const ext_lists(void);

	const unsigned int compression_count(void);
	const char *const *const compression_lists(void);

	const bool error_from_properties(
		const igs::image::properties& props
		, const int ext_number
		, std::ostringstream& error_or_warning_msg
	);
	void put_data(
		const igs::image::properties& prop
		, const size_t image_bytes
		, const unsigned char *image_array
		, const int compression_number
		, const std::string& history_str
		, const std::string& file_path
	);
  private:
	static const char *const name_; /* ファイル書式名 */
  };
 }
}

#endif /* !igs_pic_writer_h */
