#ifndef igs_image_function_invert_h
#define igs_image_function_invert_h

namespace igs {
 namespace image {
  namespace function {
	void invert(
		int bitspersample
		, size_t bytes_size
		, unsigned char *uchar_array
	);
  }
 }
}

#endif /* !igs_image_function_invert_h */
