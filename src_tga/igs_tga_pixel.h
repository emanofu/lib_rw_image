#ifndef igs_tga_pixel_h
#define igs_tga_pixel_h

namespace igs {
 namespace image {
  namespace pixel {
   struct rgba32{rgba32():r(0),g(0),b(0),a(0){} unsigned char r,g,b,a;};
   struct rgb24	{ rgb24():r(0),g(0),b(0)     {} unsigned char r,g,b;};
   struct gray8	{ gray8():value8(0)          {} unsigned char value8;};
   bool equal(const rgba32&p1, const rgba32&p2);
   bool equal(const rgb24& p1, const rgb24& p2);
   bool equal(const gray8& p1, const gray8& p2);
  }
 }
}
namespace igs {
 namespace tga {
  namespace pixel {
   struct bgra32{bgra32():b(0),g(0),r(0),a(0){} unsigned char b,g,r,a;};
   struct bgr24	{ bgr24():b(0),g(0),r(0)     {} unsigned char b,g,r; };
   struct argb1555 {argb1555():bits16(0)     {} unsigned short bits16;};
   struct argb0555 {argb0555():bits15(0)     {} unsigned short bits15;};
   struct cmap8	{ cmap8():table8(0)          {} unsigned char table8;};
   struct bw8	{ bw8():value8(0)            {} unsigned char value8;};
  }
 }
}

#endif	/* !igs_tga_pixel_h */
