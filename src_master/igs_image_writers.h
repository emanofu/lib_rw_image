#ifndef igs_image_writers_h
#define igs_image_writers_h

#include <string>
#include "igs_image_irw.h"
#if defined USE_IRW_PLUGIN //---------
#include "igs_path_irw.h"
#endif //------------------------------------------

#ifndef IGS_IMAGE_WRITERS_EXPORT
# define IGS_IMAGE_WRITERS_EXPORT
#endif

/*
	2010-2-6
	classのメンバーにstd::vectorを使うと、
	MS-C(vc2005md)のDLLにできない(原因不明)。
*/

namespace igs {
 namespace image {
  class IGS_IMAGE_WRITERS_EXPORT writers {
  public:
	writers(
#if defined USE_IRW_PLUGIN //---------
	 const std::string& dir_path =
		igs::path::plugin_dir("","","","","save")
#endif //-----------------------------
	);

	const unsigned int size(void);		/* 書式数   */
	const unsigned int size(
			 const int fmt_number);	/* 拡張子数 */

	const char *name(const int fmt_number);	/* 書式名   */
	const char *name(const int fmt_number,const int ext_number);
						/* 拡張子名 */

	const unsigned int compression_size(	const int fmt_number);
	const char *compression_name(		const int fmt_number
					, const int comp_number);

	/* パスの拡張子が有効か調べる */
	const int fmt_number_from_extension(
		const std::string& file_path
	);

	/* 書式の設定 */
	void check(const std::string& file_path);

	/* propertiesが有効か調べる */
	const bool bad_properties(
		std::ostringstream& error_or_warning_msg
		, const igs::image::properties& props
	);
	void bad_properties(
		const igs::image::properties& props
	);

	void put_data(
		const igs::image::properties& prop
		, const unsigned int image_size
		, const unsigned char *image_array
		, const int compression_number
			/*	0    -> Not Compress
				1    -> Default Compress Type
				2... -> Other Compress Type(If Exist)
			*/
		, const std::string& history_str
		, const std::string& file_path
	);

	void clear(void);

	~writers();
  private:
	int current_num_;
	int current_ext_num_;
	int module_count_;
  };
 }
}

#endif /* !igs_image_writers_h */
