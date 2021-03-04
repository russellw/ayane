#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *szs[] = {
    0,
#define X(s) #s,
#include "szs.h"
#undef X
};

// SORT
bool complete;
time_t deadline;
vec<clause *> clauses;
///

#ifdef DEBUG
w expected;
#endif

void init_problem() {
  clauses.n = 0;
  complete = 1;
#ifdef DEBUG
  expected = 0;
#endif
}

clause *addclause(w infer) {
  auto c = mkclause(infer);
  if (c)
    clauses.push(c);
  return c;
}
