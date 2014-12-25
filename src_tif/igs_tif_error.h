#ifndef igs_tif_error_h
#define igs_tif_error_h

namespace igs {
 namespace tif {
	void error_handler(
		const char* module,
		const char* fmt,
		va_list ap
	);
	char* get_error_msg(void);
 }
}

#endif /* !igs_tif_error_h */
