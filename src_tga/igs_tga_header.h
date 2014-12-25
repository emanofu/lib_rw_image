#ifndef igs_tga_header_h
#define igs_tga_header_h

namespace igs {
 namespace tga {
  namespace header {

   namespace compression {
    enum e_type {
        uncompressed = 0,	/* ���̤򤷤ʤ�         */
        rle = 1,		/* ����󥰥�����ˡ   */
    };
   }

   /******namespace image_type {
    namespace pixel_type {
	const unsigned char color_map  = 1;
	const unsigned char full_color = 2;
	const unsigned char grayscale  = 3;
    }
    namespace compress_bit {
	const unsigned char rle = 0x08;
    }
   }******/

   /* R/W���ѥե�����إå���¤ */
   struct parameters_rw {
	parameters_rw():
		id_length(0)
		, cmap_type(0)
		, image_type(0)
		, cmap_index_of_entry0(0)
		, cmap_index_of_entry1(0)
		, cmap_count_of_entries0(0)
		, cmap_count_of_entries1(0)
		, cmap_bits_depth(0)
		, left_term_pos(0)
		, lower_term_pos(0)
		, width(0)
		, height(0)
		, bits_par_pixel(0)
		, descriptor(0)
	{}
	unsigned char id_length;
	unsigned char cmap_type;
	unsigned char image_type;
	unsigned char cmap_index_of_entry0;
	unsigned char cmap_index_of_entry1;
	unsigned char cmap_count_of_entries0;
	unsigned char cmap_count_of_entries1;
	unsigned char cmap_bits_depth;
	short left_term_pos;
	short lower_term_pos;
	short width;
	short height;
	unsigned char bits_par_pixel;
	unsigned char descriptor;
   };
   /* �ե�����إå���¤ */
   struct parameters {
	parameters():
		id_length(0)
		, cmap_type(0)
		, image_type(0)
		, cmap_index_of_entry(0)
		, cmap_count_of_entries(0)
		, cmap_bits_depth(0)
		, left_term_pos(0)
		, lower_term_pos(0)
		, width(0)
		, height(0)
		, bits_par_pixel(0)
		, descriptor(0)
	{}
	unsigned char id_length;	/* ID field
					0=�ʤ�
					1<=ʸ����
					*/
	unsigned char cmap_type;	/* ColorMap field
					0=ColorMap�ʤ�
					1=ColorMap����
					*/
	unsigned char image_type;	/* ���� field
					0=�����ʤ�
					1=ColorMap(256��)(̵����)
					2=FullColor(BGRA/BGR)(̵����)
					3=Grayscale(���)(̵����)
					9=ColorMap(256��)(RLE����)
					10=FullColor(BGRA/BGR)(RLE����)
					11=Grayscale(���)(RLE����)
					*/

	short cmap_index_of_entry;	/* 0�ͤλ���cmap���� */
	short cmap_count_of_entries;
	unsigned char cmap_bits_depth;	/* 15,16,24,32bits
					32bits=BGRA
					24bits=BGR
					16bits=ARRRRRGG GGGBBBBB(bit)
					15bits=xRRRRRGG GGGBBBBB(bit)
					*/

	short left_term_pos;		/* ������ü���� 0���� */
	short lower_term_pos;		/* ������ü���� 0���� */
	short width;			/* ������ */
	short height;			/* �����⤵ */
	unsigned char bits_par_pixel;	/* ��������(8,16,24,32Bits)*/
	unsigned char descriptor;	/* ����°��
				bit0...3=��Pixel�˴ط�����°����Bits��
					TGA16=0 or 1
					TGA24=0
					TGA32=8
				bit4=������Ǽ����
					0=�����鱦
					1=�����麸
				bit5=�岼��Ǽ����
					0=�������
					1=�夫�鲼
				bit6,7=�ǡ�����¸����Interleave����
					00=non-interleaved
					01=two-way(even/odd)interleaving
					10=four way interleaving
					11=reserved
					*/
   };
  }
 }
}

#endif	/* !igs_tga_header_h */
