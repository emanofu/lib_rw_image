#include <limits> /* std::numeric_limits */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_pic_extensions.h"
#include "igs_pic_header.h"
#include "igs_pic_encode_write.h"
#include "igs_pic_writer.h"

/*----------------------------------------------------------*/
// using igs::pic::write::properties;

using igs::pic::writer;
writer::writer()
{}

/* File書式名 */
const char *const writer::name_ =
	"Softimage File Format"
	;
const char *const writer::name(void) const { return this->name_; }

/* 拡張子数 */
const unsigned int
writer::ext_count(void) {
	return sizeof(igs::pic::extensions)/sizeof(char *);
}
/* 拡張子名リスト */
const char *const *const
writer::ext_lists(void) {
	return igs::pic::extensions;
}

/* 圧縮名リスト実データ */
namespace igs {
 namespace pic { /* igs::pic::header::compress_typeと合わせること */
  const char *const compressions[] = {
		"uncompressd"
		, "mixed_rle"
		, "pure_rle"
  };
/*
0 -> uc_uncompres  0x00  uncompressed
1 -> uc_mixed_rle  0x02  rleに加えdump含む
2 -> uc_pure_rle   0x01  rleのみdump含まず
*/
 }
}
/* 圧縮種類数 */
const unsigned int
writer::compression_count(void) {
	return sizeof(igs::pic::compressions)/sizeof(char *);
}
/* 圧縮名リスト */
const char *const *const
writer::compression_lists(void) {
	return igs::pic::compressions;
}

/* 保存可不可チェック */
/* .pic  RGBA/RGB/Grayscale  8bits */
const bool writer::error_from_properties(
	const igs::image::properties& props,
	const int ext_number,
	std::ostringstream& error_or_warning_msg
) {
	bool error_sw = false;

	if (igs::pic::extension::pic != static_cast<
	    igs::pic::extension::type>(ext_number)
	) {
		error_or_warning_msg
			<< "Error : extension number(" << ext_number
			<< ") Must be "
			<< static_cast<int>(igs::pic::extension::pic)
			<< "\n";
		error_sw = true;
	}

	if (0 == props.width) {
		error_or_warning_msg
			<< "Error : width(0) Must Not be zero\n";
		error_sw = true;
	}
	if (0 == props.height) {
		error_or_warning_msg
			<< "Error : height(0) Must Not be zero\n";
		error_sw = true;
	}
	if (	(1 != props.channels) &&
		(3 != props.channels) &&
		(4 != props.channels)
	) {
		error_or_warning_msg
			<< "Error : channel(" << props.channels
			<< ") Must be 1 or 3 or 4\n";
		error_sw = true;
	}
	if (	static_cast<int>(props.bits) !=
		(std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char))
	) {
		error_or_warning_msg
			<< "Error : sampling bits(" << props.bits
			<< ") Must be "
		<< (std::numeric_limits<unsigned char>::digits *
		sizeof(unsigned char))
			<< "\n";
		error_sw = true;
	}
	/* 画像の向き(botleftの場合は保存時上下反転して保存する) */
	if (	(igs::image::orientation::topleft != props.orie_side) &&
		(igs::image::orientation::botleft != props.orie_side)
	) {
		error_or_warning_msg
		 << "Error : orientaion(" << props.orie_side
		 << ") Must be topleft("
		 << static_cast<int>(igs::image::orientation::topleft)
		 << ") or botleft("
		 << static_cast<int>(igs::image::orientation::botleft)
		 << ")\n";
		error_sw = true;
	}
	/* 解像度 */
	switch (props.reso_unit) {
	case igs::image::resolution::not_defined:
		break;
	case igs::image::resolution::centimeter:
	case igs::image::resolution::millimeter:
	case igs::image::resolution::meter:
	case igs::image::resolution::inch:
	case igs::image::resolution::nothing:
		error_or_warning_msg
			<< "Warning : resolution will not save\n";
		break;
	}
	return error_sw;
}

