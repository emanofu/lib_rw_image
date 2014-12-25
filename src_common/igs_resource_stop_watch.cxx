#if defined _WIN32 // vc compile_type
# include <windows.h>
# include <limits> /* std::numeric_limits */
#else
# include <sys/types.h> /*         _ftime(),_ftime_s() */
# include <sys/timeb.h> /* ftime(),_ftime(),_ftime_s() */
#endif
#include <iostream> /* std::cout */
#include "igs_resource_stop_watch.h"

using igs::resource::stop_watch;
#if defined _WIN32 //-------------------------------------------------
stop_watch::stop_watch() {
	QueryPerformanceFrequency(&this->m_frequencyCounter_);
}
void stop_watch::start(void) {
	QueryPerformanceCounter(&this->m_counterPrevious_);
}
double stop_watch::stop(void) {
	LARGE_INTEGER m_counterCurrent;
	QueryPerformanceCounter(&m_counterCurrent);
	double d_timeFromPrevius = (double)(
			m_counterCurrent.QuadPart -
		this->m_counterPrevious_.QuadPart
	) /	this->m_frequencyCounter_.QuadPart;
	this->m_counterPrevious_ = m_counterCurrent;
	return d_timeFromPrevius;
}
void stop_watch::stop(const char *msg) {
	std::cout.precision(std::numeric_limits<double>::digits10);
	std::cout << this->stop() << "sec" << '\t' << msg << '\n';
	std::cout.precision(6); /* defaultに戻す */
}
#else //--------------------------------------------------------------
stop_watch::stop_watch(): before_time_(0.0) {} 
void stop_watch::start(void) {
	struct timeb t_now;
	ftime(&t_now);
/*
clock_t times(struct tms *buf);
clock_t clock(void);
等々の使用を検討すること
*/
	this->before_time_ = (double)t_now.time + t_now.millitm/1000.0;
}
double stop_watch::stop(void) {
	struct timeb t_now;
	ftime(&t_now);
	double d_next = (double)t_now.time + t_now.millitm/1000.0;
	double d_difftime = d_next - this->before_time_;
	this->before_time_ = d_next;
	return d_difftime;
}
void stop_watch::stop(const char *msg) {
	std::cout << this->stop() << "sec";
	if ((0 != msg) && (0 != msg[0])) {
	 std::cout << '\t' << msg;
	}
	std::cout << '\n';
}
/******double stop_watch::_d_time(void) {
#if defined _WIN32 // vc compile_type
	struct _timeb t_now;
# if (1400 == _MSC_VER) // vc2005 compile_type
	_ftime_s(&(t_now));
# else
	_ftime(&(t_now));
# endif
#else
	struct timeb t_now;
	ftime(&(t_now));
#endif
	return (double)t_now.time + t_now.millitm/1000.0;
}******/
#endif //-------------------------------------------------------------

#if defined DEBUG_STOP_WATCH
int main(int argc,char *argv[])
{
	igs::resource::stop_watch cl_stopw;
#if defined _WIN32
	cl_stopw.start(); Sleep(1000); cl_stopw.stop("1 sec");
	cl_stopw.start(); Sleep(2000); cl_stopw.stop("2 sec");
	cl_stopw.start(); Sleep(3000); cl_stopw.stop("3 sec");
#else
	cl_stopw.start(); sleep(1); cl_stopw.stop("1 sec");
	cl_stopw.start(); sleep(2); cl_stopw.stop("2 sec");
	cl_stopw.start(); sleep(3); cl_stopw.stop("3 sec");
#endif
	return 0;
}
#endif  /* !DEBUG_STOP_WATCH */

/*
rem --- WindowsXp sp3(32bits) vc2008sp1  :95,101 w! tes_w32_stop_watch.bat
set SRC=igs_resource_stop_watch
set TGT=tes_w32_stop_watch
set INC=.
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_STOP_WATCH /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
rem --- Windows7 (64bits) vc2008sp1  :102,108 w! tes_w64_stop_watch.bat
set SRC=igs_resource_stop_watch
set TGT=tes_w64_stop_watch
set INC=.
cl /W4 /MD /EHa /O2 /wd4819 /DDEBUG_STOP_WATCH /I%INC% %SRC%.cxx /Fe%TGT%
mt -manifest %TGT%.exe.manifest -outputresource:%TGT%.exe;1
del %SRC%.obj %TGT%.exe.manifest
# --- rhel4 (32bits)  :109,110 w! tes_u32_stop_watch.csh
g++ -Wall -O2 -g -DDEBUG_STOP_WATCH -I. igs_resource_stop_watch.cxx -o tes_u32_stop_watch
*/
