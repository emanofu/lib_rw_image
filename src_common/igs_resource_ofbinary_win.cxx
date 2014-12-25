#include <windows.h>
#include "igs_resource_msg_from_err.h"
#if defined DEBUG_OFBINARY_WIN //----------------------------------
# include "igs_resource_msg_from_err_win.cxx"
#endif  /* !DEBUG_OFBINARY_WIN */ //-------------------------------

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
 void ofbinary_open_(
	LPCTSTR file_path
	,HANDLE&handle
 ) {
	handle = ::CreateFile(
		file_path		// LPCTSTR lpFileName
		,GENERIC_WRITE		//,DWORD dwDesiredAccess
		,0/* =��ͭ���ʤ� */	//,DWORD dwShareMode
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
	,LONG&file_seek_low/* 2GB�ʲ��ΤȤ��Ϥ����ͤ�����ߤ�Ф褤 */
	,LONG&file_seek_high/* 2GB�ʾ�ξ�̥��֥��� */
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
�ȥ�֥�  2011-02-09
 ::WriteFile()��(10842080����礭������������¸�����)���顼��
 "�����ƥ� �꥽��������­���Ƥ��뤿�ᡢ�׵ᤵ�줿�����ӥ���λ�Ǥ��ޤ���"
 ��������
�����ˡ  2011-02-14
 WindowsXp��ꥹ�����Ȥ���
�ƥȥ�֥�  2011-03-01
 ::WriteFile()��(62124001�ʾ����¸�����)���顼��
 "�����ƥ� �꥽��������­���Ƥ��뤿�ᡢ�׵ᤵ�줿�����ӥ���λ�Ǥ��ޤ���"
 ��������
 restart���Ƥ⡢
 ::WriteFile()��(47000000�ʾ����¸�����)���顼��
����  2011-03-01-����2
 Network��Ǥʤ���
 Local��Ǽ¹Ԥ���ȡ�
 ::WriteFile()��(2000000000����¸)OK
����  2011-03-01
 Network�������
�Ʋ����ˡ  2011-03-01
 Windows7�Ǥ�Network/Local�ɤ����(2000000000����¸)OK
 Windows7�ؤΰܹԤˤ�äƲ�褹��Τ��褤����
 WindowsXp�ǤϺ��ΤȤ����礭��ʪ�򰷤�ʤ����ᡢ������α
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
	LONG file_seek_low =0; /* 2GB�ʲ��ΤȤ��Ϥ����ͤ�����ߤ�Ф褤 */
	LONG file_seek_high=0; /* 2GB�ʾ�ξ�̥��֥��� */
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

