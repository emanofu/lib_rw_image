#include "igs_resource_irw.h"
#include "igs_tga_decode.h"

//----------------------------------------------------------
igs::tga::decode::pixel::pixel(
	const unsigned char* cmap
	, const int cmap_index_of_entry
	, const int cmap_count_of_entries
	, const int cmap_bits_depth
	, const bool byte_swap_sw
):
	cmap_(cmap)
	, cmap_index_of_entry_(cmap_index_of_entry)
	, cmap_count_of_entries_(cmap_count_of_entries)
	, cmap_bits_depth_(cmap_bits_depth)
	, byte_swap_sw_(byte_swap_sw)
{}

void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::bgra32& in
	,   igs::image::pixel::rgba32& out
) { out.r = in.r; out.g = in.g; out.b = in.b; out.a = in.a;
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::bgr24& in
	,   igs::image::pixel::rgb24& out
) { out.r = in.r; out.g = in.g; out.b = in.b;
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::argb1555& in
	,   igs::image::pixel::rgba32& out
) {
	unsigned short b16 = in.bits16;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b16); }
	out.r = static_cast<unsigned char>(((b16>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b16>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b16    )&0x1f)*0xff/0x1f);
	out.a = static_cast<unsigned char>(((b16>>15)&0x01)*0xff/0x01);
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::argb0555& in
	,   igs::image::pixel::rgb24& out
) {
	unsigned short b15 = in.bits15;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b15); }
	out.r = static_cast<unsigned char>(((b15>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b15>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b15    )&0x1f)*0xff/0x1f);
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::cmap8& in
	,   igs::image::pixel::rgba32& out
) {
	int index = static_cast<int>(in.table8) +
				this->cmap_index_of_entry_;
	if ((0<=index) && (index < this->cmap_count_of_entries_)) {
#if _WIN32 & (1200 == _MSC_VER) // vc6
		switch (this->cmap_bits_depth_) {
		case 32: {
			const igs::tga::pixel::bgra32& in =
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgra32 *
				>(this->cmap_))[index];
			out.r = in.r;
			out.g = in.g;
			out.b = in.b;
			out.a = in.a;
			}
			break;
		case 24: {
			const igs::tga::pixel::bgr24& in =
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgr24 *
				>(this->cmap_))[index];
			out.r = in.r;
			out.g = in.g;
			out.b = in.b;
			out.a = 0xff;
			}
			break;
		case 16: {
			unsigned short b16 =
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb1555 *
				>(this->cmap_))[index].bits16;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b16); }
	out.r = static_cast<unsigned char>(((b16>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b16>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b16    )&0x1f)*0xff/0x1f);
	out.a = static_cast<unsigned char>(((b16>>15)&0x01)*0xff/0x01);
			}
			break;
		case 15: {
			unsigned short b15 =
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb0555 *
				>(this->cmap_))[index].bits15;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b15); }
	out.r = static_cast<unsigned char>(((b15>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b15>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b15    )&0x1f)*0xff/0x1f);
	out.a = 0xff;
			}
			break;
		}
#else
		switch (this->cmap_bits_depth_) {
		case 32:
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgra32 *
				>(this->cmap_))[index]
				, out
			);
			break;
		case 24: {
			igs::image::pixel::rgb24 ret24;
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgr24 *
				>(this->cmap_))[index]
				, ret24
			);
			out.r = ret24.r;
			out.g = ret24.g;
			out.b = ret24.b;
			out.a = 0xff;
			}
			break;
		case 16:
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb1555 *
				>(this->cmap_))[index]
				, out
			);
			break;
		case 15:
			igs::image::pixel::rgb24 ret24;
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb0555 *
				>(this->cmap_))[index]
				, ret24
			);
			out.r = ret24.r;
			out.g = ret24.g;
			out.b = ret24.b;
			out.a = 0xff;
			break;
		}
#endif
	} else {
		out.r = 0;
		out.g = 0;
		out.b = 0;
		out.a = 0;
	}
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::cmap8& in
	,   igs::image::pixel::rgb24& out
) {
	int index = static_cast<int>(in.table8) +
				this->cmap_index_of_entry_;
	if ((0<=index) && (index < this->cmap_count_of_entries_)) {
#if _WIN32 & (1200 == _MSC_VER) // vc6
		switch (this->cmap_bits_depth_) {
		case 32: {
			const igs::tga::pixel::bgra32& in =
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgra32 *
				>(this->cmap_))[index];
			out.r = in.r;
			out.g = in.g;
			out.b = in.b;
			}
			break;
		case 24: {
			const igs::tga::pixel::bgr24& in =
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgr24 *
				>(this->cmap_))[index];
			out.r = in.r;
			out.g = in.g;
			out.b = in.b;
			}
			break;
		case 16: {
			unsigned short b16 =
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb1555 *
				>(this->cmap_))[index].bits16;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b16); }
	out.r = static_cast<unsigned char>(((b16>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b16>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b16    )&0x1f)*0xff/0x1f);
			}
			break;
		case 15: {
			unsigned short b15 =
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb0555 *
				>(this->cmap_))[index].bits15;
	if (this->byte_swap_sw_) { igs::resource::swap_endian(b15); }
	out.r = static_cast<unsigned char>(((b15>>10)&0x1f)*0xff/0x1f);
	out.g = static_cast<unsigned char>(((b15>> 5)&0x1f)*0xff/0x1f);
	out.b = static_cast<unsigned char>(((b15    )&0x1f)*0xff/0x1f);
			}
			break;
		}
#else
		switch (this->cmap_bits_depth_) {
		case 32:{
			igs::image::pixel::rgba32 ret32;
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgra32 *
				>(this->cmap_))[index]
				, ret32
			);
			out.r = ret32.r;
			out.g = ret32.g;
			out.b = ret32.b;
			}
			break;
		case 24:
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
					const igs::tga::pixel::bgr24 *
				>(this->cmap_))[index]
				, out
			);
			break;
		case 16:{
			igs::image::pixel::rgba32 ret32;
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb1555 *
				>(this->cmap_))[index]
				, ret32
			);
			out.r = ret32.r;
			out.g = ret32.g;
			out.b = ret32.b;
			}
			break;
		case 15:
			igs::tga::decode::pixel::exec(
 				(igs::resource::const_pointer_cast<
				  const igs::tga::pixel::argb0555 *
				>(this->cmap_))[index]
				, out
			);
			break;
		}
#endif
	} else {
		out.r = 0;
		out.g = 0;
		out.b = 0;
	}
}
void igs::tga::decode::pixel::exec(
	const igs::tga::pixel::bw8& in
	,   igs::image::pixel::gray8& out
) {
	out.value8 = in.value8;
}
//----------------------------------------------------------
