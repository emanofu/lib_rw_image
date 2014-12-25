#include <limits>
#include "igs_resource_irw.h"
#include "igs_image_function_invert.h"

/* ���ȿž(mono�����Τ�)
TIFFTAG_PHOTOMETRIC��PHOTOMETRIC_MINISWHITE�ξ��Ԥʤ�
TIFFTAG_PHOTOMETRIC��PHOTOMETRIC_RGB�ξ��ϴط��ʤ��ΤǤ��ʤ�
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
