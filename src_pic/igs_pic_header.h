#ifndef igs_pic_header_h
#define igs_pic_header_h

namespace igs {
 namespace pic { /* Softimage画像 */
  namespace header {
   namespace tag { /* Softimage(.pic)画像認識タグ値 */
	const unsigned int u_magic = 0x5380f634; /* big endian */
   }
   namespace field_type {
	const unsigned short us_no_field   = 0;
	const unsigned short us_odd_field  = 1;
	const unsigned short us_even_field = 2;
	const unsigned short us_full_frame = 3;
   }
   namespace compress_type { /* igs::pic::compressionsと合わせること */
	const unsigned char uc_uncompres = 0x00; /* uncompressed */
	const unsigned char uc_pure_rle  = 0x01; /* rleのみdump含まず */
	const unsigned char uc_mixed_rle = 0x02; /* rleに加えdump含む */
   }
   namespace channel {
	const unsigned char uc_red_bit = 0x80;
	const unsigned char uc_gre_bit = 0x40;
	const unsigned char uc_blu_bit = 0x20;
	const unsigned char uc_alp_bit = 0x10;
   }

   /* ファイルヘッダ構造 */
   struct parameters {
   	parameters():
		u_magic(0), f_version(0),
		us_width(0), us_height(0), f_ratio(0), us_fields(0),
		us_pad(0)
	{}
	unsigned int   u_magic;		/* Softimage|3Dデータを示すid */
	float          f_version;	/* 書式version */
	char ca80_comment[80];		/* eddieではcrop情報あり */
	char ca4_id[4];			/* 画像を示すid "PICT"が入る */ 
	unsigned short us_width;	/* 画像ピクセル幅 */
	unsigned short us_height;	/* 画像ピクセル高さ */
	float          f_ratio;		/* 画像のピクセル比(幅/高さ) */
	unsigned short us_fields;	
	unsigned short us_pad;		/* ダミーで詰めてある */
   };
   /* チャンネルパケット */
   struct channel_packet {
	channel_packet():
		uc_chained(0), uc_size(0), uc_type(0), uc_channels(0)
	{}
	unsigned char uc_chained;	/* zeroならは最終パケット
			   1ならこの後に別のパケットが続いて存在する */
	unsigned char uc_size;    /* nb of bits by channel (8)       */
	unsigned char uc_type;    /* 非圧縮あるいは圧縮方法を示す    */
	unsigned char uc_channels;/* packetにある色channelを示すflag */
   };
  }
 }
}

#endif	/* !igs_pic_header_h */
