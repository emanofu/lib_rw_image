#include "igs_resource_irw.h"
#include "igs_tga_encode.h"

//----------------------------------------------------------
igs::tga::encode::pixel::pixel(
	const bool byte_swap_sw
):
	byte_swap_sw_(byte_swap_sw)
{}
void igs::tga::encode::pixel::exec(
	const igs::image::pixel::rgba32& in
	,       igs::tga::pixel::bgra32& out
) {
	out.r = in.r;
	out.g = in.g;
	out.b = in.b;
	out.a = in.a;
}
void igs::tga::encode::pixel::exec(
	const igs::image::pixel::rgb24& in
	,       igs::tga::pixel::bgr24& out
) {
	out.r = in.r;
	out.g = in.g;
	out.b = in.b;
}
void igs::tga::encode::pixel::exec(
	const igs::image::pixel::rgba32& in
	,       igs::tga::pixel::argb1555& out
) {
	out.bits16 = static_cast<unsigned short>(
		(((int)(in.r)>>3)&0x1f)<<10 |
		(((int)(in.g)>>3)&0x1f)<< 5 |
		(((int)(in.b)>>3)&0x1f)<< 0 |
		(((int)(in.a)>>7)&0x01)<<15
	);
	if (this->byte_swap_sw_) {
		igs::resource::swap_endian(out.bits16); }
}
void igs::tga::encode::pixel::exec(
	const igs::image::pixel::rgb24&  in
	,       igs::tga::pixel::argb0555& out
) {
	out.bits15 = static_cast<unsigned short>(
		(((int)(in.r)>>3)&0x1f)<<10 |
		(((int)(in.g)>>3)&0x1f)<< 5 |
		(((int)(in.b)>>3)&0x1f)<< 0
	);
	if (this->byte_swap_sw_) {
		igs::resource::swap_endian(out.bits15); }
}
void igs::tga::encode::pixel::exec(
	const igs::image::pixel::gray8&  in
	,       igs::tga::pixel::bw8&      out
) {
	out.value8 = in.value8;
}
//----------------------------------------------------------
void igs::tga::encode::wbytes(
	char* data, int size
	, const std::string& file_path
	, std::ofstream& ofs
) {
	ofs.write(data,size);
	if (!ofs) {
		std::string str("std::ofstream("); str += file_path;
		str += ") write error";
		throw std::domain_error(str);
	}
}
