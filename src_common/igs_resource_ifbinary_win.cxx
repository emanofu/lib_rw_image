#include <windows.h>
#include "igs_resource_msg_from_err.h"
#if defined DEBUG_IFBINARY_WIN //----------------------------------
# include "igs_resource_msg_from_err_win.cxx"
#endif  /* !DEBUG_IFBINARY_WIN */ //-------------------------------
namespace {
/*
汎用データ型  ワイド文字(UNICODE)(※1)	マルチバイト文字(_MBCS)(※2)
TCHAR	      wchar_t			char 
LPTSTR	      wchar_t *			char * 
LPCTSTR	      const wchar_t *		const char * 
※1  1文字を16ビットのワイド文字として表すUnicode を使う方法
	すべての文字が 16 ビットに固定されます。
	マルチバイト文字に比べ、メモリ効率は低下しますが処理速度は向上します
※2  1文字を複数のバイトで表すマルチバイト文字
	MBCS(Multibyte Character Set) と呼ばれる文字集合を使う方法
	可変長だが、事実上、サポートされているのは 2 バイト文字までなので、
	マルチバイト文字の 1 文字は 1 バイトまたは 2 バイトとなります。
	Windows 2000 以降、Windows は内部で Unicode を使用しているため、
	マルチバイト文字を使用すると内部で文字列の変換が発生するため
	オーバーヘッドが発生します。
	UNICODEも_MBCSも未定義のときはこちらになる。
*/
 void ifbinary_open_(
	LPCTSTR file_path
	,HANDLE&handle
 ) {
	handle = ::CreateFile(
		file_path		// LPCTSTR lpFileName
		,GENERIC_READ		//,DWORD dwDesiredAccess
		,FILE_SHARE_READ	//,DWORD dwShareMode
		,NULL	//,LPSECURITY_ATTRIBUTES lpSecurityAttributes
		,OPEN_EXISTING		//,DWORD dwCreationDisposition
		,FILE_ATTRIBUTE_NORMAL	//,DWORD dwFlagsAndAttributes
		,NULL			//,HANDLE hTemplateFile
	);
	if (INVALID_HANDLE_VALUE == handle) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
 }
 void ifbinary_size_(
	LPCTSTR file_path
	,const HANDLE handle
	,DWORD&file_size_low	/* 4GB以下のときはこの値だけをみればよい */
	,DWORD&file_size_high	/* 4GB以上の上位ダブルワード */
 ) {
	file_size_low = ::GetFileSize(handle,&file_size_high);
	if (-1 == file_size_low) {
		const DWORD error_code = ::GetLastError();
		if (NO_ERROR != error_code) {
			throw std::domain_error(igs::resource::msg_from_err(
				error_code,file_path
			));
		}
	}
 }
 void ifbinary_read_(
	LPCTSTR file_path
	,const HANDLE handle
	,LPVOID lpBuffer // typedef void *LPVOID;
	,const DWORD nNumberOfBytesToRead
 ) {
	DWORD bytes_read = 0;
	if (0 == ::ReadFile(
		handle		//HANDLE hFile
		,lpBuffer
		,nNumberOfBytesToRead
		,&bytes_read	//,LPDWORD lpNumberOfBytesRead
		,NULL		//,LPOVERLAPPED lpOverlapped
	)) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
	if (nNumberOfBytesToRead != bytes_read) {
		throw std::domain_error(TEXT("Read different size"));
	}
 }
 void ifbinary_close_(
	LPCTSTR file_path
	,const HANDLE handle
 ) {
	if (0 == ::CloseHandle(handle)) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
 }
}
#if defined DEBUG_IFBINARY_WIN //----------------------------------
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
	HANDLE handle;
 std::cout << "ifbinary_open_(-)\n";
	ifbinary_open_(argv[1],handle);
	DWORD file_size_low =0;	/* 4GB以下のときはこの値だけをみればよい */
	DWORD file_size_high=0;	/* 4GB以上の上位ダブルワード */
 std::cout << "ifbinary_size_(-)\n";
	ifbinary_size_(argv[1],handle,file_size_low,file_size_high);
  std::cout	<< " file_size_low=" << file_size_low
		<< " file_size_high=" << file_size_high
		<< std::endl;
	std::vector<unsigned char> buffer(file_size_low);
	LPVOID pBuffer = static_cast<LPVOID>(&buffer.at(0));
	DWORD nNumberOfBytesToRead = static_cast<DWORD>(buffer.size());
 std::cout << "ifbinary_read_(-)\n";
	ifbinary_read_(argv[1],handle,pBuffer,nNumberOfBytesToRead);
 std::cout << "ifbinary_close_(-)\n";
	ifbinary_close_(argv[1],handle);
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_IFBINARY_WIN */ //-------------------------------
/*
rem -- WindowsXp sp3(32bits) vc2008sp1  :132,138 w! tes_w32_ifbinary_win.bat
set SRC=igs_resource_ifbinary_win
set TGT=tes_w32_ifbinary_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_IFBINARY_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem -- Windows7 (64bits) vc2008sp1  :139,145 w! tes_w64_ifbinary_win.bat
set SRC=igs_resource_ifbinary_win
set TGT=tes_w64_ifbinary_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_IFBINARY_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
*/

