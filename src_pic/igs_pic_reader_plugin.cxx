#include "igs_pic_reader.h"
#include "igs_pic_reader_plugin.h"

namespace {
	igs::pic::reader cl_reader;
}
void *plugin_main_func(void) {
	return &cl_reader;
}
