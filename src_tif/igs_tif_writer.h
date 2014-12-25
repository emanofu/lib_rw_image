#ifndef igs_tif_writer_h
#define igs_tif_writer_h

#include "igs_image_irw.h"

namespace igs {
 namespace tif {
  namespace write {
/*------comment out-----------------------------------
�ʲ�ɸ�Ż���(�餷��)
#define	COMPRESSION_NONE	1	// dump mode
#define	COMPRESSION_CCITTRLE	2	// CCITT modified Huffman RLE
#define	COMPRESSION_PACKBITS	32773	// Macintosh RLE

�ʲ���ĥ����(�餷��)
#define	COMPRESSION_CCITTFAX3	3	// CCITT Group 3 fax encoding
#define	COMPRESSION_CCITT_T4	3       // CCITT T.4 (TIFF 6 name)
#define	COMPRESSION_CCITTFAX4	4	// CCITT Group 4 fax encoding
#define	COMPRESSION_CCITT_T6	4       // CCITT T.6 (TIFF 6 name)
#define	COMPRESSION_LZW		5       // Lempel-Ziv  & Welch
#define	COMPRESSION_JPEG	7	// %JPEG DCT compression
(���)
CCITTFAX3,CCITTFAX4�ǣ��Ͳ�������¸����ȡ�
Photoshop CS2(9.0.2)�ǡ�
	1.�ԥ������椬0.5�Ȥʤ�
	2.PHOTOMETRIC_MINISBLACK�λ����̵�뤷��0-->white,1-->black
�Ȥʤ����꤬���롣
����(5590x3417x1bits)�Υե����륵�����ϡ� 
	COMPRESSION_NONE	2409444bytes(Photoshop CS2(9.0.2))
	COMPRESSION_CCITTFAX4	  21542bytes
	COMPRESSION_CCITTFAX3	 129696bytes
	COMPRESSION_CCITTRLE	 125934bytes
�Ȥʤ�
------comment out----------------------------------*/
  }
  class writer : public igs::image::writer {
  public:
	writer();

	const char *const name(void) const; /* �ե������̾ */

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
	static const char *const name_; /* �ե������̾ */
  };
 }
}

#endif /* !igs_tif_writer_h */
