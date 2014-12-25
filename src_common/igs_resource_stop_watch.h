#ifndef igs_resource_stop_watch_h
#define igs_resource_stop_watch_h

/* ����®�ٷ�¬�ѥ��ȥåץ����å����饹 */

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  class IGS_RESOURCE_IRW_EXPORT stop_watch {
  public: stop_watch();
	void start(void);
	double stop(void);
	void stop(const char *msg);
  private:
	LARGE_INTEGER m_counterPrevious_, m_frequencyCounter_;
  };
 }
}
#else //--------------------------------------------------------------
namespace igs {
 namespace resource {
  class stop_watch {
  public: stop_watch();
	void start(void);
	double stop(void);
	void stop(const char *msg);
  private:
	double before_time_;
  };
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_stop_watch_h */
