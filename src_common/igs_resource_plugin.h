#ifndef igs_resource_plugin_h
#define igs_resource_plugin_h

/* --- .so or .dll�ե�������ɤ߹��ߥ��饹 - */
/* ���Ѿ��:
	���ץ�Υȥåפξ��ǥ����å��Ȥ��ƻȤ�
	���顼�Ϥ��δؿ��θƤӽФ��Ƥ�catch����
	��å������Ф��Ƽ���plugin�ɲäط�³������
*/

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# include <vector>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  /* ʣ���ץ饰����������ƻȤ� */
  IGS_RESOURCE_IRW_EXPORT void plugins_add(
	LPCTSTR file_path
	,std::vector<HMODULE>&libraries
	,std::vector<void *>&symbols
  );
  IGS_RESOURCE_IRW_EXPORT void plugins_close(
	std::vector<HMODULE>&libraries
  );
 }
}
#else //--------------------------------------------------------------
#include <vector>
namespace igs {
 namespace resource {
  /* ʣ���ץ饰����������ƻȤ� */
  void plugins_add(
	const char* file_path
	,std::vector<void *>&libraries
	,std::vector<void *>&symbols
  );
  void plugins_close(
	std::vector<void *>&libraries
  );
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_plugin_h */
