#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *szsNames[] = {
    0,
#define X(s) #s,
#include "szs.h"
};

// SORT
bool complete;
time_t deadline;
vec<clause *> clauses;
///

#ifdef DEBUG
szs expected;
#endif

void initProblem() {
  clauses.n = 0;
  complete = 1;
#ifdef DEBUG
  expected = szs::none;
#endif
}

clause *addClause(infer inf) {
  auto c = internClause(inf);
  if (c)
    clauses.push_back(c);
  return c;
}
