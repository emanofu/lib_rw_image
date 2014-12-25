#ifndef igs_resource_ifbinary_h
#define igs_resource_ifbinary_h
/* --- �Х��ʥ�ե�����κǹ�®(!?)����ɤ߹��ߥ��饹 - */

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  /* ------ �ե������binary���ɤ߹��� ------ */
  class IGS_RESOURCE_IRW_EXPORT ifbinary {
  public: explicit ifbinary(LPCTSTR file_path);
	void read(void *buffer,const size_t byte_size);
	void close(void); /* ɬ���Ƥ֤��� */
	~ifbinary();/* ��ư��close()��Ƥ֤����顼��̵�������Ƥ�Τ���� */
  private:
	ifbinary();
	HANDLE handle_;
  };
  /* ------ �ե����륵���������� ------ */
  IGS_RESOURCE_IRW_EXPORT const size_t bytes_of_file(LPCTSTR file_path);
 }
}
#else //--------------------------------------------------------------
namespace igs {
 namespace resource {
  /* ------ �ե������binary���ɤ߹��� ------ */
  class ifbinary {
  public: explicit ifbinary(const char *file_path);
	void read(void *buffer,const size_t byte_size);
	void close(void); /* ɬ���Ƥ֤��� */
	~ifbinary();/* ��ư��close()��Ƥ֤����顼��̵�������Ƥ�Τ���� */
  private:
	ifbinary();
	int fd_;
  };
  /* ------ �ե����륵���������� ------ */
  const size_t bytes_of_file(const char *file_path);
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_ifbinary_h */
