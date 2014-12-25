#include <stdexcept> /* std::domain_error */
#include <sstream> /* std::ostringstream */
#include "igs_tif_error.h"
#include "igs_tif_write_handle.h"

/* libtiffのTIFFヘッダ用スマートクラス定義 */
//------------------------------------------------------------
using igs::tif::write_handle;
write_handle::write_handle(const char *file_path):
	p_(0)
{
	/* スレッドセーフではない */
	TIFFSetErrorHandler( igs::tif::error_handler );

	const char *const mode = "w";
	/* mode "r"	読みこみ
		"w"	書き出し
		"wl"	littleendianで書き出し --> 使わない
		"wb"	bigendianで書き出し --> 使わない
	*/
	this->p_ = TIFFOpen(file_path, mode);
	if (0 == this->p_) {
		throw std::domain_error( this->get_error_msg() );
	}
}
char *write_handle::get_error_msg(void)
{ return igs::tif::get_error_msg(); } /* スレッドセーフではない */
void write_handle::close(void)
{ if (0 != this->p_) { TIFFClose(this->p_); this->p_ = 0; } } 
write_handle::~write_handle()
{ try { this->close(); } catch(...) {} } /* destractorの例外を無効化 */
//------------------------------------------------------------
namespace {
 template<class T> int tiff_set_field(TIFF *tif,ttag_t tag,T arg) {
	return TIFFSetField(tif,tag,arg); }
 template<class T> void error_set_field(TIFF *tif,ttag_t tag,const char *name,T arg) {
	int err = tiff_set_field(tif,tag,arg);
	if (1 != err) {
		std::ostringstream os;
		os	<< "TIFFSetField(-) returns " << err << ","
			<< name << " can not set";
		throw std::domain_error(os.str());
	}
 }
}
//------------------------------------------------------------
void write_handle::set_uint16_throw(const unsigned int val,ttag_t tag,const char *name)
{ error_set_field(this->p_,tag,name,static_cast<uint16>(val)); }
void write_handle::set_uint32_throw(const unsigned int val,ttag_t tag,const char *name)
{ error_set_field(this->p_,tag,name,static_cast<uint32>(val)); }
void write_handle::set_float_throw(const double val,ttag_t tag,const char *name)
{ error_set_field(this->p_,tag,name,static_cast<float>(val)); }
//------------------------------------------------------------
