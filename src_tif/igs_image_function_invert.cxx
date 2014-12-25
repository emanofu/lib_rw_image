#include <limits>
#include "igs_resource_irw.h"
#include "igs_image_function_invert.h"

/* 白黒反転(mono画像のみ)
TIFFTAG_PHOTOMETRICがPHOTOMETRIC_MINISWHITEの場合行なう
TIFFTAG_PHOTOMETRICがPHOTOMETRIC_RGBの場合は関係ないのでやらない
*/
void igs::image::function::invert(
	int bitspersample
	, size_t bytes_size
	, unsigned char *uchar_array
) {
	size_t ii;
	if (1 == bitspersample) {
		for (ii=0; ii<bytes_size; ++ii) {
			uchar_array[ii] = ~uchar_array[ii];
		}
	} else
	if (std::numeric_limits<unsigned char>::digits==bitspersample) {
		for (ii = 0; ii < bytes_size; ++ii) {
			uchar_array[ii] = std::numeric_limits<
				unsigned char>::max()-uchar_array[ii];
		}
	} else
	if (std::numeric_limits<unsigned short>::digits==bitspersample){
		unsigned short *ushort_array =
			igs::resource::pointer_cast<unsigned short *>(
				uchar_array);
		for (ii=0; ii < bytes_size/2; ++ii) {
			ushort_array[ii] = std::numeric_limits<
				unsigned short>::max()-ushort_array[ii];
		}
	}
}
