#ifndef igs_path_irw_h
#define igs_path_irw_h

/*
	2010-02-01
	OSが持つuser_home,share_home等々のpathはresourceに含む
	ここでは独自の固定パスや、パス処理関数を定義する
	各書式のcodeには使わないが、各書式のテストで使うためここに定義
*/
#include <string> /* std::string */

#ifndef IGS_PATH_IRW_EXPORT
# define IGS_PATH_IRW_EXPORT
#endif

namespace igs {
 namespace path {	/* ファイルパスの拡張子に対する操作 */
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
