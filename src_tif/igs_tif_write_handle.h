#ifndef igs_tif_write_handle_h
#define igs_tif_write_handle_h

#include "tiffio.h" /* TIFF */

namespace igs {
 namespace tif {
  class write_handle { /* libtiffの保存用TIFFヘッダ生成破棄クラス */
  /* igs::tif:get_error_msg()を使うためスレッドセーフではない */
  public:
	explicit
	write_handle(const char *file_path);

	char *get_error_msg(void);
	operator TIFF*() { return this->p_; }

	void set_uint16_throw(const unsigned int val,
			ttag_t tag,const char *name);
	void set_uint32_throw(const unsigned int val,
			ttag_t tag,const char *name);
	void set_float_throw(const double val,
			ttag_t tag,const char *name);

	void close(void);
	~write_handle();
  private:
	write_handle();

	TIFF *p_;
  };
 }
}

#endif /* !igs_tif_write_handle_h */
