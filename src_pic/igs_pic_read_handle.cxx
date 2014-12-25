#include <vector> /* std::vector */
#include <sstream> /* std::ostringstream */
#include <stdexcept> /* std::domain_error */
#include "igs_resource_irw.h"
#include "igs_pic_read_handle.h"

/* 
�ե�����񼰤����������ɤ���������ͤ��֤���
byte swap���뤫�򻲾Ȱ��������֤�
*/
const bool
igs::pic::read_handle::istag(const unsigned int bytes_count, const unsigned char *const tag_array, bool& byte_swap_sw) {
	if (bytes_count < 4) { return false; }

	union { unsigned char uca[4]; unsigned int ui; } ut;
	ut.ui = igs::pic::header::tag::u_magic;

	if (	(ut.uca[0] == tag_array[0]) &&
		(ut.uca[1] == tag_array[1]) &&
		(ut.uca[2] == tag_array[2]) &&
		(ut.uca[3] == tag_array[3])
	) {
		/* cpu��byte order��file��byte order��Ʊ�� */
		byte_swap_sw = false;
		return true; /* OK */
	}
	if (	(ut.uca[3] == tag_array[0]) &&
		(ut.uca[2] == tag_array[1]) &&
		(ut.uca[1] == tag_array[2]) &&
		(ut.uca[0] == tag_array[3])
	) {
		/* cpu��byte order��file��byte order���դΤȤ� */
		byte_swap_sw = true;/* byte swap���ʤ����ɤ� */
		return true; /* OK */
	}
	return false; /* Error */
}
namespace {
 /* �ե�����񼰤���äƤ����throw */
 void check_tag(
	const unsigned int bytes_count,
	const unsigned char *const tag_array,
	bool& byte_swap_sw
 ) {
	if (igs::pic::read_handle::istag(
		bytes_count, tag_array, byte_swap_sw)) { return; }

	std::ostringstream os;
	  os
		<< std::hex
		<< "file tag(";
	for (unsigned int ii=0; ii < bytes_count; ++ii) {
	 if (0 < ii) {
	  os
		<< ' ';
	 }
	  os
		<< (int)tag_array[ii];
	}
	  os
		<< ")"
		<< "is not pic image tag("
		<< igs::pic::header::tag::u_magic
		<< ")"
		<< std::dec;
	throw std::domain_error(os.str());
 }
 void byte_swap_header(igs::pic::header::parameters *header_ptr) {
	igs::resource::swap_endian( header_ptr->u_magic );
	igs::resource::swap_endian( header_ptr->f_version );
	igs::resource::swap_endian( header_ptr->us_width );
	igs::resource::swap_endian( header_ptr->us_height );
	igs::resource::swap_endian( header_ptr->f_ratio );
	igs::resource::swap_endian( header_ptr->us_fields );
	igs::resource::swap_endian( header_ptr->us_pad );
 }
 void read_packet_to_flag(
	igs::resource::ifbinary& cl_fp,
	unsigned char& channel_flag_ref
 ) {
	unsigned char uc4_buf[4]; // 0=chained,1=size,2=type,3=channels
	do {	cl_fp.read(uc4_buf,4);
		channel_flag_ref |= uc4_buf[3];
	} while (0 != uc4_buf[0]);
 }
 void channel_flag_to_size(unsigned char channel_flag, unsigned int& channel_size_ref) {
	using namespace igs::pic::header::channel;
	if (uc_red_bit&channel_flag) {++channel_size_ref;}
	if (uc_gre_bit&channel_flag) {++channel_size_ref;}
	if (uc_blu_bit&channel_flag) {++channel_size_ref;}
	if (uc_alp_bit&channel_flag) {++channel_size_ref;}
 }
}
/* �ե����뤫�顢��ǽ�ʤ��٤ƤΥե������������� */
void
igs::pic::read_handle::get_info(
	const std::string& file_path,
	bool &byte_swap_sw,
	igs::pic::header::parameters& header_params_ref,
	std::vector<igs::pic::header::channel_packet>& packet_vector,
	unsigned char &channel_flag,
	unsigned int &channel_size
) {
	/* �ե����볫�� */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* �ե����뤫��إå����Τ��ɤ� */
	cl_fp.read(
	 igs::resource::pointer_cast<unsigned char *>(&header_params_ref)
		,sizeof(igs::pic::header::parameters)
	);

	/* �Ϥ��4bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	byte_swap_sw=false;
	check_tag(
		sizeof(igs::pic::header::tag::u_magic),
		igs::resource::pointer_cast<unsigned char *>(&header_params_ref),
		byte_swap_sw
	);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(&header_params_ref); }

	/* channel��(channel packet���ɤ߹����)Ĵ�� */
	channel_flag = 0;
	if (0 < packet_vector.size()) {
		packet_vector.clear();
	}
	unsigned char uc4_buf[4];
	do {
		packet_vector.resize(
		 packet_vector.size()+1
		);
		cl_fp.read(uc4_buf,4);

		packet_vector.back().uc_chained =uc4_buf[0];
		packet_vector.back().uc_size    =uc4_buf[1];
		packet_vector.back().uc_type    =uc4_buf[2];
		packet_vector.back().uc_channels=uc4_buf[3];
		channel_flag |=
		 packet_vector.back().uc_channels;
	} while (0 != packet_vector.back().uc_chained);

	channel_size = 0;
 	channel_flag_to_size(channel_flag, channel_size);

	cl_fp.close();
}

