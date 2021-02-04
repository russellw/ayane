struct Parser {
  const char *file;
  const char *textStart;
  const char *text;

  // current token
  const char *tokStart;
  w tok;
  Sym *tokSym;

  Parser(const char *file);
  ~Parser();

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};

// SZS status codes
enum {
#define _(s) s_##s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

inline bool isdigit1(char c) { return '0' <= c && c <= '9'; }

// metadata
extern bool conjecture;
#ifdef DEBUG
extern w status;
#endif
