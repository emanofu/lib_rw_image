#ifndef igs_resource_ofbinary_h
#define igs_resource_ofbinary_h
/* --- バイナリファイルの最高速(!?)一括読み込みクラス - */

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
	void close(void); /* 必ず呼ぶこと */
	~ofbinary();/* 自動でclose()を呼ぶがエラーは無効化してるので注意 */
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
	void close(void); /* 必ず呼ぶこと */
	~ofbinary();/* 自動でclose()を呼ぶがエラーは無効化してるので注意 */
  private:
	ofbinary();
	int fd_;
  };
 }
}
#endif //-------------------------------------------------------------

#endif /* !igs_resource_ofbinary_h */
