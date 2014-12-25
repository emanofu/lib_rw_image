#ifndef igs_tif_read_tile_h
#define igs_tif_read_tile_h

#include <vector>
#include "igs_tif_read_handle.h"

namespace igs {
 namespace tif {
  class read_tile {
  public:
	read_tile(
		igs::tif::read_handle& handle_ref,
		unsigned char *dest_top,
		const size_t dest_size
	);

	unsigned int count(void) const; /* v_segment()�¹Բ�� */

	void segment_exec(
		igs::tif::read_handle& handle_ref
	); /* �ƽ���缡(��ʬ)���� */

	void clear(void);

	~read_tile();
  private:
	read_tile();

	unsigned int _u_he, _u_wi, _u_bi;
	unsigned int _u_tile_he, _u_tile_wi;
	unsigned int _u_phot;
	unsigned int _u_tile_num_he, _u_tile_num_wi;
	unsigned int _u_pixel_size;
	std::vector<unsigned char> _cl_mem_tile;
	unsigned int
		_u_read_bytes_size,
		_u_read_bytes_crnt,
		_u_current_count,
		_u_current_x_image,
		_u_current_y_image;
	size_t dest_size_;
	unsigned char *dest_top_; /* �������껲�� */

	unsigned int _u_current_tile_w(void) const;
	unsigned int _u_current_tile_h(void) const;
  };
 }
}

#endif /* !igs_tif_read_tile_h */
