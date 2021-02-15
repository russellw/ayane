#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *szs[] = {
    0,
#define _(s) #s,
#include "szs.h"
#undef _
};

// SORT
bool complete;
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
