#include "igs_path_irw.h"

const std::string igs::path::ext_from_path(const std::string&file_path)
{
	std::string::size_type index = file_path.rfind('.');
	if (index == std::string::npos) { return ""; }
	return file_path.substr(++index);
}

const std::string igs::path::plugin_dir(
	 const std::string& plugin_dir
	,const std::string& plugin_type_dir
	,const std::string& plugin_top_dir
	,const std::string& compile_type_dir
	,const std::string& root_dirpath
) {
	std::string path;

	if (root_dirpath.empty()) {
		path += "C:/plugin"	// Root folder default
	} else {path += root_dirpath;}

	path += '/';

	if (compile_type_dir.empty()) {
		path += "vc2008mdAMD64"; // CompileType folder default
	} else {path += compile_type_dir;}

	path += '/';

	if (plugin_top_dir.empty()) {
		path += "plugin";
	} else {path += plugin_top_dir;}

	path += '/';

	if (plugin_type_dir.empty()) {
		path += "image";
	} else {path += plugin_type_dir;}

	path += '/';

	if (plugin_dir.empty()) { // open/save
		path += "open";
	} else {path += plugin_dir;}

	return path;
}
#if 0 //----------------------------------------------------------------
#include <iostream>
int main() 
{
	std::cout
 << "piC<" << igs::path::ext_from_path("n:/tmp/tmp.0001.piC") << ">\n"
 << "pic<" << igs::path::ext_from_path("n:/tmp/tmp.0001.pic") << ">\n"
 << "tmp<" << igs::path::ext_from_path("n:/tmp/tmp") << ">\n"
 << "0001.<" << igs::path::ext_from_path("n:/tmp/tmp.0001.") << ">\n"
 << "rgb<" << igs::path::ext_from_path("./tmp.rgb") << ">\n"
 << "plugin_dir<" << igs::path::plugin_dir(0,0,0,0,0) << ">\n"
	;
	return 0;
}
#endif //---------------------------------------------------------------
