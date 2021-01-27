// current file
extern const char *file;
// beginning of source text
extern const char *src0;
// current position in source text
extern const char *src;

// current token
extern const char *tok0;
extern int tok;
extern vec<char> buf;
extern sym *tokSym;

struct File {
  // remember previous file
  // for nested includes
  // and so that after parsing is done
  // error reporting will know we are no longer in a file
  const char *old_file;
  const char *old_src0;
  const char *old_src;

  File(const char *file);
  ~File();
};

// SZS status codes
enum {
#define _(s) s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

// metadata
extern bool conjecture;
#ifdef DEBUG
extern int status;
#endif

// error
#ifdef _MSC_VER
__declspec(noreturn)
#endif
    void err(const char *s);
