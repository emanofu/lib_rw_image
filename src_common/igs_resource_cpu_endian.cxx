#include "igs_resource_cpu_endian.h"

const bool igs::resource::cpu_is_little_endian(void)
{
	union { unsigned short us; unsigned char uc[2]; } buf;
	buf.us = 1;
	/*
		CPU Byte Orderの説明
		Big Endian CPUではbuf.usに1いれるとbuf.uc[1]が1となる
				+---------------+
		unsigned short	|      us       |
				|           1   |
				+-------+-------+
		unsigned char	| uc[0] | uc[1] |
				|   0   |   1   |
				+-------+-------+
		Little Endian CPUではbuf.usに1いれるとbuf.uc[0]が1となる
				+---------------+
		unsigned short	|      us       |
				|   1           |
				+-------+-------+
		unsigned char	| uc[0] | uc[1] |
				|   1   |   0   |
				+-------+-------+
	*/
	return 1 == buf.uc[0]; /* true by Little Endian CPU */
}
#if defined DEBUG_CPU_ENDIAN
#include <iostream>
int main(int argc,char *argv[])
{
	if (1 != argc) {
		std::cout << "Usage : " << argv[0] << std::endl;
		return 1;
	}
 try {
	if (true==igs::resource::cpu_is_little_endian()) {
		std::cout << "cpu is little endian" << std::endl;
	} else {
		std::cout << "cpu is big endian" << std::endl;
	}
 }
 catch (std::exception& e) {
	std::cerr << "exception <" << e.what() << ">" << std::endl;
 }
 catch (...) {
	std::cerr << "other exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_CPU_ENDIAN */
/*
rem --- WindowsXp sp3(32bits) vc2008sp1  :52,58 w! tes_w32_cpu_endian.bat
set SRC=igs_resource_cpu_endian
set TGT=tes_w32_cpu_endian
set INC=.
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_CPU_ENDIAN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem --- Windows7 (64bits) vc2008sp1  :59,65 w! tes_w64_cpu_endian.bat
set SRC=igs_resource_cpu_endian
set TGT=tes_w64_cpu_endian
set INC=.
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_CPU_ENDIAN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# --- rhel4 (32bits)  :66,67 w! tes_u32_cpu_endian.csh
g++ -Wall -O2 -g -DDEBUG_CPU_ENDIAN -I. igs_resource_cpu_endian.cxx -o tes_u32_cpu_endian
*/
