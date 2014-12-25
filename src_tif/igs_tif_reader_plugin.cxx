#include "igs_tif_reader.h"
#include "igs_tif_reader_plugin.h"

namespace {
	igs::tif::reader cl_reader;
}
void *plugin_main_func(void) {
	return &cl_reader;
}
