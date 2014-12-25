#if defined _WIN32 // win32 or win64
# include "igs_resource_file_stat_win.cxx"
#else
# include "igs_resource_file_stat_unix.cxx"
#endif