/* �ե����뤫�顢�ե����륵���������� */
void
igs::pic::read_handle::get_size(
	const std::string &file_path,
	unsigned int &image_size
) {
	/* �ե����볫�� */
	igs::resource::ifbinary cl_fp(file_path.c_str());

	/* �ե����뤫��إå����Τ��ɤ� */
	igs::pic::header::parameters header_params;
	cl_fp.read(
		igs::resource::pointer_cast<unsigned char *>(&header_params)
		,sizeof(igs::pic::header::parameters)
	);

	/* �Ϥ��4bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	bool byte_swap_sw=false;
	check_tag(
		sizeof(igs::pic::header::tag::u_magic),
		igs::resource::pointer_cast<unsigned char *>(&header_params),
		byte_swap_sw
	);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(&header_params); }

	/* channel��(channel packet���ɤ߹����)Ĵ�� */
	unsigned char channel_flag = 0;
	read_packet_to_flag(cl_fp,channel_flag);
	unsigned int packet_size = 0;
	channel_flag_to_size(channel_flag, packet_size);

	image_size = header_params.us_height *
		     header_params.us_width * packet_size;

	cl_fp.close();
}
/* �ǡ���(�ե����륤�᡼��)���顢�ƾ���ȥǡ��������� */
void
igs::pic::read_handle::get_data(
	unsigned char *file_array,
	bool &byte_swap_sw,
	igs::pic::header::parameters *&header_ptr_ref, // <-- file_array
	igs::pic::header::channel_packet *&packet_ptr_ref,// <--file_array
	unsigned int &packet_size_ref,
	unsigned int &channel_size_ref,
	unsigned char *&image_ptr_ref // <-- file_array
) {
	/* �Ϥ��4bytes����ե�����񼰳�ǧ�ȡ�byte swap���뤫�ɤ��� */
	check_tag(
		sizeof(igs::pic::header::tag::u_magic),
		file_array,
		byte_swap_sw
	);

	/* �ե����륤�᡼������إå�����ݥ��󥿤� */
	header_ptr_ref = igs::resource::pointer_cast<
		igs::pic::header::parameters *>(file_array);

	/* file�Ǥ�endian��OnMemory��endian���դΤȤ�swap���� */
	if (byte_swap_sw) { byte_swap_header(header_ptr_ref); }

	/* channel packet �ڤ� channel�� */
	packet_ptr_ref = igs::resource::pointer_cast<
		igs::pic::header::channel_packet *>(
			file_array +
			sizeof(igs::pic::header::parameters)
		);
	/* packet��������channel�� */
	unsigned char channel_flag = 0;
	int ii = 0;
	do { channel_flag |= (packet_ptr_ref)[ii].uc_channels;
	} while (0 != (packet_ptr_ref)[ii++].uc_chained);
	packet_size_ref = ii;

	channel_size_ref = 0;
 	channel_flag_to_size(channel_flag, channel_size_ref);

	/* ������������ */
	image_ptr_ref =  file_array +
	 sizeof(igs::pic::header::parameters) +
	 sizeof(igs::pic::header::channel_packet) * packet_size_ref;
}

