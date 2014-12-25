#ifndef igs_path_irw_h
#define igs_path_irw_h

/*
	2010-02-01
	OS������user_home,share_home������path��resource�˴ޤ�
	�����Ǥ��ȼ��θ���ѥ��䡢�ѥ������ؿ����������
	�ƽ񼰤�code�ˤϻȤ�ʤ������ƽ񼰤Υƥ��ȤǻȤ����ᤳ�������
*/
#include <string> /* std::string */

#ifndef IGS_PATH_IRW_EXPORT
# define IGS_PATH_IRW_EXPORT
#endif

namespace igs {
 namespace path {	/* �ե�����ѥ��γ�ĥ�Ҥ��Ф������ */
  IGS_PATH_IRW_EXPORT const std::string ext_from_path(
	const std::string& file_path
  );
  IGS_PATH_IRW_EXPORT const std::string plugin_dir(
	 const std::string& plugin_dir = ""
	,const std::string& plugin_type_dir = ""
	,const std::string& plugin_top_dir = ""
	,const std::string& compile_type_dir = ""
	,const std::string& root_dirpath = ""
  );
 }
}

#endif /* !igs_path_irw_h */
