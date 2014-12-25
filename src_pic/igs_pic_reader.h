#ifndef igs_pic_reader_h
#define igs_pic_reader_h

#include "igs_image_irw.h"

namespace igs {
 namespace pic {
  class reader : public igs::image::reader {
  public:
	const char *const name(void) const; /* ファイル書式名 */
	const unsigned int ext_count(void);
	const char *const *const ext_lists(void);

	const bool istag(
		const size_t byte_count
		, const unsigned char *const tag_array);
	void get_info(
		const std::string& file_path
		, std::string& info);
	void get_line_head(
		std::string& line_head);
	void get_line_info(
		const std::string& file_path
		, std::string& line_info);
	void get_size( /* 用意すべきサイズを得る */
		const std::string& file_path
		, size_t& image_bytes
		, size_t& history_bytes);
	void get_data(
		const std::string& file_path
		, igs::image::properties& props
		, const size_t image_bytes
		, unsigned char *image_array
		, std::string& history_str
		, bool& break_sw /* 中断Switch。外からtureで中断 */
		, const size_t segment_bytes = 1000000
			/* 分割読込の個々サイズ(tif以外で使用する) */
	);

  private:
	static const char *const name_; /* ファイル書式名 */
  };
 }
}

#endif /* !igs_pic_reader_h */
