/* コンピュータリソース(memory,disk,OS資源等々)の(可搬性を持つ)API */
#ifndef igs_resource_etc_h
#define igs_resource_etc_h

#include <algorithm> /*	std::swap() */

namespace igs {
 namespace resource {
  /* --- Endian用バイトスワップ処理テンプレート --------------------- */
  template<class T> void swap_endian(T& val) {
	union { T val; unsigned char uc[sizeof(T)]; } buffer; 
	buffer.val = val;
	for (size_t ii=0; ii < sizeof(T)/2; ++ii) {
		std::swap( buffer.uc[ii],buffer.uc[sizeof(T)-1-ii] );
	}
	val = buffer.val;
  }
  /* --- 配列版のEndian用バイトスワップ処理テンプレート ------------- */
  template<class T> void swap_array_endian(
	T *val_array, const size_t size
  ) { for (size_t ii=0;ii<size; ++ii) { swap_endian(val_array[ii]); } }

  /* --- ポインタ間限定キャスト"reinterpret_cast"の代わり ----------- */
  /* "int" --> "void *" や "void *" --> "int" 等はエラーとなる */
  template<class T> inline T pointer_cast(void *p) {
	return static_cast<T>(p);
  }
  /* constポインタからconstポインタへのcast
	   constでないものからconstへも出来てしまう! ---------------- */
  template<class T> inline T const_pointer_cast(const void *p) {
	return static_cast<T>(p);
  }
 }
}

#endif /* !igs_resource_etc_h */
