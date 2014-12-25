#ifndef pic_read_handle_h
#define pic_read_handle_h

#include <string>
#include <vector>
#include "igs_pic_header.h"

namespace igs {
 namespace pic {
  namespace read_handle {
   const bool istag(
	const unsigned int bytes_count
	, const unsigned char *const tag_array
	, bool& byte_swap_sw
   );
   void get_info( /* 情報をパラメータで得る */
	const std::string& file_path
	, bool& byte_swap_sw
	, igs::pic::header::parameters& header_params
	, std::vector<igs::pic::header::channel_packet>& packet_vector
	, unsigned char& channel_flag
	, unsigned int& packet_size
   );
   void get_size(
	const std::string& file_path
	, unsigned int& image_size
   );
   void get_data(
	unsigned char *file_array
	, bool& byte_swap_sw
	, igs::pic::header::parameters*& header_ptr_ref
	, igs::pic::header::channel_packet*& packet_ptr_ref
	, unsigned int& packet_size_ref
	, unsigned int& channel_size_ref
	, unsigned char*& image_ptr_ref
   );
  }
 }
}

#endif	/* !pic_read_handle_h */
