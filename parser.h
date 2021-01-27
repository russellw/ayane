struct Parser {
  const char *file;
  const char *src0;
  const char *src;

  // current token
  const char *tok0;
  int tok;
  sym *tokSym;

  Parser(const char *file);
  ~Parser();

#ifdef _MSC_VER
  __declspec(noreturn)
#endif
      void err(const char *s);
};

// SZS status codes
enum {
#define _(s) s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

extern vec<char> buf;

// metadata
extern bool conjecture;
#ifdef DEBUG
extern int status;
#endif
