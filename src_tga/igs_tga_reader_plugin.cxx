#include "igs_tga_reader.h"
#include "igs_tga_reader_plugin.h"

namespace {
	igs::tga::reader cl_reader;
}
void *plugin_main_func(void)
{
	return &cl_reader;
}
