enum {
  s_none,
#define _(s) s_##s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

// SORT
extern bool complete;
extern vec<clause *> clauses;
///

#ifdef DEBUG
extern w expected;
#endif

void init_problem();
clause *addclause(w infer);
