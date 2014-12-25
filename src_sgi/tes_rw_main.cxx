#include <vector> /* std::vector */
#include <fstream> /* std::ifstream */
#include <iostream> /* std::cout std::cerr */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_image_irw.h"

#if defined USE_IRW_PLUGIN
#include <vector>
#endif
//------------------------------------------------------------
#if defined USE_IMAGE_RW_PLUGIN
class plugins {
public:
	void add(const std::string& file_path);
	~plugins();
	std::vector<void *> symbols;
private:
	std::vector<void *> libraries_;
};
void plugins::add(const std::string& file_path) {
	try {
		void *library_ptr = 0;
		void *param_ptr = 0;
		igs::resource::plugin_open(file_path,
			&library_ptr,&param_ptr);
		this->libraries_.push_back(library_ptr);
		this->symbols.push_back(param_ptr);
        }
	catch(...) { /* bad_allocしかこないハズだが?... */
		/* エラーメッセージは?... */
		//exit(EXIT_FAILURE); /* ...この場で終了でいいのか?... */
		throw;
	}
}
plugins::~plugins() {
	try {
		for (std::vector<void *>::reverse_iterator it =
		      this->libraries_.rbegin();
		it != this->libraries_.rend(); ++it) {
			igs::resource::plugin_close(*it);
		}
	}
	catch(...) {} /* destractorの例外を無効化する */
}
#else /*-------------------------------------static link---*/
# include "igs_sgi_reader.h"
# include "igs_sgi_writer.h"
#endif
//------------------------------------------------------------
namespace {
 const std::string ext_from_path_( const std::string& file_path ) {
	std::string::size_type index = file_path.rfind('.');
	if (index != std::string::npos) {
		return file_path.substr(++index);
	}
	return "";
 }
}
//------------------------------------------------------------
int main(int argc, char *argv[])
{
	if (3 != argc) {
		return 1;
	}

#if defined USE_IMAGE_RW_PLUGIN /*----------dynamic link---*/
# if defined _WIN32 || defined __HP_aCC // vc or b11_23u compile_type
plugins cl_dl; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# endif
#endif

 try {
#if defined USE_IMAGE_RW_PLUGIN /*----------dynamic link---*/
	// const char *dirpath = "../../../utility_tools/";
	std::string dirpath("C:/users/public");//home
	dirpath += "/utility_tools";
	const char *syst_dir =
# if   defined __GNUC_RH_RELEASE__ // rhel4 compile_type
		"rhel4"
# elif defined _WIN32 & (1400 == _MSC_VER) & defined _DLL// compile_type
		"vc2005md"
# elif defined _WIN32 & (1200 == _MSC_VER) & defined _DLL// compile_type
		"vc6md"
# elif defined _WIN32 & (1200 == _MSC_VER) & !defined _DLL// compile_type
		"vc6mt"
# elif defined __APPLE_CC__ // darwin8 compile_type
		"darwin8"
# elif defined __HP_aCC // b11_23u compile_type
		"b11_23u"
# endif
		;
	const char *openpath = "/plugin/image/open/sgi.";
	const char *savepath = "/plugin/image/save/sgi.";
	const char *ext =
# if defined _WIN32 && defined _DLL // vc md compile_type
		"dll"
# else
		"so"
# endif
		;
	std::ostringstream path_open;
	path_open << dirpath << syst_dir << openpath << ext;

	std::ostringstream path_save;
	path_save << dirpath << syst_dir << savepath << ext;

# if !defined _WIN32 && !defined __HP_aCC // Not vc&b11_23u compile_type
	plugins cl_dl; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# endif

	cl_dl.add(path_open.str());
	cl_dl.add(path_save.str());

	igs::image::reader *
	p_rdr = static_cast<igs::image::reader *>(cl_dl.symbols[0]);
	igs::image::writer *
	p_wtr = static_cast<igs::image::writer *>(cl_dl.symbols[1]);
#else /*--------------------------------------static link---*/
	igs::sgi::reader sgi_re;
	igs::image::reader *p_rdr = &sgi_re;
	igs::sgi::writer sgi_wr;
	igs::image::writer *p_wtr = &sgi_wr;
#endif

	igs::resource::stop_watch cl_stopw;
	bool
		b_tv_sw = true,
		b_v_sw = true,
		b_pv_sw = true;//false;

	/* ファイルパス */
if (b_v_sw) {
	std::cout << "open " << argv[1] << std::endl;
}

	/* 拡張子確認 */
if (b_pv_sw) {
	bool ext_sw = false;
	for (unsigned int ii=0;ii<p_rdr->ext_count();++ii) {
		if (ext_from_path_(argv[1]) ==
		p_rdr->ext_lists()[ii]) { ext_sw = true; break; }
	}
	std::cout
		<< "open extension is "
		<< std::boolalpha
		<< ext_sw
		<< std::endl;
}

#if 0
	/* ファイルヘッダタグ確認 */
	std::ifstream ifs(argv[1],std::ios::binary);
	if (!ifs) {
		std::string str;
		str = "Can not open ";
		str += argv[1];
		throw std::domain_error(str);
	}
	unsigned char ca2[2];
	ifs.read((char *)ca2,sizeof(ca2));
	if (ifs.bad()) {
		std::string str;
		str = "Read error of ";
		str += argv[1];
		throw std::domain_error(str);
	}
	ifs.close();
if (b_pv_sw) {
	std::cout
		<< "open file tag is "
		<< std::boolalpha << p_rdr->istag(2,ca2)
		<< "\""
		<< (int)ca2[0]
		<< ","
		<< (int)ca2[1]
		<< "\""
		<< std::endl;
}
#endif

if (b_pv_sw) {
	std::string str_info;
	p_rdr->get_info(argv[1],str_info);
	std::cout << str_info;
	p_rdr->get_line_head(str_info);
	std::cout << str_info;
	p_rdr->get_line_info(argv[1],str_info);
	std::cout << str_info;
}

	/* 用意すべき画像と作業情報のサイズを得る */
	unsigned image_size=0,history_size=0;
if (b_tv_sw) cl_stopw.start();
	p_rdr->get_size(argv[1],image_size,history_size);
if (b_tv_sw) cl_stopw.stop("time:p_rdr->get_size(-)");
if (b_v_sw) {
	std::cout
		<< "open image_size " << image_size << "bytes\n"
		<< "open history_size " << history_size << "bytes"
		<< std::endl;
}

	/* メモリの確保 */
if (b_tv_sw) cl_stopw.start();
	std::vector<unsigned char> cl_image;
	if (0 < image_size) { cl_image.resize(image_size); }
	std::string history_str;
	if (0 < history_size) {history_str.resize(history_size+1);}
if (b_tv_sw) cl_stopw.stop("time:new []");

	/* 読む */
if (b_tv_sw) cl_stopw.start();
	igs::image::properties prop;
	bool break_sw = false;
	p_rdr->get_data(
		argv[1]
		, prop
		, cl_image.size()
		, &cl_image.at(0)
		, history_str
		, break_sw
	);
if (b_tv_sw) cl_stopw.stop("time:p_rdr->get_data()");

	/* ヘッダ情報 */
if (b_v_sw) {
	std::cout
		<< "open image_properties "
		<< prop.width
		<< "x" << prop.height
		<< "," << prop.channels << "ch"
		<< "," << prop.bits << "bits"
		;
}
	using namespace igs::image;
	char *cp_tmp=0;
	switch (prop.orie_side) {
	case orientation::not_defined: cp_tmp = "nd"; break;
	case orientation::topleft: cp_tmp = "tl"; break;
	case orientation::toprigh: cp_tmp = "tr"; break;
	case orientation::botrigh: cp_tmp = "br"; break;
	case orientation::botleft: cp_tmp = "bl"; break;
	case orientation::lefttop: cp_tmp = "lt"; break;
	case orientation::rightop: cp_tmp = "rt"; break;
	case orientation::righbot: cp_tmp = "rb"; break;
	case orientation::leftbot: cp_tmp = "lb"; break;
	}
if (b_v_sw) {
	std::cout
		<< "," << cp_tmp;
}

	switch (prop.reso_unit) {
	case resolution::not_defined: cp_tmp = "nd"; break;
	case resolution::nothing:     cp_tmp = "no"; break;
	case resolution::meter:       cp_tmp = "me"; break;
	case resolution::centimeter:  cp_tmp = "ce"; break;
	case resolution::millimeter:  cp_tmp = "mi"; break;
	case resolution::inch:        cp_tmp = "in"; break;
	}
if (b_v_sw) {
	std::cout
		<< "," << cp_tmp
		<< " " << prop.reso_x
		<< "x" << prop.reso_y
		<< std::endl;
}
if (b_pv_sw) {
	std::cout
		<< "open history_str is\n"
		<< history_str;
}

//------------------------------------------------------------

	/* ファイルパス */
	std::string str_wr_name = argv[2];

	/* 書式名 */
if (b_pv_sw) {
	std::cout << "save plugin is \"" << p_wtr->name() << "\"\n";
}
	/* 拡張子数 */
	/* 拡張子名リスト */
	/* 拡張子確認 */
if (b_pv_sw) {
	std::cout << "save extension is ";
	for (unsigned int ii=0;ii<p_wtr->ext_count();++ii) {
		if (0 < ii) { std::cout << ','; }
		std::cout << p_wtr->ext_lists()[ii];
	}
	std::cout << '\n';
}

	/* raw画像保存 */
/******	igs::path::change_ext( str_wr_name, "raw" );
 	std::ofstream ofs(str_wr_name.c_str(), std::ios::binary);
	ofs.write(
		igs::resource::pointer_cast<char *>(&cl_image.at(0)),
		cl_image.size()
	);
	ofs.close();
exit(0);******/

	/* 拡張子追加あるいは変更(リストと違う場合のみ) */
	/**igs::path::change_ext(
		p_wtr->ext_lists()[p_wtr->ext_count()-1],
		str_wr_name
	);**/

	/* 保存可能かチェック */
	std::ostringstream error_and_warning_msg;
	std::string exten = ext_from_path_(str_wr_name);
	unsigned int ext_number=0;
	for (;ext_number<p_wtr->ext_count();++ext_number) {
		if (p_wtr->ext_lists()[ext_number] == exten) {
			p_wtr->error_from_properties(
				prop,ext_number,error_and_warning_msg );
			break;
		}
	}
	if (p_wtr->ext_count() <= ext_number) {
		std::string str;
		str = "not enable tif extension(";
		str += exten;
		str += ")";
		throw std::domain_error(str);
	}

	/* 作業情報追加 */
	std::string::size_type sindex=history_str.rfind('*');
	std::string::size_type eindex=history_str.find('\n',sindex);
	if(std::string::npos!=sindex){
	 if(std::string::npos!=eindex){
		history_str.erase(sindex,eindex-sindex+1);
	 } else {
		history_str.erase(sindex);
	 }
	}
	if ((0 < history_str.size()) &&
	('\n' != history_str.at(history_str.size()-1))) {
		history_str += "\n";
	}
	history_str +=
" username	2008-05-20 12:00:00	applicationname	hostname\n";

if (b_v_sw) {
	std::cout << "save " << str_wr_name << std::endl;
}
	/* 保存 */
if (b_tv_sw) cl_stopw.start();
	p_wtr->put_data(
		prop
		, cl_image.size()
		, &cl_image.at(0)
		, 0
		, history_str
		, str_wr_name
	);
if (b_tv_sw) cl_stopw.stop("time:p_wtr->put_data()");
 }
 catch (std::bad_alloc& e) {
	std::cerr
			<< "std::bad_alloc <" << e.what() << ">"
			<< std::endl;
 }
 catch (std::exception& e) {
	std::cerr
			<< "exception <" << e.what() << ">"
			<< std::endl;
 }
 catch (...) {
	std::cerr
			<< "other exception"
			<< std::endl;
 }
	return 0;
}
