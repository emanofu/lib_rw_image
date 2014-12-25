/* ファイル情報を取得し、コンソールに表示する */
#include <windows.h>
#include <shlwapi.h>	// ::PathIsDirectory(-)
#include <iomanip>	// std::setw()
#include <sstream>	// std::ostringstream
#include <stdexcept>	// std::domain_error

#include "igs_resource_msg_from_err.h"
#if defined DEBUG_IGS_RESOURCE_FILE_STAT_WIN
# include "igs_resource_msg_from_err.cxx"
#endif  /* !DEBUG_IGS_RESOURCE_FILE_STAT_WIN */

#include "igs_resource_file_stat.h"

/* エラーのときはゼロを返す */
using igs::resource::file_stat;
file_stat::file_stat(LPCTSTR file_path)
	: is_exist(false)
	, is_dir(false), is_file(false)
	, bytes_size(0)
	, a_year(0),a_mon(0),a_mday(0),a_hour(0),a_min(0),a_sec(0)
	, m_year(0),m_mon(0),m_mday(0),m_hour(0),m_min(0),m_sec(0)
	, c_year(0),c_mon(0),c_mday(0),c_hour(0),c_min(0),c_sec(0)
{
#if 0//---------------------------------------------------------------
	// if (::PathFileExists(file_path)==FALSE) { return; }

	const HANDLE handle = ::CreateFile(
		file_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING
		,FILE_ATTRIBUTE_NORMAL,NULL
	);
	if (INVALID_HANDLE_VALUE == handle) {
#if defined DEBUG_IGS_RESOURCE_FILE_STAT_WIN
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
#endif  /* !DEBUG_IGS_RESOURCE_FILE_STAT_WIN */
		return;
	}
#endif//--------------------------------------------------------------

	HANDLE handle = 0;
	if (::PathIsDirectory(file_path)) {
		handle = ::CreateFile(
			file_path ,GENERIC_READ ,FILE_SHARE_READ ,NULL
			,OPEN_EXISTING ,FILE_FLAG_BACKUP_SEMANTICS ,NULL
		);
		this->dir_path = file_path;
		//this->file_name = "";
	} else {
		handle = ::CreateFile(
			file_path ,GENERIC_READ ,FILE_SHARE_READ ,NULL
			,OPEN_EXISTING ,FILE_ATTRIBUTE_NORMAL ,NULL
		);
		std::string filepath(file_path);
		std::string::size_type index = filepath.find_last_of("/\\");
		if (std::string::npos == index) {
			this->dir_path = ".";
			this->file_name = filepath;
		} else {
			this->dir_path  = filepath.substr(0,index);
			this->file_name = filepath.substr(index+1);
		}
	}
	if (INVALID_HANDLE_VALUE == handle) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}

	BY_HANDLE_FILE_INFORMATION fileInfo;
	if (0 == ::GetFileInformationByHandle(handle, &fileInfo)) {
		::CloseHandle(handle);
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
	if (0 == ::CloseHandle(handle)) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}

	this->is_exist = true;
	DWORD file_attributes = fileInfo.dwFileAttributes;
	this->is_dir  = (0 != (file_attributes&FILE_ATTRIBUTE_DIRECTORY));
	this->is_file = (0 == (file_attributes&FILE_ATTRIBUTE_DIRECTORY));
	this->bytes_size = fileInfo.nFileSizeLow;

	FILETIME localTime;
	SYSTEMTIME systemTime;

	::FileTimeToLocalFileTime(&fileInfo.ftCreationTime,&localTime);
	::FileTimeToSystemTime(&localTime, &systemTime);
	this->c_year = systemTime.wYear;
	this->c_mon  = systemTime.wMonth;
	this->c_mday = systemTime.wDay;
	this->c_hour = systemTime.wHour;
	this->c_min  = systemTime.wMinute;
	this->c_sec  = systemTime.wSecond;
		// systemTime.wMilliseconds,
//const char *dayOfWeek[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
		// dayOfWeek[systemTime.wDayOfWeek])

	::FileTimeToLocalFileTime(&fileInfo.ftLastWriteTime,&localTime);
	::FileTimeToSystemTime(&localTime, &systemTime);
	this->m_year = systemTime.wYear;
	this->m_mon  = systemTime.wMonth;
	this->m_mday = systemTime.wDay;
	this->m_hour = systemTime.wHour;
	this->m_min  = systemTime.wMinute;
	this->m_sec  = systemTime.wSecond;

	::FileTimeToLocalFileTime(&fileInfo.ftLastAccessTime,&localTime);
	::FileTimeToSystemTime(&localTime, &systemTime);
	this->a_year = systemTime.wYear;
	this->a_mon  = systemTime.wMonth;
	this->a_mday = systemTime.wDay;
	this->a_hour = systemTime.wHour;
	this->a_min  = systemTime.wMinute;
	this->a_sec  = systemTime.wSecond;
}
void file_stat::form_stat(std::ostringstream& os) {
	 os << "Dir : \"" << this->dir_path << "\"\n";
	if (this->is_dir) {
	 os << "Dir\n";
	}
	if (this->is_file) {
	 os << "File\n";
	}
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

	std::ostringstream os_change;
	os_change
	 << this->c_year
	 << '-' << std::setw(2) << std::setfill('0') << this->c_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->c_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->c_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->c_min
	 << ':' << std::setw(2) << std::setfill('0') << this->c_sec;
	os << " CreationTime : " << os_change.str() << '\n';

	std::ostringstream os_modify;
	os_modify
	 << this->m_year
	 << '-' << std::setw(2) << std::setfill('0') << this->m_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->m_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->m_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->m_min
	 << ':' << std::setw(2) << std::setfill('0') << this->m_sec;
	os << " LastWriteTime : " << os_modify.str() << '\n';

	std::ostringstream os_access;
	os_access
	 << this->a_year
	 << '-' << std::setw(2) << std::setfill('0') << this->a_mon
	 << '-' << std::setw(2) << std::setfill('0') << this->a_mday
	 << ' ' << std::setw(2) << std::setfill('0') << this->a_hour
	 << ':' << std::setw(2) << std::setfill('0') << this->a_min
	 << ':' << std::setw(2) << std::setfill('0') << this->a_sec;
	os << " LastAccessTime : " << os_access.str() << '\n';
}
#if defined DEBUG_IGS_RESOURCE_FILE_STAT_WIN
#include <iostream>
int main(int argc,char *argv[])
{
	if (2 != argc) {
		std::cout
		<< "Usage : " << argv[0] << " filepath" << std::endl;
		return 1;
	}
 try {
	igs::resource::file_stat filestat(argv[1]);

	std::cout << "\"" << argv[1] <<"\"" << std::endl;

	std::ostringstream os;
	filestat.form_stat(os);
	std::cout << os.str() << std::endl;
 }
 catch(std::exception&e) {
	std::cerr << "exception=\"" << e.what() << "\"\n";
 }
 catch(...) {
	std::cerr << "ohter exception\n";
 }

	return 0;
}
#endif  /* !DEBUG_IGS_RESOURCE_FILE_STAT_WIN */
/*
rem vc2008sp1 32/64bits :196,203 w! tes_win_igs_resource_file_stat_win.bat
set SRC=igs_resource_file_stat_win
set TGT=tes_%PROCESSOR_ARCHITECTURE%_igs_resource_file_stat_win
set INC=../../l01log/src
cl /W4 /WX /MD /EHa /O2 /wd4819 /DDEBUG_IGS_RESOURCE_FILE_STAT_WIN /I%INC% %SRC%.cxx shlwapi.lib /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
*/
