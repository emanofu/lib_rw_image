#ifndef igs_tif_read_handle_h
#define igs_tif_read_handle_h

#include "tiffio.h" /* TIFF */

namespace igs {
 namespace tif {
  class read_handle { /* libtiffの開く用TIFFヘッダ生成破棄クラス */
  /* igs::tif::get_error_msg()を使うためスレッドセーフではない */
  public:
	explicit
	read_handle(const char *file_path);

	char *get_error_msg(void);
	operator TIFF*() { return this->p_; }

	unsigned int imagelength(void);
	unsigned int imagewidth(void);
	unsigned int samplesperpixel(void);
	unsigned int bitspersample(void);
	unsigned int photometric(void);
	unsigned int compression(void);
	unsigned int planarconfig(void);
	unsigned int tilewidth(void);
	unsigned int tilelength(void);
	unsigned int orientation(void);
	unsigned int resolutionunit(void);
	double       xresolution(void);
	double       yresolution(void);
	unsigned int fillorder(void);
	int istiled(void);

/******	int i_read_strip(
		unsigned int u_current_strip,
		unsigned char* ucharp_current,
		int i_strip_size
	);
	int i_read_tile(
		unsigned char* ucharp_current,
		unsigned int u_current_x_image,
		unsigned int u_current_y_image
	);
	int i_read_scanline(
		unsigned char* ucharp_current,
		unsigned int u_y
	);
	unsigned int u_size_scanline(void);
	int i_strip_size(void);
	unsigned int u_strip_count(void);
******/

	void check_parameters(void);

	void close(void);
	~read_handle();
  private:
	read_handle();

	TIFF *p_;
  };
 }
}

#endif /* !igs_tif_read_handle_h */
