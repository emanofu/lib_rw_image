#include <stdexcept>	// std::domain_error(-)
#include "igs_resource_msg_from_err.h"
#include "igs_resource_dir_list.h"

namespace {
  void set_list_(
	WIN32_FIND_DATA&fd
	, std::set< std::basic_string<TCHAR> > &set_list
  ) {
	std::string file_name( fd.cFileName );
	if (("."  != file_name) && (".." != file_name)
	  && !(fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) /* for admin */
	  && !(fd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM) /* for OS */
	) {
		std::basic_string<TCHAR> str(
			(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			? TEXT("d")
			: TEXT("f")
		);
		str += fd.cFileName;
		set_list.insert(str);
	}
  }
}
void igs::resource::dir_list(
	LPCTSTR lpFileName
	, std::set< std::basic_string<TCHAR> > &set_list
) {
	HANDLE hSearch=0;
	WIN32_FIND_DATA fd; ::ZeroMemory(&fd,sizeof(fd));
	std::basic_string<TCHAR> find_path(lpFileName);
	find_path += TEXT("\\*");
	hSearch = ::FindFirstFile( find_path.c_str(), &fd );
	if (INVALID_HANDLE_VALUE == hSearch) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),find_path.c_str()
		));
	}
	set_list_( fd, set_list );
  try {
   for (;;) {
	if ( !::FindNextFile( hSearch, &fd ) ) {
	 const DWORD dw = ::GetLastError();
	 if (ERROR_NO_MORE_FILES == dw) { break; }
	 else {
		throw std::domain_error(igs::resource::msg_from_err(
			dw,find_path.c_str()
		));
	 }
	}
	set_list_( fd, set_list );
   }
  }
  catch(...) {
	::FindClose( hSearch ); /* このエラーは無視 */
	throw; /* try内のエラーを再投 */
  }
	if (!::FindClose( hSearch )) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),find_path.c_str()
		));
	}
}
