#ifndef igs_resource_ofbinary_h
#define igs_resource_ofbinary_h
/* --- �Х��ʥ�ե�����κǹ�®(!?)����ɤ߹��ߥ��饹 - */

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  class IGS_RESOURCE_IRW_EXPORT ofbinary {
  public: explicit ofbinary(LPCTSTR file_path);
	void seek(const long offset);
	void write(const void *buffer,const size_t byte_size);
	void close(void); /* ɬ���Ƥ֤��� */
	~ofbinary();/* ��ư��close()��Ƥ֤����顼��̵�������Ƥ�Τ���� */
  private:
	ofbinary();
	HANDLE handle_;
  };
 }
}
#else //--------------------------------------------------------------
namespace igs {
 namespace resource {
  class ofbinary {
  public: explicit ofbinary(const char *file_path);
	void seek(const long offset);
	void write(const void *buffer,const size_t byte_size);
	void close(void); /* ɬ���Ƥ֤��� */
	~ofbinary();/* ��ư��close()��Ƥ֤����顼��̵�������Ƥ�Τ���� */
  private:
	ofbinary();
	int fd_;
  };
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_ofbinary_h */
