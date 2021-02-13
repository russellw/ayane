enum {
  s_none,
#define _(s) s_##s,
#include "szs.h"
#undef _
  n_szs
};

extern bool complete;
extern const char *szs[];

#ifdef DEBUG
extern w status;
#endif

void init_problem();
