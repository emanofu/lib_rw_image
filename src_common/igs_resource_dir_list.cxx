#include "igs_resource_dir_list.h"

#if defined _WIN32
#include "igs_resource_dir_list_win.cxx"
#else
#include "igs_resource_dir_list_unix.cxx"
#endif

#if defined DEBUG_IGS_RESOURCE_DIR_LIST
# include <iostream>
# include "igs_resource_msg_from_err.cxx"
int main(int argc,char *argv[])
{
	if (2 != argc) {
		std::cout << "Usage : " << argv[0] << " dir_path\n";
		return 0;
	}
#if defined _WIN32
	std::set< std::basic_string<TCHAR> > sorting_list;
#else
	std::set<std::string> sorting_list;
#endif
 try {
	std::cout << "dir_path " << '\"' << argv[1] << '\"' <<std::endl;
	igs::resource::dir_list(
		igs::resource::from_mbcs(argv[1]).c_str()
		, sorting_list
	);
 }
 catch (std::exception& e) {
	std::cerr << "exception <" << e.what() << ">" << std::endl;
 }
 catch (...) {
	std::cerr << "other exception" << std::endl;
 }

#if defined _WIN32
	for (std::set<std::basic_string<TCHAR> >::iterator
	it = sorting_list.begin(); sorting_list.end() != it; ++it
	) {
		std::string str(igs::resource::mbcs_from(*it));
		std::cout
			<< str.at(0) << '\t' << '\"' << &(str.at(1)) << '\"'
			<< std::endl;
	}
#else
	for (std::set<std::string>::iterator
	it = sorting_list.begin(); sorting_list.end() != it; ++it
	) {
		std::cout
			<< it->at(0) << '\t' << '\"' << &(it->at(1)) << '\"'
			<< std::endl;
	}
#endif
	return 0;
}
#endif  /* !DEBUG_IGS_RESOURCE_DIR_LIST */
/*
rem Windows vc2008sp1 32/64bits :59,65 w! tes_win_igs_resource_dir_list.bat
set SRC=igs_resource_dir_list
set TGT=tes_%PROCESSOR_ARCHITECTURE%_dir_list
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_IGS_RESOURCE_DIR_LIST /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# rhel4 (32bits)  :21,22 w! tes_u32_dir_list.csh
g++ -Wall -O2 -g -DDEBUG_IGS_RESOURCE_DIR_LIST -I../../l01log/src igs_resource_dir_list.cxx -o tes_u32_dir_list
*/
