#ifndef igs_tif_writer_h
#define igs_tif_writer_h

#include "igs_image_irw.h"

namespace igs {
 namespace tif {
  namespace write {
/*------comment out-----------------------------------
以下標仕仕様(らしい)
#define	COMPRESSION_NONE	1	// dump mode
#define	COMPRESSION_CCITTRLE	2	// CCITT modified Huffman RLE
#define	COMPRESSION_PACKBITS	32773	// Macintosh RLE

以下拡張仕様(らしい)
#define	COMPRESSION_CCITTFAX3	3	// CCITT Group 3 fax encoding
#define	COMPRESSION_CCITT_T4	3       // CCITT T.4 (TIFF 6 name)
#define	COMPRESSION_CCITTFAX4	4	// CCITT Group 4 fax encoding
#define	COMPRESSION_CCITT_T6	4       // CCITT T.6 (TIFF 6 name)
#define	COMPRESSION_LZW		5       // Lempel-Ziv  & Welch
#define	COMPRESSION_JPEG	7	// %JPEG DCT compression
(注意)
CCITTFAX3,CCITTFAX4で２値画像を保存すると、
Photoshop CS2(9.0.2)で、
	1.ピクセル比が0.5となり
	2.PHOTOMETRIC_MINISBLACKの指定を無視して0-->white,1-->black
となる問題がある。
画像(5590x3417x1bits)のファイルサイズは、 
	COMPRESSION_NONE	2409444bytes(Photoshop CS2(9.0.2))
	COMPRESSION_CCITTFAX4	  21542bytes
	COMPRESSION_CCITTFAX3	 129696bytes
	COMPRESSION_CCITTRLE	 125934bytes
となる
------comment out----------------------------------*/
  }
  class writer : public igs::image::writer {
  public:
	writer();

	const char *const name(void) const; /* ファイル書式名 */

	const unsigned int ext_count(void);
	const char *const *const ext_lists(void);

	const unsigned int compression_count(void);
	const char *const *const compression_lists(void);

	const bool error_from_properties(
		const igs::image::properties& props
		, const int ext_number
		, std::ostringstream& error_or_warning_msg
	);
	void put_data(
		const igs::image::properties& prop
		, const size_t image_bytes
		, const unsigned char *image_array
		, const int compression_number
		, const std::string& history_str
		, const std::string& file_path
	);
  private:
	static const char *const name_; /* ファイル書式名 */
  };
 }
}

#endif /* !igs_tif_writer_h */
