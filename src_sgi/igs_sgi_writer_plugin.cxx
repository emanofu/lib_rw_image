#include "igs_sgi_writer.h"
#include "igs_sgi_writer_plugin.h"

namespace {
	igs::sgi::writer cl_writer;
}
void *plugin_main_func(void) {
	return &cl_writer;
}
