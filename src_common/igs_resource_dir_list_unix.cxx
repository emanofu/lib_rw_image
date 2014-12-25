#include <iostream>
#include <stdexcept>	// std::domain_error()
#include <sys/types.h>	// opendir(), readdir(), closedir()
#include <dirent.h>	// opendir(), readdir(), closedir()
#include <errno.h>	// errno
#include <cstring>	// strerror()

#include "igs_resource_msg_from_err.h"
#include "igs_resource_dir_list.h"

void igs::resource::dir_list(
	const char* dir_path
	, std::set<std::string>& set_list
) {
	DIR *directory_stream=::opendir(dir_path);
	if (NULL == directory_stream) { /* ���顼����NULL������ */
		throw std::domain_error(igs::resource::msg_from_err(
			errno, dir_path
		));
	}
 try {
	struct dirent *directory_entry=0;
	while (1) {
	  errno = 0;
	  directory_entry = ::readdir(directory_stream);
	  if (NULL == directory_entry) {/* ���顼���ե�����Ǹ��NULL�֤� */
	    if (0 != errno) {	/* ���顼 */
		throw std::domain_error(igs::resource::msg_from_err(
			errno, "readdir(-)"
		));
	    } else { break; }	/* �ե�����κǸ��ã���� */
	  }
	  std::string file_name( directory_entry->d_name );
	  if (("."  != file_name) && (".." != file_name)) {
	/* d_type �ϡ�rhel4,darwin8 �ˤ����ơ�DT_DIR(4)�ʤ�dir�Ǥ��� */
		std::string str( (DT_DIR&directory_entry->d_type)?"d":"f" );
		str += directory_entry->d_name; // p->d_name[NAME_MAX]
		set_list.insert(str);
	  }
	}
 }
 catch (...) {
	(void)::closedir(directory_stream); /* ���Υ��顼��̵�� */
	throw; /* try��Υ��顼����� */
 }
	if (0 != ::closedir(directory_stream)) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno, "closedir()"
		));
	}
}
