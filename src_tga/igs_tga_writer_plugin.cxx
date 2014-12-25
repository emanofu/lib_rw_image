#include "igs_tga_writer.h"
#include "igs_tga_writer_plugin.h"

namespace {
	igs::tga::writer cl_writer;
}
void *plugin_main_func(void) {
	return &cl_writer;
}
