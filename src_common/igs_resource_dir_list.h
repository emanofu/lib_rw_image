#ifndef igs_resource_dir_list_h
#define igs_resource_dir_list_h

/* --- ディレクトリリストを読む - */

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# include <string> // std::basic_string<TCHAR>
# include <set>	// std::set
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  IGS_RESOURCE_IRW_EXPORT void dir_list(
	LPCTSTR lpFileName
	, std::set< std::basic_string<TCHAR> >&set_list
  );
 }
}
#else //--------------------------------------------------------------
# include <string> // std::string
# include <set>	// std::set
namespace igs {
 namespace resource {
  void dir_list(
	const char* dir_path
	, std::set<std::string>&set_list
  );
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_dir_list_h */
