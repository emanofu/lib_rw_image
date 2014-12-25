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
	class�Υ��С���std::vector��Ȥ��ȡ�
	MS-C(vc2005md)��DLL�ˤǤ��ʤ�(��������)��
------ �������� ------
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
src_master\igs_image_readers.h(84) : warning C4251: 'igs::image::readers::modules_2' : class 'std::vector<_Ty>' �� __export ������ɤ�Ȥä� class 'igs::image::readers' �˥������ݡ��Ȥ��Ƥ���������
        with
        [
            _Ty=void *
        ]
------ �����ޤ� ------
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
	void valid(void);			/* ����ͭ�� */
	void valid(const int fmt_number);	/* ����񼰤Τ�ͭ�� */
	void valid(const int fmt_number,const int ext_number);
					/* �����&��ĥ�ҤΤ�ͭ�� */
	const bool valid(const std::string& file_path); /* ��ĥ��Ƚ�� */

	const unsigned int size(void);		/* �񼰿�(valid�Τ�)  */
	const unsigned int size(const int fmt_number);
						/* ��ĥ�ҿ�(valid�Τ�)*/

	const char *name(const int fmt_number);	/* ��̾(valid�Τ�)  */
	const char *name(const int fmt_number, const int ext_number);
						/* ��ĥ��̾(valid�Τ�)*/

	/* �����ǡ������̤Ƚ񼰤����� */
	void check(const std::string& file_path);

	/* ������������ */
	void get_info(const std::string& file_path, std::string& info);

	/* command line�Ѿ�����ܤ����� */
	void get_line_head(std::string& line_head);
	/* command line�Ѳ������������ */
	void get_line_info(
		const std::string& file_path
		, std::string& line_info
	);

	/* �������ꥵ���������� */
	void get_size(
		const std::string& file_path
		, unsigned int& image_size
		, unsigned int& history_size
	);

	/* ���������� */
	void get_data(
		const std::string& file_path
		, igs::image::properties &props
		, const unsigned int image_size
		, unsigned char *image_array
		, std::string& history_str
		, bool&break_sw /* Job�������� */
		, const int segment_bytes = 0x7fffffff /* Job���Ǵֳ� */
			/* break_sw��Ȥ����1,000,000(=2^20)���餤 */
	);

	/* plugin��� */
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
