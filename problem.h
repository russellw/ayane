enum class szs {
  none,
#define X(s) s,
#include "szs.h"
#undef X
  max
};

extern const char *szs_names[];

// SORT
extern bool complete;
extern time_t deadline;
extern vec<clause *> clauses;
///

#ifdef DEBUG
extern szs expected;
#endif

void init_problem();
clause *addclause(w infer);
