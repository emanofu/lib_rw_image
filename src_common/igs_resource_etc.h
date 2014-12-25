/* ����ԥ塼���꥽����(memory,disk,OS������)��(�����������)API */
#ifndef igs_resource_etc_h
#define igs_resource_etc_h

#include <algorithm> /*	std::swap() */

namespace igs {
 namespace resource {
  /* --- Endian�ѥХ��ȥ���å׽����ƥ�ץ졼�� --------------------- */
  template<class T> void swap_endian(T& val) {
	union { T val; unsigned char uc[sizeof(T)]; } buffer; 
	buffer.val = val;
	for (size_t ii=0; ii < sizeof(T)/2; ++ii) {
		std::swap( buffer.uc[ii],buffer.uc[sizeof(T)-1-ii] );
	}
	val = buffer.val;
  }
  /* --- �����Ǥ�Endian�ѥХ��ȥ���å׽����ƥ�ץ졼�� ------------- */
  template<class T> void swap_array_endian(
	T *val_array, const size_t size
  ) { for (size_t ii=0;ii<size; ++ii) { swap_endian(val_array[ii]); } }

  /* --- �ݥ��󥿴ָ��ꥭ�㥹��"reinterpret_cast"������ ----------- */
  /* "int" --> "void *" �� "void *" --> "int" ���ϥ��顼�Ȥʤ� */
  template<class T> inline T pointer_cast(void *p) {
	return static_cast<T>(p);
  }
  /* const�ݥ��󥿤���const�ݥ��󥿤ؤ�cast
	   const�Ǥʤ���Τ���const�ؤ����Ƥ��ޤ�! ---------------- */
  template<class T> inline T const_pointer_cast(const void *p) {
	return static_cast<T>(p);
  }
 }
}

#endif /* !igs_resource_etc_h */
