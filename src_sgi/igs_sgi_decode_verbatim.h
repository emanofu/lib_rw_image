#ifndef igs_sgi_decode_verbatim_h
#define igs_sgi_decode_verbatim_h

namespace igs {
 namespace sgi { /* SiliconGraphic�����ե����� */
  class decode_verbatim {
  public:
	decode_verbatim(
		const unsigned char *sour_top
		, unsigned char *dest_top
		, const unsigned int he
		, const unsigned int wi
		, const unsigned int ch
		, const unsigned int sampling_bytes
	);

	void segment_exec(
		const int yy, const int zz, const bool rvs_sw=false
	); /* ��ʬ���� */
  private:
	decode_verbatim();

	const	unsigned char *sour_top_;	/* �������껲�� */
		unsigned char *dest_top_;	/* �������껲�� */
	const int he_, wi_, ch_, sampling_bytes_, scanline_bytes_;
  };
 }
}

#endif	/* !igs_sgi_decode_verbatim_h */
