#include "igs_pic_writer.h"
#include "igs_pic_writer_plugin.h"

namespace {
	igs::pic::writer cl_writer;
}
void *plugin_main_func(void) {
	return &cl_writer;
}
