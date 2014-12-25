#include <dlfcn.h>	// dlopen(),dlerror(),dlsym(),dlclose()
#include <stdexcept>	// std::domain_error

namespace {
 void *dynamic_library_open_(const char* file_path) {
	/* .soファイルをロードしハンドルを返す */
	void *p_lib = ::dlopen(file_path,RTLD_LAZY);
	if (NULL == p_lib) {
		std::string msg("dlopen(-) : "); msg += dlerror();
		throw std::domain_error(msg);
	}
	return p_lib;
 }
 void *dynamic_library_symbol_(
	void *plugin_handle
	,const char* str_symbol
 ) {
	/* シンボルへのアドレスを返す */
	void *p_sym = ::dlsym( plugin_handle, str_symbol );
	if (NULL == p_sym) {
		::dlclose(plugin_handle);
		std::string msg("dlsym(-) : "); msg += dlerror();
		throw std::domain_error(msg);
	}
	return p_sym;
 }
 void dynamic_library_close_(void *plugin_handle) {
	if (0 != plugin_handle) {
		::dlclose(plugin_handle);
	}
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
	const char* file_path
	,std::vector<void *>&libraries
	,std::vector<void *>&symbols
) {
 try {
	void *plugin_handle = dynamic_library_open_(file_path);
	libraries.push_back(plugin_handle);

	const char *cp_symbol = "plugin_main_func";
	void *(*p_func)(void);
	*(void **)(&p_func) = dynamic_library_symbol_(
		plugin_handle,cp_symbol
	);
	void *param_handle = p_func();
	if (0 == param_handle) {
 		std::string str;
		str += "file \""; str += file_path;
		str += "\" symbol\""; str += cp_symbol;
		str += "\" function returns NULL";
		throw std::domain_error(str);
	}
	symbols.push_back(param_handle);
 }
 catch(std::exception& e) {
	igs::resource::plugins_close(libraries);
	throw;
 }
 catch(...) {
	igs::resource::plugins_close(libraries);
	throw;
 }
}
void igs::resource::plugins_close(
	std::vector<void *>&libraries
) {
	for (std::vector<void *>::iterator
	it =  libraries.begin();
	it != libraries.end(); ++it) {
		dynamic_library_close_(*it);
	}
	libraries.clear();
}

#if defined DEBUG_PLUGIN_UNIX
#include <iostream>	// std::cerr std::cout
#include "igs_image_irw.h"
int main(int argc,char *argv[])
{
	const char *openpic = "C:/users/public/utility_tools/rhel4/plugin/image/open/pic.so" ;
	const char *savepic = "C:/users/public/utility_tools/rhel4/plugin/image/save/pic.so" ;
	const char *opensgi = "C:/users/public/utility_tools/rhel4/plugin/image/open/sgi.so";
	const char *savesgi = "C:/users/public/utility_tools/rhel4/plugin/image/save/sgi.so";
	const char *opentga = "C:/users/public/utility_tools/rhel4/plugin/image/open/tga.so";
	const char *savetga = "C:/users/public/utility_tools/rhel4/plugin/image/save/tga.so";
	const char *opentif = "C:/users/public/utility_tools/rhel4/plugin/image/open/tif.so";
	const char *savetif = "C:/users/public/utility_tools/rhel4/plugin/image/save/tif.so";
 try {
	std::vector<void *> libraries;
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
#endif  /* !DEBUG_PLUGIN_UNIX */
/*
# ------ rhel4 (32bits)  :114,115 w! tes_u32_plugin_unix.csh
g++ -Wall -O2 -g -I. -DDEBUG_PLUGIN_UNIX igs_resource_plugin_unix.cxx -ldl -o tes_u32_plugin_unix
*/
