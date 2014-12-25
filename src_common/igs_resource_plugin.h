#ifndef igs_resource_plugin_h
#define igs_resource_plugin_h

/* --- .so or .dllファイルの読み込みクラス - */
/* 使用条件:
	アプリのトップの場所でスタックとして使う
	エラーはこの関数の呼び出し親がcatchして
	メッセージ出して次のplugin追加へ継続したい
*/

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# include <vector>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  /* 複数プラグイン管理して使う */
  IGS_RESOURCE_IRW_EXPORT void plugins_add(
	LPCTSTR file_path
	,std::vector<HMODULE>&libraries
	,std::vector<void *>&symbols
  );
  IGS_RESOURCE_IRW_EXPORT void plugins_close(
	std::vector<HMODULE>&libraries
  );
 }
}
#else //--------------------------------------------------------------
#include <vector>
namespace igs {
 namespace resource {
  /* 複数プラグイン管理して使う */
  void plugins_add(
	const char* file_path
	,std::vector<void *>&libraries
	,std::vector<void *>&symbols
  );
  void plugins_close(
	std::vector<void *>&libraries
  );
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_plugin_h */
