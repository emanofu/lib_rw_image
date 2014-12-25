#include <stdexcept> /* std::domain_error */
#include <cassert> /* assert(-) */
#include <vector>
#include <fstream> /* std::ifstream */
// #include <iostream> // std::cout
#include "igs_resource_irw.h"
#include "igs_path_irw.h"
#include "igs_image_irw.h"
#include "igs_image_readers.h"

//--------------------------------------------------------------------
using igs::image::readers;

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
readers::readers(const std::string& dir_path):
	valid_ext_ptr_(0)
	, valid_reader_num_(-1)
	, current_num_(-1)
	, module_count_(0)
{
	if (0 < library_.size()) { // already setuped plugin
		this->module_count_ = library_.size();
		return;
	}

	std::set<std::string> file_list;
	igs::resource::dir_list(dir_path.c_str(),file_list); 

	if (file_list.size() <= 0) { // Not exist plugin files
		return;
	}

	library_.resize(file_list.size());
	functon_.resize(file_list.size());

	std::string str;
	for (std::set<std::string>::iterator it = file_list.begin();
	 file_list.end() != it; ++it, ++this->module_count_
	) {
		if ('f' != it->at(0)) { continue; } // Not file
		str = dir_path; str += '/'; str += &(it->at(1));
		igs::resource::plugins_add(str.c_str(),library_,functon_);
	}
}
void readers::clear(void) {
	this->valid_ext_ptr_ = 0;
	this->valid_reader_num_ = -1;
	this->current_num_ = -1;

	igs::resource::plugins_close(library_);
	this->module_count_ = 0;

	functon_.clear();
	library_.clear();
}
readers::~readers()
{ try { this->clear(); } catch(...) {} /* destractorの例外を無効化 */ }
#else //---------------------------------------
# include "igs_pic_reader.h"
# include "igs_sgi_reader.h"
# include "igs_tga_reader.h"
# include "igs_tif_reader.h"
namespace {
	igs::pic::reader cl_rdr_pic;
	igs::sgi::reader cl_rdr_sgi;
	igs::tga::reader cl_rdr_tga;
	igs::tif::reader cl_rdr_tif;
}
readers::readers():
	valid_ext_ptr_(0)
	, valid_reader_num_(-1)
	, current_num_(-1)
	, module_count_(0)
{
	functon_.resize(4);
	functon_.at(this->module_count_++) = &cl_rdr_pic;
	functon_.at(this->module_count_++) = &cl_rdr_sgi;
	functon_.at(this->module_count_++) = &cl_rdr_tga;
	functon_.at(this->module_count_++) = &cl_rdr_tif;
}
void readers::clear(void) {
	this->valid_ext_ptr_ = 0;
	this->valid_reader_num_ = -1;
	this->current_num_ = -1;
	this->module_count_ = 0;
	functon_.clear();
}
readers::~readers()
{ try { this->clear(); } catch(...) {} /* destractorの例外を無効化 */ }
#endif //--------------------------------------
//--------------------------------------------------------------------
/* 全書式有効にする */
void readers::valid(void) { // default
	this->valid_reader_num_ = -1;
	this->valid_ext_ptr_ = 0;
}
/* 指定書式のみ有効にする */
void readers::valid(const int fmt_number) {
	this->valid_reader_num_ = fmt_number;
	this->valid_ext_ptr_ = 0;
}
/* 指定書式&拡張子のみ有効にする */
void readers::valid(const int fmt_number, const int ext_number) {
	this->valid(fmt_number);
	if (this->valid_reader_num_ < 0) { return; }

	this->valid_ext_ptr_ = static_cast<igs::image::reader *>(
		functon_.at(this->valid_reader_num_)
	)->ext_lists()[ext_number];
}

/* 拡張子で有効判別 */
const bool readers::valid(const std::string& file_path) {
	const std::string ext = igs::path::ext_from_path(file_path);
	if (ext.size() <= 0) { return false; }

	if (0 <= this->valid_reader_num_) {
	 if (0 != this->valid_ext_ptr_) {
		if (this->valid_ext_ptr_ == ext) { return true; }
	 } else {
		igs::image::reader *ptr =
			static_cast<igs::image::reader *>(
				functon_.at(this->valid_reader_num_)
			);
		for (unsigned int ii = 0; ii < ptr->ext_count(); ++ii) {
			if (ptr->ext_lists()[ii] == ext) {return true;}
		}
	 }
	} else {
	 for (int ii = 0; ii < this->module_count_; ++ii) {
		igs::image::reader *rptr=static_cast<
			igs::image::reader *>(functon_.at(ii));
		for (unsigned int ii=0; ii < rptr->ext_count(); ++ii) {
		 if (rptr->ext_lists()[ii] == ext) { return true; }
		}
	 }
	}
	return false;
}
/* 書式数(validのみ) */
const unsigned int readers::size(void) {
	if (0 <= this->valid_reader_num_) { return 1; }
	return this->module_count_;
}
/* 拡張子数(validのみ) */
const unsigned int readers::size(const int fmt_number) {
	if (	(0 <= this->valid_reader_num_) &&
		(fmt_number != this->valid_reader_num_)
	) { return 0; }

	return static_cast<igs::image::reader *>(
		functon_.at(fmt_number)
	)->ext_count();
}

