#include <unistd.h>	// ::write(-),::lseek(-),::close(-)
#include <fcntl.h>	// ::open(-),O_RDONLY,O_DIRECT
#include <sys/types.h>	// open(-),creat(-)
#include <sys/stat.h>	// open(-),creat(-)
#include <fcntl.h>	// open(-),creat(-)
#include <stdexcept>	// std::domain_error
#include <sstream>	// std::ostringstream
#include <errno.h>	// errno

# include "igs_resource_msg_from_err.h"
#if defined DEBUG_OFBINARY_UNIX //----------------------------------
# include "igs_resource_msg_from_err_unix.cxx"
#endif  /* !DEBUG_OFBINARY_UNIX */ //-------------------------------

namespace {
 void ofbinary_open_(const char *file_path,int&fd) {
	//fd = ::creat( file_path, S_IRWXU | S_IRWXG | S_IRWXO );
	/* linux: "man 2 creat"より
		creat()は flags に O_CREAT|O_WRONLY|O_TRUNC
		を指定して open() を行うのと等価
	*/
	fd = ::open( file_path
		,O_CREAT | O_WRONLY | O_TRUNC
#if defined __GNUC_RH_RELEASE__
//	| O_DIRECT
	/* RedHatlinux4  ">man 2 open"より
	O_DIRECT: ファイルのI/Oはユーザー空間バッファに対して直接行われる
	バグ	「O_DIRECT でいつも困るのは、インタフェース全部が本当にお
		馬鹿な点だたぶん危ないマインドコントロール剤で頭がおかしく
		なったサルが設計したんじゃないかな」 -- Linus
	*/
	/*
	O_DIRECTは大きいサイズでallocerror(?)となるため使わない2011-3-3 */
#endif
		,S_IRWXU | S_IRWXG | S_IRWXO
	);
	if (-1 == fd) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
 }
 void ofbinary_write_(
	const char *file_path
	, const int fd
	, const void *buffer
	, const size_t bytes
 ) {
	const ssize_t byt = ::write(fd, buffer, bytes);
	if ( -1 == byt ) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
	if (bytes != static_cast<size_t>(byt)) {
	/* write(-)のエラーではないが
	ofbinary_write_(-)としてはエラーとする */
		throw std::domain_error("Write different size");
	}
	/***if ( -1 == ::fsync(fd) ) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}***/
 }
 void ofbinary_seek_(
	const char *file_path
	, const int fd
	, const off_t offset
 ) {
	const off_t byt = ::lseek(fd ,offset ,SEEK_SET);
	if ((off_t)-1 == byt) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
 }
 void ofbinary_close_(
	const char *file_path
	, const int fd
 ) {
	if (-1 == ::close(fd)) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
 }
}

#if defined DEBUG_OFBINARY_UNIX //----------------------------------
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
	int fd=0;
 std::cout << "ofbinary_open_(-)\n";
	ofbinary_open_(argv[1],fd);
 std::cout << "ofbinary_seek_(-)\n";
	ofbinary_seek_(argv[1],fd,0);
	const size_t bytes = atoi(argv[2]);
 std::cout << "memory_alloc=" << bytes << "bytes\n";
	std::vector<unsigned char> buffer(bytes);
 std::cout << "memory_init=" << bytes << "bytes\n";
	for (size_t ii=0;ii<(bytes-1);ii+=2) {
		buffer[ii]   = 'i';
		buffer[ii+1] = '\n';
	}
	buffer[bytes-1] = '\n';
 std::cout << "ofbinary_write_(-)\n";
	ofbinary_write_(argv[1],fd
		,static_cast<void *>(&buffer.at(0)),bytes);
 std::cout << "ofbinary_close_(-)\n";
	ofbinary_close_(argv[1],fd);
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_OFBINARY_UNIX */ //-------------------------------
/*
# ------ rhel4 (32bits)  :132,133 w! tes_u32_ofbinary_unix.csh
g++ -Wall -O2 -g -DDEBUG_OFBINARY_UNIX -I../../l01log/src igs_resource_ofbinary_unix.cxx -o tes_u32_ofbinary_unix
*/