/* 保存
正常終了、例外処理のcatch、デストラクタ、で
内部メモリ解放し、ファイルを閉じること
*/
void
writer::put_data(
	const igs::image::properties& props
	, const size_t image_bytes
	, const unsigned char *image_array
	, const int compression_number
	, const std::string& history_str
	, const std::string& file_path
) {
	history_str; /* for stopping warning C4100 */
	if (image_bytes < (
		static_cast<size_t>(props.width) *
		props.height * props.channels
	)) {
		std::ostringstream os;
		os << "memory size<" << image_bytes
			<< "> is smaller than "
			<< props.width << "x"
			<< props.height << "x"
			<< props.channels;
		throw std::domain_error(os.str());
	}
	/* ヘッダ設定 --------------------------------------*/
	igs::pic::header::parameters head_params;
	head_params.u_magic = igs::pic::header::tag::u_magic;

	/* 最後に使っていたDKitのVersion */
	head_params.f_version = 3.4909f;

	for (int ii=0;ii<80;++ii){head_params.ca80_comment[ii]=0;}
	head_params.ca4_id[0] = 'P';
	head_params.ca4_id[1] = 'I';
	head_params.ca4_id[2] = 'C';
	head_params.ca4_id[3] = 'T';
	head_params.us_width  =
		static_cast<unsigned short>(props.width);
	head_params.us_height =
		static_cast<unsigned short>(props.height);
	head_params.f_ratio = 1.0f;
	head_params.us_fields = 
		igs::pic::header::field_type::us_full_frame;
	head_params.us_pad = 0;

	/* CPUインディアン
	intel(x86)上で、そのまま保存した場合 byte-swappedの画像になる
	mips (68k)上で、そのまま保存した場合 byte-orderedの画像になる
	intel(x86)(little endian)のCPUでは、byte swapして保存する
	----------------------------------------------------*/
	union { unsigned short us; unsigned char uc[2]; } buf;
	buf.us = 1;
	/*
	CPU Byte Orderの説明
	Big Endian CPUではbuf.usに1いれるとbuf.uc[1]が1となる
			+---------------+
	unsigned short	|      us       |
			|           1   |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   0   |   1   |
			+-------+-------+
	Little Endian CPUではbuf.usに1いれるとbuf.uc[0]が1となる
			+---------------+
	unsigned short	|      us       |
			|   1           |
			+-------+-------+
	unsigned char	| uc[0] | uc[1] |
			|   1   |   0   |
			+-------+-------+
	*/
	bool byte_swap_sw = false;
	if (1 == buf.uc[0]) { /* Little Endian CPU */
		byte_swap_sw = true; /* then save with swapping */
	}

	/* ヘッダをバイトスワップする ----------------------*/
	igs::pic::header::parameters save_params = head_params;
	if ( byte_swap_sw ) {
		igs::resource::swap_endian( save_params.u_magic );
		igs::resource::swap_endian( save_params.f_version );
		igs::resource::swap_endian( save_params.us_width );
		igs::resource::swap_endian( save_params.us_height );
		igs::resource::swap_endian( save_params.f_ratio );
		igs::resource::swap_endian( save_params.us_fields );
		igs::resource::swap_endian( save_params.us_pad );
	}

	/* ファイル開く ------------------------------------*/
 	std::ofstream ofs(file_path.c_str(), std::ios::binary);
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> open<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* ヘッダ保存 --------------------------------------*/
	ofs.write(
		igs::resource::pointer_cast<char *>( &save_params ),
		sizeof(igs::pic::header::parameters)
	);
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> write<std::ofstream> error";
		throw std::domain_error(str);
	}

	/* packet保存 --------------------------------------*/
	unsigned char uc4_buf[4];
	std::vector<igs::pic::header::channel_packet>
						packet_array2(2);
	unsigned char comp_num = 0;
	switch (compression_number) {
	case 0: comp_num = igs::pic::header::compress_type::uc_uncompres; break;
	case 1: comp_num = igs::pic::header::compress_type::uc_mixed_rle; break;
	case 2: comp_num = igs::pic::header::compress_type::uc_pure_rle; break;
	}
/*
0 -> uc_uncompres  0x00  uncompressed
1 -> uc_mixed_rle  0x02  rleに加えdump含む
2 -> uc_pure_rle   0x01  rleのみdump含まず
*/
	if (4 <= props.channels) {
	 packet_array2[0].uc_chained = uc4_buf[0] = 1;
	} else {
	 packet_array2[0].uc_chained = uc4_buf[0] = 0;
	}
	packet_array2[0].uc_size    = uc4_buf[1] =
			std::numeric_limits<unsigned char>::digits;
	packet_array2[0].uc_type    = uc4_buf[2] = comp_num;
	packet_array2[0].uc_channels= uc4_buf[3] =
		igs::pic::header::channel::uc_red_bit |
		igs::pic::header::channel::uc_gre_bit |
		igs::pic::header::channel::uc_blu_bit;
	ofs.write( igs::resource::pointer_cast<char *>(uc4_buf),4 );
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> write<std::ofstream> error";
		throw std::domain_error(str);
	}

	if (4 <= props.channels) {
	 packet_array2[1].uc_chained = uc4_buf[0] = 0;
	 packet_array2[1].uc_size    = uc4_buf[1] =
			std::numeric_limits<unsigned char>::digits;
	 packet_array2[1].uc_type    = uc4_buf[2] = comp_num;
	 packet_array2[1].uc_channels= uc4_buf[3] =
		igs::pic::header::channel::uc_alp_bit;
	 ofs.write( igs::resource::pointer_cast<char *>(uc4_buf),4 );
	 if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> write<std::ofstream> error";
		throw std::domain_error(str);
	 }
	}

	/* 画像保存 ----------------------------------------*/
	/*****************const unsigned char *save_array = image_array;

	igs::resource::memory_array<unsigned char> write_image;
	if (igs::image::orientation::botleft == props.orie_side) {
		write_image.resize(
			props.height *
			igs::image::byte_of_scanline(
				props.width,
				props.channels,
				props.bits
			)
		);
		unsigned int sl_samp_sz = props.width * props.channels;
		const unsigned char *sl_in  = image_array;
		unsigned char *sl_out = &write_image.at(0) +
				(props.height - 1) * sl_samp_sz;
		for (unsigned int yy = 0; yy < props.height/2; ++yy) {
			for (unsigned int xx=0; xx<sl_samp_sz; ++xx) {
				sl_out[xx] = sl_in[xx];
			}
			sl_in  += sl_samp_sz;
			sl_out -= sl_samp_sz;
		}

		save_array = &write_image.at(0);
	}**********************************/
	/* topleftならそのまま、botleftの場合は上下反転して保存する */
	igs::pic::encode_write cl_encode_wr(
		//save_array,
		const_cast<unsigned char *>(image_array),
		static_cast<unsigned int>(head_params.us_height),
		static_cast<unsigned int>(head_params.us_width),
		props.channels,
		(igs::image::orientation::botleft == props.orie_side)
	);
	for (unsigned int ui = 0; ui < cl_encode_wr.count(); ++ui) {
		cl_encode_wr.segment_exec(
			packet_array2,
			byte_swap_sw,
			ofs
		);
	}
	ofs.close();
	if (!ofs) {
		std::string str;
		str = "file<";
		str += file_path;
		str += "> close<std::ofstream> error";
		throw std::domain_error(str);
	}
}

