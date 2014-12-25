#include <cstring> /* strlen() */
#include <cstdio> /* vsnprintf() */
#include <cstdarg> /* vsnprintf() */
#include "igs_tif_error.h"

namespace {
 size_t copy_char(const char *src, const size_t dest_len, char *dest) {
	/* dest_len�Ͻ�üʸ����ޤ��礭�� */
	/* ���������礭�����ʤ���Фʤˤ⥳�ԡ����ʤ� */
	if (dest_len <= 1) { return 0; }

	/* ���ԡ������礭�������� */
	size_t siz = strlen(src);

	/* �礭���򥪡��С��������ǽ��ʬ��������� */
	if ((dest_len-1) < siz) { siz = dest_len-1; }
	if (0 < siz) { memmove(dest, src, siz); dest[siz] = '\0'; }

	/* ���ԡ��������򤫤��� */
	return siz;
 }
 size_t copy_va( const char* fmt, va_list ap, const size_t dest_len, char *dest ) {
	/* dest_len�Ͻ�üʸ����ޤ��礭�� */
	/* ���������礭�����ʤ���Фʤˤ⥳�ԡ����ʤ� */
	if (dest_len <= 1) { return 0; }

#if defined _WIN32 & (1200 == _MSC_VER) // vc6 compile_type
	const int i_ret = _vsnprintf( dest, dest_len, fmt,ap );
#elif defined _WIN32 & (1400 == _MSC_VER) // vc2005 compile_type
	const int i_ret = vsnprintf_s(dest, dest_len,dest_len, fmt,ap );
#else
	const int i_ret = vsnprintf(  dest, dest_len, fmt,ap );
#endif
	if (dest_len < static_cast<size_t>(i_ret)) {
		/* �ڤ�ͤ��줿�ΤǤ�����ο����֤� */
		return dest_len;
	}
	return i_ret;
 }
}

/*----------
libtiff��variable argument���顼��å�������charʸ����ˤ���
��ա�����åɥ����դǤϤʤ�

#include <tiffio.h>
#include <cstdarg>
typedef void (*TIFFErrorHandler)(const char* module, const char* fmt, va_list ap);
TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler handler);

Exsample1: "tmp.tif: 0: Invalid strip byte count, strip 0"
Exsample2: "tmp: Not a TIFF file, bad magic number 25462 (0x6376)
----------*/

namespace { char _ca_msg[1000]; }
void igs::tif::error_handler(
	const char* module,
	const char* fmt,
	va_list ap
) {
	/* ����� */
	size_t siz = 0;
	_ca_msg[0] = '\0';

	/* ���ϥե�����̾("tmp.tif"��) */
	siz += copy_char(module,1000-siz,&(_ca_msg[siz]));

	/* ���ڤ� */
	siz += copy_char(": ",  1000-siz,&(_ca_msg[siz]));

	/* libtiff�Υ��顼��å����� */
	siz += copy_va(fmt,ap,    1000-siz,&(_ca_msg[siz]));
}

/* libtiff�ؿ����顼�ΤȤ��Υ��顼��å�������charʸ��������� */
char *igs::tif::get_error_msg(void) { return _ca_msg; }
