#ifndef igs_resource_ifbinary_h
#define igs_resource_ifbinary_h
/* --- バイナリファイルの最高速(!?)一括読み込みクラス - */

#if defined _WIN32 //-------------------------------------------------
# include <windows.h>
# ifndef IGS_RESOURCE_IRW_EXPORT
#  define IGS_RESOURCE_IRW_EXPORT
# endif
namespace igs {
 namespace resource {
  /* ------ ファイルをbinaryで読み込む ------ */
  class IGS_RESOURCE_IRW_EXPORT ifbinary {
  public: explicit ifbinary(LPCTSTR file_path);
	void read(void *buffer,const size_t byte_size);
	void close(void); /* 必ず呼ぶこと */
	~ifbinary();/* 自動でclose()を呼ぶがエラーは無効化してるので注意 */
  private:
	ifbinary();
	HANDLE handle_;
  };
  /* ------ ファイルサイズを得る ------ */
  IGS_RESOURCE_IRW_EXPORT const size_t bytes_of_file(LPCTSTR file_path);
 }
}
#else //--------------------------------------------------------------
namespace igs {
 namespace resource {
  /* ------ ファイルをbinaryで読み込む ------ */
  class ifbinary {
  public: explicit ifbinary(const char *file_path);
	void read(void *buffer,const size_t byte_size);
	void close(void); /* 必ず呼ぶこと */
	~ifbinary();/* 自動でclose()を呼ぶがエラーは無効化してるので注意 */
  private:
	ifbinary();
	int fd_;
  };
  /* ------ ファイルサイズを得る ------ */
  const size_t bytes_of_file(const char *file_path);
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_ifbinary_h */
