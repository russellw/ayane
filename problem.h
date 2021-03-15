enum class szs {
  none,
#define X(s) s,
#include "szs.h"
  max
};

extern const char *szsNames[];

// SORT
extern bool complete;
extern time_t deadline;
extern vec<clause *> clauses;
///

#ifdef DEBUG
extern szs expected;
#endif

void initProblem();
clause *addClause(infer inf);
