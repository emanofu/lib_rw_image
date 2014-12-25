#ifndef igs_tga_encode_h
#define igs_tga_encode_h

#include <fstream> // std::ofstream
#include <sstream> // std::ostringstream
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_tga_pixel.h"

namespace igs {
 namespace tga {
  namespace encode {
   /* --- Pixelñ�̤ν��� ------------------------------- */
   class pixel {
   public:
	pixel(const bool byte_swap_sw);

	void exec(const igs::image::pixel::rgba32& in
			, igs::tga::pixel::bgra32&   out);
	void exec(const igs::image::pixel::rgb24&  in
			, igs::tga::pixel::bgr24&    out);
	void exec(const igs::image::pixel::rgba32& in
			, igs::tga::pixel::argb1555& out);
	void exec(const igs::image::pixel::rgb24&  in
			, igs::tga::pixel::argb0555& out);
	void exec(const igs::image::pixel::gray8&  in
			, igs::tga::pixel::bw8&      out);
   private:
	//pixel(){}
	const bool byte_swap_sw_;
   };
   /* --- 1�İʾ��ƱPixelϢ³ -------------------------- */
   template<class SOU> size_t count_rle_pixel(
	const SOU* pixel, const size_t psize
   ) {
	for (size_t current=1; current < psize; ++current) {/* ������ */
		SOU tmp = *pixel++;		  /* pixel���꼡�� */
		if (!igs::image::pixel::equal(tmp,(*pixel))) {
			return current; }		/* ��Pixel�� */
	}
	return psize;		       /* �롼�פ��Ǹ�ޤǤ��ä���� */
   }
   /* --- 0�İʾ�ΰ�Pixel��Ϣ³ ------------------------ */
   template<class SOU> size_t count_raw_pixel(
	const SOU* pixel, const size_t psize
   ) {
	for (size_t current=1; current < psize; ++current) {/* ������ */
		SOU tmp = *pixel++;		  /* pixel���꼡�� */
		if (igs::image::pixel::equal(tmp,(*pixel))) {
			return current-1; }		/* ³Pixel�� */
	}
	return psize;		       /* �롼�פ��Ǹ�ޤǤ��ä���� */
   }
   /* --- bytes data����¸ ------------------------------ */
   void wbytes(
	char* data, int size
	, const std::string& file_path
	, std::ofstream& ofs
   );
   /* --- Pixel����¸ ----------------------------------- */
   template<class DES> void wpixel(
	const DES& pixel
	, const std::string& file_path
	, std::ofstream& ofs
   ) {
	igs::tga::encode::wbytes( igs::resource::pointer_cast<char *>(
		const_cast<DES *>(&pixel)), sizeof(DES), file_path, ofs
	);
   }
   /* --- Run Length����沽(encode)����¸ -------------- */
   template<class SOU, class DES> void run_length(
	  const SOU* sour_image
	, const size_t sour_bytes
	, DES& dest_pixel
	, igs::tga::encode::pixel& encoder
	, const std::string& file_path, std::ofstream& ofs
   ) {
	size_t rle_count = 0;
	size_t raw_count = 0;

	const	size_t sou_tota_pixel = sour_bytes / sizeof(SOU);
		size_t sou_rest_pixel = sour_bytes / sizeof(SOU);
	for (size_t ii = 0; ii < sou_tota_pixel; ++ii) {
		rle_count = count_rle_pixel(sour_image,sou_rest_pixel);
		raw_count = count_raw_pixel(sour_image,sou_rest_pixel);
		if (128 < rle_count) { rle_count = 128; }
		if (128 < raw_count) { raw_count = 128; }

		if (1 < rle_count) {
			if (sou_rest_pixel < rle_count) {return;}
			sou_rest_pixel -= rle_count;
			ii += rle_count;

			/* Ƭ�Υ�����(RLE packet) */
			char byte=static_cast<char>((rle_count-1)|0x80);
			wbytes(&byte,1,file_path,ofs);

			/* rle�ǡ�������¸ */
			encoder.exec(*sour_image,dest_pixel);
			wpixel(dest_pixel,file_path,ofs);
			sour_image += rle_count;
		}
		if (0 < raw_count) {
			if (sou_rest_pixel < raw_count) {return;}
			sou_rest_pixel -= raw_count;
			ii += rle_count;

			/* Ƭ�Υ�����(RAW packet) */
			char byte = static_cast<char>(raw_count-1);
			wbytes(&byte,1,file_path,ofs);

			/* raw�ǡ�������¸ */
			while (0 < raw_count--) {
				encoder.exec(*sour_image++,dest_pixel);
				wpixel(dest_pixel,file_path,ofs);
			}
		}
	}
   }
   /* --- Uncompressed�Ȥ�����¸ ------------------------ */
   template<class SOU, class DES> void uncompressed(
	  const SOU* sour_image
	, const size_t sour_bytes
	, DES& dest_pixel
	, igs::tga::encode::pixel& encoder
	, const std::string& file_path, std::ofstream& ofs
   ) {
	size_t sou_rest_pixel = sour_bytes / sizeof(SOU);
	for (size_t ii = 0; ii < sou_rest_pixel; ++ii) {
		encoder.exec(*sour_image++,dest_pixel);
		wpixel(dest_pixel,file_path,ofs);
	}
   }
  }
 }
}

#endif	/* !igs_tga_encode_h */
