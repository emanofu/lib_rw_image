#if defined _WIN32
#include "igs_resource_plugin_win.cxx"
#else
#include "igs_resource_plugin_unix.cxx"
#endif
/*
rem ------ WindowsXp sp3(32bits) vc2008sp1  :7,13 w! tes_w32_plugin.bat
set SRC=igs_resource_plugin
set TGT=tes_w32_plugin
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_PLUGIN_WIN /I%INC% %SRC%.cxx %HOMEDRIVE%%HOMEPATH%/utility_tools/vc2008x32md/implib/libigs_resource_irw_imp.lib %HOMEDRIVE%%HOMEPATH%/utility_tools/vc2008x32md/tiff-3.9.4/implib/libtiff_i.lib /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem ------ Windows7 (64bits) vc2008sp1  :14,20 w! tes_w64_plugin.bat
set SRC=igs_resource_plugin
set TGT=tes_w64_plugin
set INC=../../l01log/src
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_PLUGIN_WIN /I%INC% %SRC%.cxx %HOMEDRIVE%%HOMEPATH%/utility_tools/vc2008x64md/implib/libigs_resource_irw_imp.lib %HOMEDRIVE%%HOMEPATH%/utility_tools/vc2008x64md/tiff-3.9.4/implib/libtiff_i.lib /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# ------ rhel4 (32bits)  :21,22 w! tes_u32_plugin.csh
g++ -Wall -O2 -g -I. -DDEBUG_PLUGIN_UNIX igs_resource_plugin.cxx ${HOME}/utility_tools/rhel4/so/libigs_resource_irw.so ${HOME}/utility_tools/rhel4/tiff-3.9.4/lib/libtiff.so -ldl -o tes_u32_plugin
*/