/* 書式名(validのみ) */
const char *readers::name(const int fmt_number) {
	if (	(0 <= this->valid_reader_num_) &&
		(fmt_number != this->valid_reader_num_)
	) { return 0; }

	return static_cast<igs::image::reader *>(
		functon_.at(fmt_number)
	)->name();
}
/* 拡張子名(validのみ) */
const char *readers::name(const int fmt_number, const int ext_number) {
	if (	(0 <= this->valid_reader_num_) &&
		(fmt_number != this->valid_reader_num_)
	) { return 0; }

	if (this->valid_ext_ptr_ && (static_cast<igs::image::reader *>(
		functon_.at(fmt_number)
	)->ext_lists()[ext_number] != this->valid_ext_ptr_)) {
		return 0;
	}

	return static_cast<igs::image::reader *>(
		functon_.at(fmt_number)
	)->ext_lists()[ext_number];
}

//--------------------------------------------------------------------
/* 画像データ識別と書式の設定 */
/* 画像ファイルヘッダを読んで識別(拡張子は無視) */
void readers::check(const std::string& file_path) {
	/* ファイルのヘッダータグ読み込み */
	std::ifstream ifs(file_path.c_str(),std::ios::binary);
	std::string msg("Can't ");
	if (!ifs) {
		msg += "open "; msg += file_path;
		throw std::domain_error(msg);
	}
	unsigned char tag_array[4];
	ifs.read((char *)tag_array,sizeof(tag_array));
	if (!ifs) {
		msg += "read "; msg += file_path;
		throw std::domain_error(msg);
	}
	ifs.close();

	/* ヘッダータグから画像読み込みモジュールを得る */
	this->current_num_ = -1;
	for (int ii = 0; ii < this->module_count_; ++ii) {
		igs::image::reader *ptr=static_cast<
			igs::image::reader *>(functon_.at(ii));
		if (ptr->istag(4,tag_array)) {
			this->current_num_ = ii;
			return;
		}
	}

	msg += "take "; msg += file_path;
	throw std::domain_error(msg);
}
void readers::get_info(const std::string& file_path, std::string& info)
{
	if (this->current_num_ < 0) { return; }

	igs::resource::file_stat fsta(file_path.c_str());
	if (!fsta.is_exist) { return; }

	std::ostringstream os;
	fsta.form_stat(os);
	info += os.str();

	igs::image::reader *ptr = static_cast<igs::image::reader *>(
		functon_.at(this->current_num_)
	);
	ptr->get_info( file_path, info );
}
void readers::get_line_head(std::string& line_head) {
	if (this->current_num_ < 0) { return; }
	igs::image::reader *ptr = static_cast<igs::image::reader *>(
		functon_.at(this->current_num_)
	);
	ptr->get_line_head( line_head );
}
void readers::get_line_info(const std::string& file_path, std::string& line_info) {
	if (this->current_num_ < 0) { return; }
	igs::image::reader *ptr = static_cast<igs::image::reader *>(
		functon_.at(this->current_num_)
	);
	ptr->get_line_info( file_path, line_info );
}
void readers::get_size(
	const std::string& file_path,
	unsigned int& image_size,
	unsigned int& history_size
) {
	if (this->current_num_ < 0) { return; }
	igs::image::reader *ptr = static_cast<igs::image::reader *>(
		functon_.at(this->current_num_)
	);
	size_t imagesize=0;
	size_t historysize=0;
	ptr->get_size(file_path,imagesize,historysize);
	image_size = imagesize;
	history_size = historysize;
}
void readers::get_data(
	const std::string& file_path
	, igs::image::properties &props
	, const unsigned int image_size
	, unsigned char *image_array
	, std::string& history_str
	, bool&break_sw /* Job中断制御 */
	, const int segment_bytes /* Job中断間隔 */
) {
	if (this->current_num_ < 0) { return; }
	igs::image::reader *ptr = static_cast<igs::image::reader *>(
		functon_.at(this->current_num_)
	);
	ptr->get_data(
		file_path
		,props
		,image_size
		,image_array
		,history_str
		,break_sw
		,segment_bytes
	);
}
