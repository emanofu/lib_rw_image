#include <windows.h>
#include "igs_resource_msg_from_err.h"
#if defined DEBUG_OFBINARY_WIN //----------------------------------
# include "igs_resource_msg_from_err_win.cxx"
#endif  /* !DEBUG_OFBINARY_WIN */ //-------------------------------

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
 void ofbinary_open_(
	LPCTSTR file_path
	,HANDLE&handle
 ) {
	handle = ::CreateFile(
		file_path		// LPCTSTR lpFileName
		,GENERIC_WRITE		//,DWORD dwDesiredAccess
		,0/* =共有しない */	//,DWORD dwShareMode
		,NULL	//,LPSECURITY_ATTRIBUTES lpSecurityAttributes
		,CREATE_ALWAYS		//,DWORD dwCreationDisposition
		,FILE_ATTRIBUTE_NORMAL	//,DWORD dwFlagsAndAttributes
		,NULL			//,HANDLE hTemplateFile
	);
	if (INVALID_HANDLE_VALUE == handle) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
 }
 void ofbinary_seek_(
	LPCTSTR file_path
	,const HANDLE handle
	,LONG&file_seek_low/* 2GB以下のときはこの値だけをみればよい */
	,LONG&file_seek_high/* 2GB以上の上位ダブルワード */
 ) {
	file_seek_low = ::SetFilePointer(
		handle ,file_seek_low ,&file_seek_high ,FILE_BEGIN
	);

	if (INVALID_SET_FILE_POINTER == file_seek_low) {
		const DWORD error_code = ::GetLastError();
		if (NO_ERROR != error_code) {
			throw std::domain_error(igs::resource::msg_from_err(
				error_code,file_path
			));
		}
	}
 }
 void ofbinary_write_(
	LPCTSTR file_path
	,const HANDLE handle
	,LPCVOID lpBuffer
	,const DWORD nNumberOfBytesToWrite
 ) {
/*
トラブル  2011-02-09
 ::WriteFile()で(10842080より大きいサイズで保存すると)エラー、
 "システム リソースが不足しているため、要求されたサービスを完了できません。"
 が起きた
解決方法  2011-02-14
 WindowsXpをリスタートする
再トラブル  2011-03-01
 ::WriteFile()で(62124001以上で保存すると)エラー、
 "システム リソースが不足しているため、要求されたサービスを完了できません。"
 が起きた
 restartしても、
 ::WriteFile()で(47000000以上で保存すると)エラー、
現象  2011-03-01-その2
 Network上でなく、
 Local上で実行すると、
 ::WriteFile()で(2000000000で保存)OK
原因  2011-03-01
 Network上の制約。
再解決方法  2011-03-01
 Windows7ではNetwork/Localどちらも(2000000000で保存)OK
 Windows7への移行によって解決するのがよいだろう
 WindowsXpでは今のところ大きい物を扱わないため、、、保留
*/
	DWORD bytes_write = 0;
	if (0 == ::WriteFile(
		handle		//HANDLE hFile
		,lpBuffer
		,nNumberOfBytesToWrite
		,&bytes_write	//,LPDWORD lpNumberOfBytesWritten
		,NULL		//,LPOVERLAPPED lpOverlapped
	)) {
		throw std::domain_error(igs::resource::msg_from_err(
			::GetLastError(),file_path
		));
	}
	if (nNumberOfBytesToWrite != bytes_write) {
		throw std::domain_error(TEXT("Write different size"));
	}
 }
 void ofbinary_close_(
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
#if defined DEBUG_OFBINARY_WIN //----------------------------------
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
	HANDLE handle;
 std::cout << "ofbinary_open_(-)\n";
	ofbinary_open_(argv[1],handle);
 std::cout << "ofbinary_seef_(-)\n";
	LONG file_seek_low =0; /* 2GB以下のときはこの値だけをみればよい */
	LONG file_seek_high=0; /* 2GB以上の上位ダブルワード */
	ofbinary_seek_(argv[1],handle,file_seek_low,file_seek_high);
  std::cout	<< " file_seek_low=" << file_seek_low
		<< " file_seek_high=" << file_seek_high
		<< std::endl;
	const size_t bytes = atoi(argv[2]);
 std::cout << "memory_alloc=" << bytes << "bytes\n";
	std::vector<unsigned char> buffer(bytes);
 std::cout << "memory_init=" << bytes << "bytes\n";
	LPVOID pBuffer = static_cast<LPVOID>(&buffer.at(0));
	DWORD nNumberOfBytesToRead = static_cast<DWORD>(buffer.size());
	for (DWORD ii=0;ii<(nNumberOfBytesToRead-1);ii+=2) {
		buffer[ii]   = 'i';
		buffer[ii+1] = '\n';
	}
	buffer[nNumberOfBytesToRead-1] = '\n';
 std::cout << "ofbinary_write_(-)\n";
 std::cout << " size=" << nNumberOfBytesToRead << std::endl;
	ofbinary_write_(argv[1],handle,pBuffer,nNumberOfBytesToRead);
 std::cout << "ofbinary_close_(-)\n";
	ofbinary_close_(argv[1],handle);
 }
 catch (std::exception& e) {
	std::cerr << "exception \"" << e.what() << "\"" << std::endl;
 }
 catch (...) {
	std::cerr << "other std::exception" << std::endl;
 }
	return 0;
}
#endif  /* !DEBUG_OFBINARY_WIN */ //-------------------------------
/*
rem -- WindowsXp sp3(32bits) vc2008sp1  :169,175 w tes_w32_ofbinary_win.bat
set SRC=igs_resource_ofbinary_win
set TGT=tes_w32_ofbinary_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_OFBINARY_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem -- Windows7 (64bits) vc2008sp1  :176,182 w tes_w64_ofbinary_win.bat
set SRC=igs_resource_ofbinary_win
set TGT=tes_w64_ofbinary_win
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_OFBINARY_WIN /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
*/

