#ifndef igs_sgi_header_h
#define igs_sgi_header_h

namespace igs {
 namespace sgi { /* SiliconGraphic画像 */
  namespace header {
   namespace tag { /* SiliconGraphic(.rgb,.sgi)画像認識タグ値 */
	const short s_magic = 474; /* for parameters.s_magic */
	/* 474(decimal) =0732(octal) =0x1da(hex) =111011010(binary) */
   }
   namespace compression { /* igs::sgi::compressionsと合わせること */
   /* 画像圧縮タイプ(parameters.c_storage) */
    enum e_type {
	verbatim = 0,	/* 圧縮をしない		*/
	rle      = 1,	/* ランレングス圧縮法	*/
    };
   }
   namespace form {
   /* 画像書式タイプ(parameters.l_colormap) */
    enum e_type {
	normal = 0,	/* Image formed RGB/RGBA/GrayMap */
	dithered = 1,	/* Image formed RGB      about 1byte(1chanel) */
			/* R(0-2Bits),G(3-5Bits),B(6,7Bits) */
	screen   = 2,	/* Image formed ColorMap about 1byte(1chanel) */
	colormap = 3,	/* ColorMap saved by sgi machine */
    };
   }
   namespace resolution {
   /* 解像度保存のための独自拡張 2007-11-5 */
	/* 解像度の単位
		tiff(Spec 6.0) use inch and centimeter
		png(Spec 1.2) use meter
	*/
	const char *const ca2_meter      = "me";
	const char *const ca2_centimeter = "ce";
	const char *const ca2_millimeter = "mi";
	const char *const ca2_inch       = "in";
   }
   /* ファイルヘッダ構造 */
   struct parameters {
 	short s_magic;			/* SiliconGraphicを示すタグ */
 	char c_storage;			/* 保存時の圧縮モード (0/1) */
 	char c_bytes_per_ch;		/* 各channelのbyte数  (1/2) */
 	unsigned short us_dimension;	/* xyz次元数(1/2/3)	*/
 	unsigned short us_xsize;	/* pixel幅		*/
 	unsigned short us_ysize;	/* pixel高さ		*/
 	unsigned short us_zsize;	/* channel数(1/3/4)	*/
 	long l_pixmin;			/* pixelの最小値	*/
 	long l_pixmax;			/* pixelの最大値	*/
 	long l_dummy;			/* dummy		*/
 	char ca_imagename[80];		/* 画像情報		*/
 	long l_colormap;		/* 画像の種類を示す	*/
	char ca_dummy[404];
	/*	sizeof(igs::sgi::header::parameters)を、
		512bytesにするためのdummy
	*/
   };
  }
 }
}

#endif	/* !igs_sgi_header_h */
