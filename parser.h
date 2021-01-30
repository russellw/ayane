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

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};

// SZS status codes
enum {
#define _(s) s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

inline bool isDigit(char c) { return '0' <= c && c <= '9'; }

// metadata
extern bool conjecture;
#ifdef DEBUG
extern int status;
#endif
