#include "igs_sgi_reader.h"
#include "igs_sgi_reader_plugin.h"

namespace {
	igs::sgi::reader cl_reader;
}
void *plugin_main_func(void)
{
	return &cl_reader;
}
