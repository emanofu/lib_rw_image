#include "igs_resource_msg_from_err.h"
#if defined DEBUG_IFBINARY
#if defined _WIN32
# include "igs_resource_msg_from_err_win.cxx"
#else
# include "igs_resource_msg_from_err_unix.cxx"
#endif
#endif  /* !DEBUG_IFBINARY */

#if defined _WIN32
# include "igs_resource_ifbinary_win.cxx"
#else
# include "igs_resource_ifbinary_unix.cxx"
#endif

#include "igs_resource_ifbinary.h"

#if defined _WIN32 //-------------------------------------------------------
/* ------ ファイルをbinaryで読み込む ------ */
using igs::resource::ifbinary;
ifbinary::ifbinary(LPCTSTR file_path): handle_(INVALID_HANDLE_VALUE) {
	ifbinary_open_(file_path,this->handle_);
}
void ifbinary::read(void *buffer,const size_t bytes) {
	if (INVALID_HANDLE_VALUE != this->handle_) {
		ifbinary_read_(
			0,this->handle_,buffer,static_cast<DWORD>(bytes)
		);
	}
}
void ifbinary::close(void)  {
	if (INVALID_HANDLE_VALUE != this->handle_) {
		ifbinary_close_(0,this->handle_);
		this->handle_ = INVALID_HANDLE_VALUE;
	}
}
ifbinary::~ifbinary() {/* このdestractorにまかせずをclose()を実行すること */
	try { this->close(); } catch(...) {}
}
/* ------ ファイルサイズを得る ------ */
/* エラーのときはゼロを返す */
const size_t igs::resource::bytes_of_file(LPCTSTR file_path) {
	HANDLE handle_=0;
	ifbinary_open_(file_path,handle_);
	DWORD file_size_low =0;	/* 4GB以下のときはこの値だけをみればよい */
	DWORD file_size_high=0;	/* 4GB以上の上位ダブルワード */
 try {	ifbinary_size_(file_path,handle_,file_size_low,file_size_high); }
 catch(...) {}
	ifbinary_close_(file_path,handle_);
	return static_cast<size_t>(file_size_low);
}
#else //--------------------------------------------------------------------
/* ------ ファイルをbinaryで読み込む ------ */
using igs::resource::ifbinary;
ifbinary::ifbinary(const char *file_path): fd_(-1) {
	ifbinary_open_(file_path,this->fd_);
}
void ifbinary::read(void *buffer,const size_t bytes) {
	if (-1 != this->fd_) { ifbinary_read_(0,this->fd_,buffer,bytes); }
}
void ifbinary::close(void)  {
	if (-1 != this->fd_) { ifbinary_close_(0,this->fd_); this->fd_=-1; }
}
ifbinary::~ifbinary() {/* このdestractorにまかせずをclose()を実行すること */
	try { this->close(); } catch(...) {}
}
/* ------ ファイルサイズを得る ------ */
/* エラーのときはゼロを返す */
const size_t igs::resource::bytes_of_file(const char *file_path) {
	off_t file_size=0;
	ifbinary_size_(file_path,file_size);
	return static_cast<size_t>(file_size);
}
#endif //-------------------------------------------------------------------

#if defined DEBUG_IFBINARY
#include <iostream>
#include <vector>
int main( int argc, char *argv[] )
{
	if (2 != argc) {
		std::cout
			<< "Usage : " << argv[0] << " file\n";
		return 1;
	}
 try {
 std::cout << "igs::resource::bytes_of_file(-)\n";
 	const size_t bytes = igs::resource::bytes_of_file(argv[1]);
 std::cout << "memory_alloc=" << bytes << std::endl;
	std::vector<unsigned char> buffer(bytes);
	//buffer.resize(bytes);
 std::cout << "ifbinary open by constructor()\n";
	igs::resource::ifbinary ifb(argv[1]);
 std::cout << "ifbinary.read()\n";
	ifb.read(&buffer.at(0),bytes);
 std::cout << "ifbinary.close(-)\n";
	ifb.close();
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_IFBINARY */
/*
rem ------ WindowsXp sp3(32bits) vc2008sp1  :109,115 w! tes_w32_ifbinary.bat
set SRC=igs_resource_ifbinary
set TGT=tes_w32_ifbinary
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_IFBINARY /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem ------ Windows7 (64bits) vc2008sp1  :116,122 w! tes_w64_ifbinary.bat
set SRC=igs_resource_ifbinary
set TGT=tes_w64_ifbinary
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_IFBINARY /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# ------ rhel4 (32bits)  :123,124 w! tes_u32_ifbinary.csh
g++ -Wall -O2 -g -DDEBUG_IFBINARY -I../../l01log/src igs_resource_ifbinary.cxx -o tes_u32_ifbinary
*/

