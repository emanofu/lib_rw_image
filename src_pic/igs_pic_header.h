#ifndef igs_pic_header_h
#define igs_pic_header_h

namespace igs {
 namespace pic { /* Softimage���� */
  namespace header {
   namespace tag { /* Softimage(.pic)����ǧ�������� */
	const unsigned int u_magic = 0x5380f634; /* big endian */
   }
   namespace field_type {
	const unsigned short us_no_field   = 0;
	const unsigned short us_odd_field  = 1;
	const unsigned short us_even_field = 2;
	const unsigned short us_full_frame = 3;
   }
   namespace compress_type { /* igs::pic::compressions�ȹ�碌�뤳�� */
	const unsigned char uc_uncompres = 0x00; /* uncompressed */
	const unsigned char uc_pure_rle  = 0x01; /* rle�Τ�dump�ޤޤ� */
	const unsigned char uc_mixed_rle = 0x02; /* rle�˲ä�dump�ޤ� */
   }
   namespace channel {
	const unsigned char uc_red_bit = 0x80;
	const unsigned char uc_gre_bit = 0x40;
	const unsigned char uc_blu_bit = 0x20;
	const unsigned char uc_alp_bit = 0x10;
   }

   /* �ե�����إå���¤ */
   struct parameters {
   	parameters():
		u_magic(0), f_version(0),
		us_width(0), us_height(0), f_ratio(0), us_fields(0),
		us_pad(0)
	{}
	unsigned int   u_magic;		/* Softimage|3D�ǡ����򼨤�id */
	float          f_version;	/* ��version */
	char ca80_comment[80];		/* eddie�Ǥ�crop���󤢤� */
	char ca4_id[4];			/* �����򼨤�id "PICT"������ */ 
	unsigned short us_width;	/* �����ԥ������� */
	unsigned short us_height;	/* �����ԥ�����⤵ */
	float          f_ratio;		/* �����Υԥ�������(��/�⤵) */
	unsigned short us_fields;	
	unsigned short us_pad;		/* ���ߡ��ǵͤ�Ƥ��� */
   };
   /* �����ͥ�ѥ��å� */
   struct channel_packet {
	channel_packet():
		uc_chained(0), uc_size(0), uc_type(0), uc_channels(0)
	{}
	unsigned char uc_chained;	/* zero�ʤ�Ϻǽ��ѥ��å�
			   1�ʤ餳�θ���̤Υѥ��åȤ�³����¸�ߤ��� */
	unsigned char uc_size;    /* nb of bits by channel (8)       */
	unsigned char uc_type;    /* �󰵽̤��뤤�ϰ�����ˡ�򼨤�    */
	unsigned char uc_channels;/* packet�ˤ��뿧channel�򼨤�flag */
   };
  }
 }
}

#endif	/* !igs_pic_header_h */
