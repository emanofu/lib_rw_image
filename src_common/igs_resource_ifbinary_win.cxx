#include <windows.h>
#include "igs_resource_msg_from_err.h"
#if defined DEBUG_IFBINARY_WIN //----------------------------------
# include "igs_resource_msg_from_err_win.cxx"
#endif  /* !DEBUG_IFBINARY_WIN */ //-------------------------------
namespace {
/*
���ѥǡ�����  �磻��ʸ��(UNICODE)(��1)	�ޥ���Х���ʸ��(_MBCS)(��2)
TCHAR	      wchar_t			char 
LPTSTR	      wchar_t *			char * 
LPCTSTR	      const wchar_t *		const char * 
��1  1ʸ����16�ӥåȤΥ磻��ʸ���Ȥ���ɽ��Unicode ��Ȥ���ˡ
	���٤Ƥ�ʸ���� 16 �ӥåȤ˸��ꤵ��ޤ���
	�ޥ���Х���ʸ������١������Ψ���㲼���ޤ�������®�٤ϸ��夷�ޤ�
��2  1ʸ����ʣ���ΥХ��Ȥ�ɽ���ޥ���Х���ʸ��
	MBCS(Multibyte Character Set) �ȸƤФ��ʸ�������Ȥ���ˡ
	����Ĺ���������¾塢���ݡ��Ȥ���Ƥ���Τ� 2 �Х���ʸ���ޤǤʤΤǡ�
	�ޥ���Х���ʸ���� 1 ʸ���� 1 �Х��Ȥޤ��� 2 �Х��ȤȤʤ�ޤ���
	Windows 2000 �ʹߡ�Windows �������� Unicode ����Ѥ��Ƥ��뤿�ᡢ
	�ޥ���Х���ʸ������Ѥ����������ʸ������Ѵ���ȯ�����뤿��
	�����С��إåɤ�ȯ�����ޤ���
	UNICODE��_MBCS��̤����ΤȤ��Ϥ�����ˤʤ롣
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
	,DWORD&file_size_low	/* 4GB�ʲ��ΤȤ��Ϥ����ͤ�����ߤ�Ф褤 */
	,DWORD&file_size_high	/* 4GB�ʾ�ξ�̥��֥��� */
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
	DWORD file_size_low =0;	/* 4GB�ʲ��ΤȤ��Ϥ����ͤ�����ߤ�Ф褤 */
	DWORD file_size_high=0;	/* 4GB�ʾ�ξ�̥��֥��� */
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

