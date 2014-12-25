#ifndef igs_resource_file_stat_h
#define igs_resource_file_stat_h

/*--- �ե�����ξ���(°����security��timestamp����)�����뤿��Υ��饹 ---*/

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# include <string> // std::string
# include <sstream> // std::ostringstream
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  class IGS_RESOURCE_IRW_EXPORT file_stat {
  public:
	explicit file_stat(LPCTSTR lpFileName);
	void form_stat(std::ostringstream& os);
	std::string dir_path;
	std::string file_name;

	bool is_exist;
	bool is_dir, is_file;
	unsigned int bytes_size;
	int a_year,a_mon,a_mday,a_hour,a_min,a_sec; // LastAccessTime
	int m_year,m_mon,m_mday,m_hour,m_min,m_sec; // LastWriteTime
	int c_year,c_mon,c_mday,c_hour,c_min,c_sec; // CreationTime
  };
 }
}
#else //--------------------------------------------------------------
# include <string> // std::string
# include <sstream> // std::ostringstream
namespace igs {
 namespace resource {
  class file_stat {
  public:
	explicit file_stat(const char* file_path);
	void form_stat(std::ostringstream& os);
	std::string dir_path;
	std::string file_name;

	bool is_exist;
	bool is_dir, is_file;
	unsigned int bytes_size;
	int a_year,a_mon,a_mday,a_hour,a_min,a_sec; // last access
	int m_year,m_mon,m_mday,m_hour,m_min,m_sec; // last modification
	int c_year,c_mon,c_mday,c_hour,c_min,c_sec; // last status change
  };
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_file_stat_h */
