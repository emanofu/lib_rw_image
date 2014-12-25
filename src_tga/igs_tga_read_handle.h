#ifndef igs_tga_read_handle_h
#define igs_tga_read_handle_h

#include <string>
// #include "igs_image_irw.h"
#include "igs_tga_header.h"

namespace igs {
 namespace tga { /* Targe画像 */
  namespace read_handle {
   const bool istag(
	const unsigned int bytes_count
	, const unsigned char* const tag_array
	, bool& byte_swap_sw
   );
   void get_info( /* 情報をパラメータで得る */
	const std::string& file_path
	, bool& byte_swap_sw
	, igs::tga::header::parameters& header_params
   );
   int bytes_from_bits(
	const int bits_par_pixel
   );
   int dest_channels(
	const int bits_par_pixel
	, const int cmap_bits_depth
   );
   void get_size(
	const std::string&file_path
	, unsigned int& image_size
	, unsigned int& history_size
   );
   void check_parameters(
	igs::tga::header::parameters& param
   );
   void get_data(
	const size_t file_size
	, unsigned char* file_image
	, bool& byte_swap_sw
	, igs::tga::header::parameters& param_ref
	, unsigned char*& image_ptr_ref
	, size_t& image_size_ref
	, unsigned char*& cmap_ptr_ref
   );
  }
 }
}

#endif	/* !igs_tga_read_handle_h */
