#include "igs_resource_msg_from_err.h"
#if defined DEBUG_OFBINARY
# if defined _WIN32
#  include "igs_resource_msg_from_err_win.cxx"
# else
#  include "igs_resource_msg_from_err_unix.cxx"
# endif
#endif  /* !DEBUG_OFBINARY */

#if defined _WIN32
# include "igs_resource_ofbinary_win.cxx"
#else
# include "igs_resource_ofbinary_unix.cxx"
#endif

#include "igs_resource_ofbinary.h"

#if defined _WIN32 //-------------------------------------------------------
using igs::resource::ofbinary;
ofbinary::ofbinary(LPCTSTR file_path): handle_(INVALID_HANDLE_VALUE) {
	ofbinary_open_(file_path,this->handle_);
}
void ofbinary::seek(const long offset) {
	LONG file_seek_low = static_cast<LONG>(offset);
	LONG file_seek_high=0;
	if (INVALID_HANDLE_VALUE != this->handle_) {
		ofbinary_seek_(
			0,this->handle_,file_seek_low,file_seek_high
		);
	}
}
void ofbinary::write(const void *buffer,const size_t bytes) {
	if (INVALID_HANDLE_VALUE != this->handle_) {
		ofbinary_write_(
			0,this->handle_,buffer,static_cast<DWORD>(bytes)
		);
	}
}
void ofbinary::close(void)  {
	if (INVALID_HANDLE_VALUE != this->handle_) {
		ofbinary_close_(0,this->handle_);
		this->handle_ = INVALID_HANDLE_VALUE;
	}
}
ofbinary::~ofbinary() {/* このdestractorにまかせずをclose()を実行すること */
	try { this->close(); } catch(...) {}
}
#else //--------------------------------------------------------------------
using igs::resource::ofbinary;
ofbinary::ofbinary(const char *file_path): fd_(-1) {
	ofbinary_open_(file_path,this->fd_);
}
void ofbinary::seek(const long offset) {
	if (-1 != this->fd_) {
		ofbinary_seek_(0,this->fd_,static_cast<off_t>(offset));
	}
}
void ofbinary::write(const void *buffer,const size_t bytes) {
	if (-1 != this->fd_) { ofbinary_write_(0,this->fd_,buffer,bytes); }
}
void ofbinary::close(void)  {
	if (-1 != this->fd_) { ofbinary_close_(0,this->fd_); this->fd_=-1; }
}
ofbinary::~ofbinary() {/* このdestractorにまかせずをclose()を実行すること */
	try { this->close(); } catch(...) {}
}
#endif //-------------------------------------------------------------------

#if defined DEBUG_OFBINARY
#include <iostream>
#include <vector>
int main( int argc, char *argv[] )
{
	if (3 != argc) {
		std::cout
		<< "Usage : " << argv[0] << " file size[2000000000]\n";
		return 1;
	}
 try {
	const size_t bytes = atoi(argv[2]);
 std::cout << "memory_alloc=" << bytes << std::endl;
	std::vector<unsigned char> buffer(bytes);
 std::cout << "memory_init=" << bytes << std::endl;
	for (size_t ii=0;ii<(bytes-1);ii+=2) {
		buffer[ii]   = 'i';
		buffer[ii+1] = '\n';
	}
	buffer[bytes-1] = '\n';
 std::cout << "ofbinary open by constructor()\n";
	igs::resource::ofbinary ofb(argv[1]);
 std::cout << "ofbinary.seek(-)\n";
	ofb.seek(static_cast<long>(0));
 std::cout << "ofbinary.write()\n";
	ofb.write(&buffer.at(0),bytes);
 std::cout << "ofbinary.close(-)\n";
	ofb.close();
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_OFBINARY */
/*
rem ------ WindowsXp sp3(32bits) vc2008sp1  :108,114 w! tes_w32_ofbinary.bat
set SRC=igs_resource_ofbinary
set TGT=tes_w32_ofbinary
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_OFBINARY /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem ------ Windows7 (64bits) vc2008sp1  :115,121 w! tes_w64_ofbinary.bat
set SRC=igs_resource_ofbinary
set TGT=tes_w64_ofbinary
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_OFBINARY /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# ------ rhel4 (32bits)  :122,123 w! tes_u32_ofbinary.csh
g++ -Wall -O2 -g -DDEBUG_OFBINARY -I../../l01log/src igs_resource_ofbinary.cxx -o tes_u32_ofbinary
*/

