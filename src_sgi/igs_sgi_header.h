#ifndef igs_sgi_header_h
#define igs_sgi_header_h

namespace igs {
 namespace sgi { /* SiliconGraphic���� */
  namespace header {
   namespace tag { /* SiliconGraphic(.rgb,.sgi)����ǧ�������� */
	const short s_magic = 474; /* for parameters.s_magic */
	/* 474(decimal) =0732(octal) =0x1da(hex) =111011010(binary) */
   }
   namespace compression { /* igs::sgi::compressions�ȹ�碌�뤳�� */
   /* �������̥�����(parameters.c_storage) */
    enum e_type {
	verbatim = 0,	/* ���̤򤷤ʤ�		*/
	rle      = 1,	/* ����󥰥�����ˡ	*/
    };
   }
   namespace form {
   /* �����񼰥�����(parameters.l_colormap) */
    enum e_type {
	normal = 0,	/* Image formed RGB/RGBA/GrayMap */
	dithered = 1,	/* Image formed RGB      about 1byte(1chanel) */
			/* R(0-2Bits),G(3-5Bits),B(6,7Bits) */
	screen   = 2,	/* Image formed ColorMap about 1byte(1chanel) */
	colormap = 3,	/* ColorMap saved by sgi machine */
    };
   }
   namespace resolution {
   /* ��������¸�Τ�����ȼ���ĥ 2007-11-5 */
	/* �����٤�ñ��
		tiff(Spec 6.0) use inch and centimeter
		png(Spec 1.2) use meter
	*/
	const char *const ca2_meter      = "me";
	const char *const ca2_centimeter = "ce";
	const char *const ca2_millimeter = "mi";
	const char *const ca2_inch       = "in";
   }
   /* �ե�����إå���¤ */
   struct parameters {
 	short s_magic;			/* SiliconGraphic�򼨤����� */
 	char c_storage;			/* ��¸���ΰ��̥⡼�� (0/1) */
 	char c_bytes_per_ch;		/* ��channel��byte��  (1/2) */
 	unsigned short us_dimension;	/* xyz������(1/2/3)	*/
 	unsigned short us_xsize;	/* pixel��		*/
 	unsigned short us_ysize;	/* pixel�⤵		*/
 	unsigned short us_zsize;	/* channel��(1/3/4)	*/
 	long l_pixmin;			/* pixel�κǾ���	*/
 	long l_pixmax;			/* pixel�κ�����	*/
 	long l_dummy;			/* dummy		*/
 	char ca_imagename[80];		/* ��������		*/
 	long l_colormap;		/* �����μ���򼨤�	*/
	char ca_dummy[404];
	/*	sizeof(igs::sgi::header::parameters)��
		512bytes�ˤ��뤿���dummy
	*/
   };
  }
 }
}

#endif	/* !igs_sgi_header_h */
