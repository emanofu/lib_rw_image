#ifndef igs_sgi_decode_rle_h
#define igs_sgi_decode_rle_h

namespace igs {
 namespace sgi { /* SiliconGraphic�����ե����� */
  class decode_rle {
  public:
	decode_rle(
		  const unsigned char	*sour_top
		, const unsigned long	*sour_seek_set
		, const unsigned long	*sour_bytes_size
		, unsigned char	*dest_top
		, const int	dest_byte
		, const int	he
		, const int	wi
		, const int	ch
		, const int	sampling_bytes
	);

	void segment_exec(
		const int yy, const int zz, const bool rvs_sw=false
	); /* ��ʬ���� */
  private:
	decode_rle();

	const unsigned char *sour_top_;		/* �������껲�� */
	const unsigned long *sour_seek_set_,	/* �������껲�� */
			    *sour_bytes_size_;	/* �������껲�� */

	unsigned char	*dest_top_;		/* �������껲�� */
	const int	 dest_bytes_;
	const int he_, ch_, sampling_bytes_, scanline_bytes_;
  };
 }
}

#endif	/* !igs_sgi_decode_rle_h */
