#include "igs_tif_writer.h"
#include "igs_tif_writer_plugin.h"

namespace {
	igs::tif::writer cl_writer;
}
void *plugin_main_func(void) {
	return &cl_writer;
}
