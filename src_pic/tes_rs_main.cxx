#include <fstream> /* std::ifstream */
#include <iostream> /* std::cout std::cerr */
#include <sstream> /* std::ostringstream */
#include <iomanip> /* std::setw(-) */
#include <stdexcept> /* std::domain_error(-) */
#include <vector>
#include <string>
#include "igs_resource_irw.h"
#include "igs_image_irw.h"
#include "igs_pic_reader.h"
#include "tes_rs.h"

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
		exit(EXIT_FAILURE); /* ...この場で終了でいいのか?... */
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
# include "igs_pic_reader.h"
# include "igs_pic_writer.h"
#endif
//------------------------------------------------------------
namespace {
	igs::resource::stop_watch cl_stopw;
	std::vector<double> timev;
}
void igs::test::speed::clear(void){if(0<timev.size()){timev.clear();}}
void igs::test::speed::start(void){cl_stopw.start(); }
void igs::test::speed::stop(void) {timev.push_back(cl_stopw.stop());}
//------------------------------------------------------------
namespace {
 void set_path( // -----------------
	const std::string& head,
	const std::string& ext,
	const int num,
	std::string& path
 ) {
	std::ostringstream os;
	os << std::setw(4) << std::setfill('0') << num;
	path = head;
	path += os.str();
	path += ext;
 }
 void exec01check_file_tag( // -----------------
	const std::string path,
	igs::image::reader *p_rdr
 ) {
#if defined TES_SPEED
	igs::test::speed::start();
#endif
	std::ifstream ifs(path.c_str(),std::ios::binary);
	if (!ifs) {
		std::cout << "\"" << path << "\" can not open";
		throw std::domain_error("file open error");
	}
	unsigned char ca4[4]; 
	ifs.read(sizeof(ca4),(char *)ca4);
	if (ifs.bad()) { throw std::domain_error("file read error"); }
	ifs.close();
	if (false == p_rdr->istag(4,ca4)) { throw std::domain_error("bad tag"); }
#if defined TES_SPEED
	igs::test::speed::stop(); // 1
#endif
 }
 void exec02check_file_size( // ----------------
	const std::string path,
	igs::image::reader *p_rdr,
	unsigned int& image_size,
	unsigned int& history_size
 ) {
#if defined TES_SPEED
	igs::test::speed::start();
#endif
	/* 用意すべき画像と作業情報のサイズを得る */
	p_rdr->get_size(path,image_size,history_size);
#if defined TES_SPEED
	igs::test::speed::stop(); // 2
#endif
 }
 void exec03memory_array( // -------------------
	unsigned int image_size,
	unsigned int history_size,
	std::vector<unsigned char>& cl_image,
	std::string& history_str
 ) {
#if defined TES_SPEED
	igs::test::speed::start();
#endif
	/* メモリの確保 */
	if (0 < image_size) { cl_image.resize(image_size); }
	if (0 < history_size) {history_str.resize(history_size+1);}
#if defined TES_SPEED
	igs::test::speed::stop(); // 3
#endif

 }
 void print_column_val(std::vector<double>&timev) {
	std::cout.setf( std::ios::fixed );
	for (std::vector<double>::iterator
	it  = timev.begin();
	it != timev.end(); ++it) {
		if (it != timev.begin()) { std::cout << "\t"; }
		std::cout
#if defined _WIN32 // vc compile_type
			<< std::setprecision(5)
#else
			<< std::setprecision(3)
#endif
			<< (*it);
	}
	std::cout << "\n";
 }
//------------------------------------------------------------
 void execute(const std::string& head, const std::string& ext,const int i_startf,const int i_endf) {
	/* 各ファイルパス用変数 */
	std::string path;

	/* 始めのファイルへのパス */
 	set_path(head, ext, i_startf, path);

	/* スピード計測対照(画像読み込みmethod)のinstance化 */
#if defined USE_IMAGE_RW_PLUGIN /*----------dynamic link---*/
	// const char *dirpath = "../../../utility_tools/";
	std::string dirpath("C:/users/public"); // home
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
	const char *openpath = "/plugin/image/open/pic.";
	const char *ld_ext =
# if defined _WIN32 && defined _DLL // vc md compile_type
		"dll"
# else
		"so"
# endif
		;
	std::ostringstream path_open;
	path_open << dirpath << syst_dir << openpath << ld_ext;

	plugins cl_dl;
	cl_dl.add(path_open.str());
	igs::image::reader *
	p_rdr = static_cast<igs::image::reader *>(cl_dl.symbols[0]);
#else /*--------------------------------------static link---*/
	igs::pic::reader pic_re;
	igs::image::reader *p_rdr = &pic_re;
#endif

	/* 計測する項目 */
	std::cout << "1.tag\t2.size\t3.mem\n";

	/* スピード記録のクリア */
#if defined TES_SPEED
	igs::test::speed::clear();
#endif
	/* 始めのファイルのみtagとsizeのチェックと時間計測 */
	exec01check_file_tag(path,p_rdr);
	unsigned int image_size=0,history_size=0;
	exec02check_file_size(path,p_rdr,image_size,history_size);

	/* メモリ確保と時間計測 */
	std::vector<unsigned char> cl_image;
	std::string history_str;
	exec03memory_array(image_size * (i_endf-i_startf+1),
		/* history_size*/0, cl_image, history_str);

	/* 結果表示 */
	print_column_val(timev);

	/* 計測する項目 */
	std::cout << "1.path\t2.fmem\t3.read\t4.dhead\t5.ddata\t6.total"
		  << "\n";

	/* 1frameにかかる全時間をはかる時計の準備 */
	igs::resource::stop_watch cl_stopw_set_path;
	igs::resource::stop_watch cl_stopw_1frame;

	igs::image::properties st_properties;
	bool break_sw = false;
	for (int ii = i_startf; ii <= i_endf; ++ii) {
		/* スピード記録のクリア */
#if defined TES_SPEED
		igs::test::speed::clear();
#endif
		/* 1frameにかかる全時間をはかる時計の準備 */
		cl_stopw_1frame.start();

#if defined TES_SPEED
		cl_stopw_set_path.start();
#endif
 		set_path(head, ext, ii, path);
#if defined TES_SPEED
		timev.push_back(cl_stopw_set_path.stop()); // 1
#endif

		/* 読む */	// 2,3,4,5
		p_rdr->get_data(path,st_properties,
			cl_image.size() - image_size * (ii - i_startf)
			, &cl_image.at(0) + image_size * (ii - i_startf)
			, history_str
			, break_sw
		);

#if defined TES_SPEED
		timev.push_back(cl_stopw_1frame.stop()); // 6
#endif
		print_column_val(timev);
	}
 }
}
//------------------------------------------------------------
int main(int argc,char *argv[]) {
	if (4 != argc) {
		std::cout << "Usage : "
			<< argv[0]
			<< " fullpath startf endf\n";
		return 1;
	}

	/* 全時間をはかる時計の準備 */
	igs::resource::stop_watch cl_stopw_total;
	cl_stopw_total.start();

	/* 拡張子を得る */
	std::string head = argv[1];
	std::string::size_type index = head.rfind('.');
	if (index == std::string::npos) { return 2; }
	std::string ext = head.substr(index); // ".pic"

	/* ファイル名付きのパスを得る */
	head.erase(index); // "/tmp/tmp.0001.pic" --> "/tmp/tmp.0001"
	std::string::size_type index2 = head.rfind('.');
	if (index2 == std::string::npos) { return 3; }
	head.erase(index2+1); // "/tmp/tmp.0001" --> "/tmp/tmp."

	/* 実行 */
	execute(head, ext, atoi(argv[2]), atoi(argv[3]));

	/* 全時間をプリント */
	std::cout
#if defined _WIN32 // vc compile_type
		<< std::setprecision(5)
#else
		<< std::setprecision(3)
#endif
		<< cl_stopw_total.stop()
		<< "(time with this printing time)"
		<< "\n";

	return 0;
}
