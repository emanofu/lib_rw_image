#ifndef igs_image_readers_h
#define igs_image_readers_h

#include <string>
#include "igs_image_irw.h"
#if defined USE_IRW_PLUGIN //---------------------------------
#include "igs_path_irw.h"
#endif //-----------------------------------------------------

#ifndef IGS_IMAGE_READERS_EXPORT
# define IGS_IMAGE_READERS_EXPORT
#endif

/*
	2010-3-17
	classのメンバーにstd::vectorを使うと、
	MS-C(vc2005md)のDLLにできない(原因不明)。
------ ここから ------
        cl   /W3 /MD /EHa /O2 /wd4819 /I. /I..\src_common /I..\..\..\utility_too
ls\vc2005md\include /LD /DIGS_IMAGE_READERS_EXPORT="__declspec(dllexport)" /DUSE
_IRW_PLUGIN /Feigs_image_readers.dll igs_image_readers.cxx  ..\..\..\utility_too
ls\vc2005md\implib\libigs_resource_irw_imp.lib ..\..\..\utility_tools\vc2005md\i
mplib\libigs_path_irw_imp.lib ..\..\..\utility_tools\vc2005md\implib\libigs_reso
urce_log_imp.lib
/link /implib:libigs_image_readers_imp.lib
Microsoft(R) 32-bit C/C++ Optimizing Compiler Version 14.00.50727.762 for 80x86
Copyright (C) Microsoft Corporation.  All rights reserved.

igs_image_readers.cxx
src_master\igs_image_readers.h(84) : warning C4251: 'igs::image::readers::modules_2' : class 'std::vector<_Ty>' は __export キーワードを使って class 'igs::image::readers' にエクスポートしてください。
        with
        [
            _Ty=void *
        ]
------ ここまで ------
*/

namespace igs {
 namespace image {
  class IGS_IMAGE_READERS_EXPORT readers {
  public:
	readers(
#if defined USE_IRW_PLUGIN //---------------------------------
	 	const std::string& dir_path = igs::path::plugin_dir()
#endif //-----------------------------------------------------
	);
	void valid(void);			/* 全書式有効 */
	void valid(const int fmt_number);	/* 指定書式のみ有効 */
	void valid(const int fmt_number,const int ext_number);
					/* 指定書式&拡張子のみ有効 */
	const bool valid(const std::string& file_path); /* 拡張子判別 */

	const unsigned int size(void);		/* 書式数(validのみ)  */
	const unsigned int size(const int fmt_number);
						/* 拡張子数(validのみ)*/

	const char *name(const int fmt_number);	/* 書式名(validのみ)  */
	const char *name(const int fmt_number, const int ext_number);
						/* 拡張子名(validのみ)*/

	/* 画像データ識別と書式の設定 */
	void check(const std::string& file_path);

	/* 画像情報得る */
	void get_info(const std::string& file_path, std::string& info);

	/* command line用情報項目を得る */
	void get_line_head(std::string& line_head);
	/* command line用画像情報を得る */
	void get_line_info(
		const std::string& file_path
		, std::string& line_info
	);

	/* 画像メモリサイズを得る */
	void get_size(
		const std::string& file_path
		, unsigned int& image_size
		, unsigned int& history_size
	);

	/* 画像を得る */
	void get_data(
		const std::string& file_path
		, igs::image::properties &props
		, const unsigned int image_size
		, unsigned char *image_array
		, std::string& history_str
		, bool&break_sw /* Job中断制御 */
		, const int segment_bytes = 0x7fffffff /* Job中断間隔 */
			/* break_swを使う場合1,000,000(=2^20)くらい */
	);

	/* plugin解除 */
	void clear(void);

 	~readers();

  private:
	const char *valid_ext_ptr_;
	int valid_reader_num_;
	int current_num_;
	int module_count_;
  };
 }
}

#endif /* !igs_image_readers_h */
