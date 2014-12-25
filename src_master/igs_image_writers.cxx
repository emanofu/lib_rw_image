#include <stdexcept> /* std::domain_error  */
#include <cassert> /* assert(-) */
#include <vector>
#include <fstream> /* std::ifstream */
#include "igs_resource_irw.h"
#include "igs_path_irw.h"
#include "igs_image_irw.h"
#include "igs_image_writers.h"

//--------------------------------------------------------------------
using igs::image::writers;

namespace { /* pluginは状態パラメータを持ってはならない */
	std::vector<void *> functon_;
}

#if defined USE_IRW_PLUGIN //------------------
namespace { /* pluginは状態パラメータを持ってはならない */
#if defined _WIN32
	std::vector<HMODULE> library_;
#else
	std::vector<void *> library_;
#endif
}
writers::writers(const std::string& dir_path):
	current_num_(-1)
	, current_ext_num_(-1)
	, module_count_(0)
{
	if (0 < library_.size()) { // already setuped plugin
		this->module_count_ = library_.size();
		return;
	}

	std::set<std::string> file_list;
	igs::resource::dir_list( dir_path.c_str(), file_list ); 

	if (file_list.size() <= 0) { // Not exist plugin files
		return;
	}

	library_.resize(file_list.size());
	functon_.resize(file_list.size());

	std::string str;
	for ( std::set<std::string>::iterator it = file_list.begin();
	 file_list.end() != it; ++it, ++this->module_count_
	) {
		if ('f' != it->at(0)) { continue; } // Not file
		str = dir_path; str += '/'; str += &(it->at(1));
		igs::resource::plugins_add(str.c_str(),library_,functon_);
	}
}
void writers::clear(void)
{
	this->current_num_ = -1;
	this->current_ext_num_ = -1;

	igs::resource::plugins_close(library_);
	this->module_count_ = 0;

	functon_.clear();
	library_.clear();
}
writers::~writers()
{ try { this->clear(); } catch(...) {} /* destractorの例外を無効化 */ }
#else //----static link------------------------
#include "igs_pic_writer.h"
#include "igs_sgi_writer.h"
#include "igs_tga_writer.h"
#include "igs_tif_writer.h"
namespace {
	igs::pic::writer cl_wtr_pic;
	igs::sgi::writer cl_wtr_sgi;
	igs::tga::writer cl_wtr_tga;
	igs::tif::writer cl_wtr_tif;
}
writers::writers():
	current_num_(-1)
	, current_ext_num_(-1)
	, module_count_(0)
{
	functon_.resize(4);
	functon_.at(this->module_count_++) = &cl_wtr_pic;
	functon_.at(this->module_count_++) = &cl_wtr_sgi;
	functon_.at(this->module_count_++) = &cl_wtr_tga;
	functon_.at(this->module_count_++) = &cl_wtr_tif;
}
void writers::clear(void)
{
	this->current_num_ = -1;
	this->current_ext_num_ = -1;
	this->module_count_ = 0;
	functon_.clear();
}
writers::~writers()
{ try { this->clear(); } catch(...) {} /* destractorの例外を無効化 */ }
#endif //--------------------------------------
//--------------------------------------------------------------------
/* 書式数 */
const unsigned int writers::size(void) {
	return this->module_count_;
}
/* 拡張子数 */
const unsigned int writers::size(const int fmt_number) {
	return static_cast<igs::image::writer *>(
		functon_.at(fmt_number)
	)->ext_count();
}

/* 書式名 */
const char *writers::name(const int fmt_number) {
	return static_cast<igs::image::writer *>(
		functon_.at(fmt_number)
	)->name();
}
/* 拡張子名 */
const char *writers::name(const int fmt_number, const int ext_number) {
	return static_cast<igs::image::writer *>(
		functon_.at(fmt_number)
	)->ext_lists()[ext_number];
}

/* compression数 */
const unsigned int writers::compression_size(const int fmt_number) {
	return static_cast<igs::image::writer *>(
		functon_.at(fmt_number)
	)->compression_count();
}
/* compression名 */
const char *writers::compression_name(const int fmt_number, const int comp_number) {
	return static_cast<igs::image::writer *>(
		functon_.at(fmt_number)
	)->compression_lists()[comp_number];
}
//--------------------------------------------------------------------
/* パスの拡張子名から、書式番号と拡張子番号を得る */
// #include <iostream>
namespace {
 void ext_to_number_(
	void **array
	, int count
	, const std::string& ext
	, int& fmt_number
	, int& ext_number
 ) {
// std::cout << "igs_image_writers.cxx : ext is " << ext << '\n';
	for (fmt_number = 0; fmt_number < count; ++fmt_number) {
// std::cout << "igs_image_writers.cxx : fmt_number is " << fmt_number << '\n';
		igs::image::writer *wptr =
			static_cast<igs::image::writer *>(array[fmt_number]);
		for (ext_number = 0;
		ext_number < static_cast<int>(wptr->ext_count());
		++ext_number) {
// std::cout << "igs_image_writers.cxx : extension is " << wptr->ext_lists()[ext_number] << '\n';
			if (wptr->ext_lists()[ext_number] == ext) {
				return;
			}
		}
	}
	fmt_number = -1;
	ext_number = -1;
 }
}
const int writers::fmt_number_from_extension(const std::string& file_path) {
	int fmt_number = -1;
	int ext_number = -1;
	ext_to_number_(
		&functon_.at(0)
		, this->module_count_
		, igs::path::ext_from_path(file_path)
		, fmt_number
		, ext_number
	);
	return fmt_number;
}
void writers::check(const std::string& file_path) {
	ext_to_number_(
		&functon_.at(0)
		, this->module_count_
		, igs::path::ext_from_path(file_path)
		, this->current_num_
		, this->current_ext_num_
	);
}
const bool writers::bad_properties(
	std::ostringstream& error_or_warning_msg
	, const igs::image::properties& props
) {
	if (this->current_num_ < 0) { return true; }

	if (static_cast<igs::image::writer *>(
		functon_.at(this->current_num_)
	)->error_from_properties(
		props,this->current_ext_num_,error_or_warning_msg
	)) {
		return true;
	}

	return false;
}
void writers::bad_properties(const igs::image::properties& props) {
	std::ostringstream error_or_warning_msg;
	if (this->bad_properties(error_or_warning_msg,props)) {
		throw std::domain_error(error_or_warning_msg.str());
	}
}

void writers::put_data(
	const igs::image::properties& props
	, const unsigned int image_size
	, const unsigned char *image_array
	, const int compression_number
	, const std::string& history_str
	, const std::string& file_path
) {
	if (this->current_num_ < 0) { return; }

	static_cast<igs::image::writer *>(
		functon_.at(this->current_num_)
	)->put_data(
		props, image_size, image_array, compression_number
		, history_str, file_path
	);
}
