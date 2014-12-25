#ifndef igs_sgi_extensions_h
#define igs_sgi_extensions_h

namespace igs {
 namespace sgi {
  namespace extension {
	enum type {
		sgi = 0,
		rgb,
		// rgba,
		// bw,
	};
  }
  const char *const extensions[] = {
		"sgi",
		"rgb",
		// "rgba", /* 4 channel image with alhpa */
		// "bw",   /* 1 channel grayscale image */
  };
 }
}

#endif /* !igs_sgi_extensions_h */
