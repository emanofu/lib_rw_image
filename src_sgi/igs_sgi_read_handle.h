#ifndef igs_sgi_read_handle_h
#define igs_sgi_read_handle_h

#include <string>
#include "igs_image_irw.h"
#include "igs_sgi_header.h"

namespace igs {
 namespace sgi { /* SiliconGraphic画像 */
  namespace read_handle {
   const bool istag(
	const unsigned int bytes_count
	, const unsigned char *const tag_array
	, bool& byte_swap_sw
   );
   void get_info( /* 情報をパラメータで得る */
	const std::string& file_path
	, bool &byte_swap_sw
	, igs::sgi::header::parameters& header_params_ref
   );
   void get_reso(
	const char imagename[80]
	, igs::image::resolution::unit& reso_unit
	, double& reso_x
	, double& reso_y
   );
   void get_size(
	const std::string &file_path
	, size_t &image_size
	, size_t &history_size
   );
   void check_parameters(
	igs::sgi::header::parameters *header_ptr
   );
   void get_data(
	const size_t file_size
	, unsigned char *file_image
	, bool& byte_swap_sw
	, igs::sgi::header::parameters*& header_ptr_ref
	, unsigned long*& seek_set_ptr_ref
	, unsigned long*& bytes_size_ptr_ref
	, unsigned char*& image_ptr_ref
	, char*& history_ptr_ref
   );
  }
 }
}

#endif	/* !igs_sgi_read_handle_h */
