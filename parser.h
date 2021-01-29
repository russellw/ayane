struct Parser {
  const char *file;
  const char *textStart;
  const char *text;

  // current token
  const char *tokStart;
  int tok;
  sym *tokSym;

  Parser(const char *file);
  ~Parser();

#ifdef _MSC_VER
  __declspec(noreturn)
#endif
      void err(const char *msg, const char *ts);

#ifdef _MSC_VER
  __declspec(noreturn)
#endif
      void err(const char *msg) {
    err(msg, tokStart);
  }
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

inline bool isDigit(char c) { return '0' <= c && c <= '9'; }

// metadata
extern bool conjecture;
#ifdef DEBUG
extern int status;
#endif
