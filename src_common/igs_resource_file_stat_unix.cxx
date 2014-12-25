#include <sys/types.h>	// stat()
#include <sys/stat.h>	// stat()
#include <unistd.h>	// stat()
#include <iomanip>	// std:setw()
#include <sstream>	// std::ostringstream
#include <string>	// std::string

#include "igs_resource_local_tm.h"
#if defined DEBUG_IGS_RESOURCE_FILE_STAT_UNIX
# include "igs_resource_local_tm.cxx"
#endif  /* !DEBUG_IGS_RESOURCE_FILE_STAT_UNIX */

#include "igs_resource_file_stat.h"

using igs::resource::file_stat;
file_stat::file_stat(const char* file_path)
	: is_exist(false)
	, is_dir(false), is_file(false)
	, bytes_size(0)
	, a_year(0),a_mon(0),a_mday(0),a_hour(0),a_min(0),a_sec(0)
	, m_year(0),m_mon(0),m_mday(0),m_hour(0),m_min(0),m_sec(0)
	, c_year(0),c_mon(0),c_mday(0),c_hour(0),c_min(0),c_sec(0)
{
	std::string filepath(file_path);
	std::string::size_type index = filepath.find_last_of("/\\");
	if (std::string::npos == index) {
		this->dir_path = ".";
		this->file_name = filepath;
	} else {
		this->dir_path  = filepath.substr(0,index);
		this->file_name = filepath.substr(index+1);
	}

	struct stat t_stat;
	if (0 != ::stat(file_path,&t_stat)) { return; /* not exists */ }

	this->is_exist = true;
	this->is_dir  = (0 != (t_stat.st_mode & S_IFDIR));
	this->is_file = (0 != (t_stat.st_mode & S_IFREG));
	this->bytes_size = static_cast<unsigned int>(t_stat.st_size);

	struct tm t_tm;
	igs::resource::local_tm(t_tm,t_stat.st_atime);
	this->a_year = t_tm.tm_year + 1900;
	this->a_mon  = t_tm.tm_mon + 1;
	this->a_mday = t_tm.tm_mday;
	this->a_hour = t_tm.tm_hour;
	this->a_min  = t_tm.tm_min;
	this->a_sec  = t_tm.tm_sec;

	igs::resource::local_tm(t_tm,t_stat.st_mtime);
	this->m_year = t_tm.tm_year + 1900;
	this->m_mon  = t_tm.tm_mon + 1;
	this->m_mday = t_tm.tm_mday;
	this->m_hour = t_tm.tm_hour;
	this->m_min  = t_tm.tm_min;
	this->m_sec  = t_tm.tm_sec;

	igs::resource::local_tm(t_tm,t_stat.st_ctime);
	this->c_year = t_tm.tm_year + 1900;
	this->c_mon  = t_tm.tm_mon + 1;
	this->c_mday = t_tm.tm_mday;
	this->c_hour = t_tm.tm_hour;
	this->c_min  = t_tm.tm_min;
	this->c_sec  = t_tm.tm_sec;
}
void file_stat::form_stat(std::ostringstream& os) {
	 os << "Dir : \"" << this->dir_path << "\"\n";
	 os << "File\n";
	 os << " Name : \"" << this->file_name << "\"";
	if (!this->is_exist) {
	 os << " is not exist\n";
	 return;
	}
	 os << '\n';
	 os << " Size : " << this->bytes_size << " byte";
	if (1 < this->bytes_size) {
	 os << "s";
	}
	 os << '\n';

	std::ostringstream os_access;
	os_access
	 << this->a_year
	 << '-' << std::setw(2) << std::setfill('0') << this->a_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->a_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->a_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->a_min
	 << ':' << std::setw(2) << std::setfill('0') << this->a_sec;
	os << " Access : " << os_access.str() << '\n';

	std::ostringstream os_modify;
	os_modify
	 << this->m_year
	 << '-' << std::setw(2) << std::setfill('0') << this->m_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->m_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->m_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->m_min
	 << ':' << std::setw(2) << std::setfill('0') << this->m_sec;
	os << " Modify : " << os_modify.str() << '\n';

	std::ostringstream os_change;
	os_change
	 << this->c_year
	 << '-' << std::setw(2) << std::setfill('0') << this->c_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->c_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->c_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->c_min
	 << ':' << std::setw(2) << std::setfill('0') << this->c_sec;
	os << " Change : " << os_change.str() << '\n';
}
#if defined DEBUG_IGS_RESOURCE_FILE_STAT_UNIX
#include <iostream>
int main(int argc,char *argv[])
{
	if (2 != argc) {
		std::cout
		<< "Usage : " << argv[0] << " filepath" << std::endl;
		return 1;
	}
	igs::resource::file_stat filestat(argv[1]);

	std::cout << "\"" << argv[1] <<"\"" << std::endl;

	std::ostringstream os;
	filestat.form_stat(os);
	std::cout << os.str() << std::endl;

	return 0;
}
#endif  /* !DEBUG_IGS_RESOURCE_FILE_STAT_UNIX */
/*
# rhel4 (32bits)  :133,134 w! tes_u32_igs_resource_file_stat_unix.csh
g++ -Wall -O2 -g -DDEBUG_IGS_RESOURCE_FILE_STAT_UNIX -I../../l01log/src igs_resource_file_stat_unix.cxx -o tes_u32_igs_resource_file_stat_unix
*/
