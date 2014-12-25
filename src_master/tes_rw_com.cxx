#include <vector> /* std::vector  */
#include <iostream> /* std::cerr  */
#include <stdexcept> /* std::domain_error  */
#include <sstream> /* std::ostringstream  */
#include "igs_resource_irw.h" // igs::resource::stop_watch
// #include "igs_image_irw.h"
#include "igs_image_readers.h"
#include "igs_image_writers.h"

#if defined USE_IRW_PLUGIN
#include <vector>
#endif

namespace {
 void print_rdrs_valid_(igs::image::readers&rdrs,const char*path){
	std::cout << "readers : path\n\t" << path << "\n\textension is ";
	if (rdrs.valid(path))	{ std::cout << "OK"; }
	else			{ std::cout << "NG"; }
	std::cout << '\n';
 }
 void print_rdrs_name_(igs::image::readers&rdrs) {
	std::cout << "readers : name" << "\n";
	for (unsigned int ii=0; ii<rdrs.size(); ++ii) {
		std::cout << '\t' << rdrs.name(ii) << '\n';
		if (0 < rdrs.size()) {
		 for (unsigned int jj=0; jj<rdrs.size(ii); ++jj) {
			if (0 == jj) { std::cout << "\t\t"; }
			else         { std::cout << ','; }
			std::cout << rdrs.name(ii,jj);
		 }
		 std::cout << '\n';
		}
	}
 }
 void print_rdrs_info_(igs::image::readers&rdrs,const char*path) {
	std::string info;
	info += "readers : info line -----------------------------\n";
	rdrs.get_line_head(info);
	rdrs.get_line_info(path,info);
	info += "readers : info detail ---------------------------\n";
	rdrs.get_info(path,info);
	info += "readers : info end ------------------------------\n";
	std::cout << info;
 }
 void print_rdrs_sz_(const int image_size,const int history_size) {
	std::cout
		<< "image_size   " << image_size   << '\n'
		<< "history_size " << history_size << '\n'
		;
 }
 void print_wtrs_valid_(const int fmt_number,const char*path){
	std::cout
		<< "writers : path\n\t" << path
		<< "\n\tfmt_number " << fmt_number
		<< "\n\textension is ";
	if (fmt_number < 0)	{ std::cout << "NG"; }
	else			{ std::cout << "OK"; }
	std::cout << '\n';
 }
 void print_wtrs_name_(igs::image::writers&wtrs) {
	std::cout << "writers : name" << "\n";
	for (unsigned int ii=0; ii<wtrs.size(); ++ii) {
		std::cout << '\t' << wtrs.name(ii) << '\n';
		if (0 < wtrs.size(ii) || 0 < wtrs.compression_size(ii)){
		 for (unsigned int jj=0; jj<wtrs.size(ii); ++jj) {
			if (0 == jj) { std::cout << "\t\t"; }
			else         { std::cout << ','; }
			std::cout << wtrs.name(ii,jj);
		 }
		 std::cout << '\n';
		 for (unsigned int kk=0; kk<wtrs.compression_size(ii); ++kk) {
			if (0 == kk) { std::cout << "\t\t"; }
			else         { std::cout << ','; }
			std::cout << wtrs.compression_name(ii,kk);
		 }
		 std::cout << '\n';
		}
	}
 }
}
//--------------------------------------------------------------------
int main(int argc, char *argv[]) {
	if (3 != argc) { return 1; }
	const bool detail_verbode_sw = true;
 try {
	igs::resource::stop_watch stopw;
stopw.start();
#if defined USE_IRW_PLUGIN //------
	std::string dirpath("C:/user/public"); // home
	dirpath += "/utility_tools";
	igs::image::readers rdrs
	(igs::path::plugin_dir("","","","",dirpath))
	;
#else //--------------------------
	igs::image::readers rdrs;
#endif //--------------------------
stopw.stop("readers : setup module");

	if (detail_verbode_sw) { print_rdrs_valid_(rdrs,argv[1]); }
	if (detail_verbode_sw) { print_rdrs_name_(rdrs); }
	if (!rdrs.valid(argv[1]))	{
		std::string str(argv[1]);
		str += " is invalid";
		throw std::domain_error(str);
	}

stopw.start();
	rdrs.check(argv[1]);
	if (detail_verbode_sw) { print_rdrs_info_(rdrs,argv[1]); }
	unsigned int image_size=0;
	unsigned int history_size=0;
	rdrs.get_size(argv[1], image_size, history_size);
	if (detail_verbode_sw){print_rdrs_sz_(image_size,history_size);}
	igs::image::properties props;
	std::vector<unsigned char> image(image_size);
	std::string history_str; history_str.resize(history_size);
	bool break_sw = false;
	rdrs.get_data(
		argv[1],props,image.size(),&image.at(0),history_str
		,break_sw
		,0x40000000 //=1073741824
	);
	rdrs.clear();
stopw.stop("readers : check,get_size,get_data,clear");
	//------------------------------------------------------------
stopw.start();
#if defined USE_IRW_PLUGIN //------
	igs::image::writers wtrs
  (igs::path::plugin_dir("save","","","",dirpath))
	;
#else //--------------------------
	igs::image::writers wtrs;
#endif //--------------------------
stopw.stop("writers : setup module");

	int fmt_number = wtrs.fmt_number_from_extension(argv[2]);
	if (detail_verbode_sw) { print_wtrs_valid_(fmt_number,argv[2]);}
	if (detail_verbode_sw) { print_wtrs_name_(wtrs); }

stopw.start();
	wtrs.check(argv[2]);
	wtrs.bad_properties(props);
	wtrs.put_data(
		props,image.size(),&image.at(0),1,history_str,argv[2]
	);
	wtrs.clear();
stopw.stop("writers : check,bad_properties,compression_number,put_data,clear");
 }
 catch(std::exception& e) {
	std::cerr << "exception<" << e.what() << ">" << std::endl;
 }
 catch(...) {
	std::cerr << "other exception" << std::endl;
 }
	return 0;
}
