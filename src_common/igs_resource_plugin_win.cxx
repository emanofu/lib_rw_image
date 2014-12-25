#include <windows.h>
#include <stdexcept>	// std::domain_error

#include "igs_resource_msg_from_err.h"
#if defined DEBUG_PLUGIN_WIN
# include "igs_resource_msg_from_err.cxx"
#endif  /* !DEBUG_PLUGIN_WIN */

namespace {
 HMODULE dynamic_library_open_(LPCTSTR file_path) {
	HMODULE handle = ::LoadLibrary(file_path);
	if (NULL == handle) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(), file_path
		));
	}
	return handle;
 }
 FARPROC dynamic_library_symbol_(
	HMODULE plugin_handle,const char* str_symbol
 ) {
/* FARPROCの定義(msdn.microsoft.com/...より) int (FAR WINAPI * FARPROC)() */
	/* 関数シンボルへのアドレスを返す */
	FARPROC p_proc = ::GetProcAddress(plugin_handle,str_symbol);
	if (NULL == p_proc) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(), str_symbol
		));
	}
	return p_proc;
 }
 void dynamic_library_close_(HMODULE plugin_handle) {
	if (0 != plugin_handle) { ::FreeLibrary(plugin_handle); }
 }
}
/* 使用条件:
	アプリのトップの場所でスタックとして使う
	エラーはこの関数の呼び出し親がcatchして
	メッセージ出して次のplugin追加へ継続したい
*/
#include <string>	// std::string
#include "igs_resource_plugin.h"
void igs::resource::plugins_add(
	LPCTSTR file_path
	,std::vector<HMODULE>&libraries
	,std::vector<void *>&symbols
) {
 try {
	HMODULE plugin_handle = dynamic_library_open_(file_path);
	libraries.push_back(plugin_handle);

	const char *cp_symbol = "plugin_main_func";
	void *(*p_func)(void);
	*(void **)(&p_func) = dynamic_library_symbol_(
		plugin_handle,cp_symbol
	);
	void *param_handle = p_func();
	if (0 == param_handle) {
		std::basic_string<TCHAR> str;
		str += "file \""; str += file_path;
		str += "\" symbol\""; str += cp_symbol;
		str += "\" function returns NULL";
		throw std::domain_error(str);
	}
	symbols.push_back(param_handle);
 }
 catch(std::exception& e) {
	e; /* for stop warning C4101 */
	igs::resource::plugins_close(libraries);
	throw;
 }
 catch(...) {
	igs::resource::plugins_close(libraries);
	throw;
 }
}
void igs::resource::plugins_close(
	std::vector<HMODULE>&libraries
) {
	for (std::vector<HMODULE>::iterator
	it =  libraries.begin();
	it != libraries.end(); ++it) {
		dynamic_library_close_(*it);
	}
	libraries.clear();
}
#if defined DEBUG_PLUGIN_WIN
#include <iostream> /* std::cerr */
#include "igs_image_irw.h"
int main(int argc,char *argv[])
{
#if defined _WIN64
	const LPCTSTR openpic = "C:/users/public/utility_tools/vc2008x64md/plugin/image/open/pic.dll" ;
	const LPCTSTR savepic = "C:/users/public/utility_tools/vc2008x64md/plugin/image/save/pic.dll" ;
	const LPCTSTR opensgi = "C:/users/public/utility_tools/vc2008x64md/plugin/image/open/sgi.dll";
	const LPCTSTR savesgi = "C:/users/public/utility_tools/vc2008x64md/plugin/image/save/sgi.dll";
	const LPCTSTR opentga = "C:/users/public/utility_tools/vc2008x64md/plugin/image/open/tga.dll";
	const LPCTSTR savetga = "C:/users/public/utility_tools/vc2008x64md/plugin/image/save/tga.dll";
	const LPCTSTR opentif = "C:/users/public/utility_tools/vc2008x64md/plugin/image/open/tif.dll";
	const LPCTSTR savetif = "C:/users/public/utility_tools/vc2008x64md/plugin/image/save/tif.dll";
#else
	const LPCTSTR openpic = "C:/users/public/utility_tools/vc2008x32md/plugin/image/open/pic.dll" ;
	const LPCTSTR savepic = "C:/users/public/utility_tools/vc2008x32md/plugin/image/save/pic.dll" ;
	const LPCTSTR opensgi = "C:/users/public/utility_tools/vc2008x32md/plugin/image/open/sgi.dll";
	const LPCTSTR savesgi = "C:/users/public/utility_tools/vc2008x32md/plugin/image/save/sgi.dll";
	const LPCTSTR opentga = "C:/users/public/utility_tools/vc2008x32md/plugin/image/open/tga.dll";
	const LPCTSTR savetga = "C:/users/public/utility_tools/vc2008x32md/plugin/image/save/tga.dll";
	const LPCTSTR opentif = "C:/users/public/utility_tools/vc2008x32md/plugin/image/open/tif.dll";
	const LPCTSTR savetif = "C:/users/public/utility_tools/vc2008x32md/plugin/image/save/tif.dll";
#endif
 try {
	std::vector<HMODULE> libraries;
	std::vector<void *> symbols;
	igs::resource::plugins_add(openpic,libraries,symbols);
	igs::resource::plugins_add(savepic,libraries,symbols);
	igs::resource::plugins_add(opensgi,libraries,symbols);
	igs::resource::plugins_add(savesgi,libraries,symbols);
	igs::resource::plugins_add(opentga,libraries,symbols);
	igs::resource::plugins_add(savetga,libraries,symbols);
	igs::resource::plugins_add(opentif,libraries,symbols);
	igs::resource::plugins_add(savetif,libraries,symbols);
	for (unsigned ii = 0; ii < symbols.size(); ++ii) {
	  igs::image::reader *param_handle_ptr =
		static_cast<igs::image::reader *>(symbols.at(ii));
	  std::cout << "name=\"" << param_handle_ptr->name() << "\"\n";
	  for (unsigned jj = 0; jj < param_handle_ptr->ext_count(); ++jj) {
	  	std::cout
			<< "ext=\""
			<< param_handle_ptr->ext_lists()[jj]
			<< "\"\n";
	  }
	}
	igs::resource::plugins_close(libraries);
 }
 catch(std::exception& e) {
	std::cerr << "exception <" << e.what() << ">" << std::endl;
 }
 catch(...) {
	std::cerr << "other exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_PLUGIN_WIN */
/*
rem -- WindowsXp sp3(32bits) vc2008sp1  :102,108 w! tes_w32_plugin_win.bat
set SRC=igs_resource_plugin_win
set TGT=tes_w32_plugin_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_PLUGIN_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem -- Windows7 (64bits) vc2008sp1  :109,115 w! tes_w64_plugin_win.bat
set SRC=igs_resource_plugin_win
set TGT=tes_w64_plugin_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_PLUGIN_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
*/
