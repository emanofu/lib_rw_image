#include <unistd.h>	// ::read(-) ,::close(-)
#include <fcntl.h>	// ::open(-),O_RDONLY,O_DIRECT
#include <errno.h>	// errno
#include <sys/types.h>	// ::stat()
#include <sys/stat.h>	// ::stat()
#include <unistd.h>	// ::stat()
#include <stdexcept>	// std::domain_error

#include "igs_resource_msg_from_err.h"
#if defined DEBUG_IFBINARY_UNIX //----------------------------------
# include "igs_resource_msg_from_err_unix.cxx"
#endif  /* !DEBUG_IFBINARY_UNIX */ //-------------------------------

namespace {
 void ifbinary_open_(const char *file_path, int&fd) {
	fd = ::open( file_path, O_RDONLY
#if defined __GNUC_RH_RELEASE__
//	| O_DIRECT
	/* RedHatlinux4  ">man 2 open"より
	O_DIRECT: ファイルのI/Oはユーザー空間バッファに対して直接行われる
	バグ	「O_DIRECT でいつも困るのは、インタフェース全部が本当にお
		馬鹿な点だたぶん危ないマインドコントロール剤で頭がおかしく
		なったサルが設計したんじゃないかな」 -- Linus
	*/
	/* これを使うとおそくなるのでつかわないこと(2011-03-02) */
#endif
	);
	if (-1 == fd) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
 }
 void ifbinary_size_(
	const char *file_path
	,off_t&file_size
	// off_t = __off_t = __OFF_T_TYPE = __SLONGWORD_TYPE = long int
 ) {
	struct stat t_stat;
	if (-1 == ::stat( file_path, &t_stat)) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
	file_size = t_stat.st_size;
 }
 void ifbinary_read_(
	const char *file_path
	, const int fd
	, void *buffer
	, const size_t bytes
 ) {
	const ssize_t byt = ::read( fd, buffer, bytes );
	if (-1 == byt) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
	if (bytes != static_cast<size_t>(byt)) {
	/* read(-)のエラーではないがifbinary_read_(-)としてはエラーとする */
		throw std::domain_error("Read different size");
	}
 }
 void ifbinary_close_(
	const char *file_path
	,int fd
 ) {
	if (-1 == ::close(fd)) {
		throw std::domain_error(igs::resource::msg_from_err(
			errno,file_path
		));
	}
 }
}

#if defined DEBUG_IFBINARY_UNIX //----------------------------------
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
 std::cout << "ifbinary_size_(-)\n";
	off_t file_size=0;
	ifbinary_size_(argv[1],file_size);
 std::cout << " file_size=" << file_size << std::endl;
 std::cout << "ifbinary_open_(-)\n";
	int fd=0;
	ifbinary_open_(argv[1],fd);
 std::cout << "memory_alloc\n";
	std::vector<unsigned char> buffer(file_size);
 std::cout << "ifbinary_read_(-)\n";
	ifbinary_read_(argv[1],fd	,static_cast<void *>(&buffer.at(0))
					,static_cast<size_t>(file_size));
 std::cout << "ifbinary_close_(-)\n";
	ifbinary_close_(argv[1],fd);
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_IFBINARY_UNIX */ //-------------------------------
/*
# ------ rhel4 (32bits)  :113,114 w! tes_u32_ifbinary_unix.csh
g++ -Wall -O2 -g -DDEBUG_IFBINARY_UNIX -I../../l01log/src igs_resource_ifbinary_unix.cxx -o tes_u32_ifbinary_unix
*/

