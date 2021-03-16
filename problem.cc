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
vec<clause *> inputClauses;
///

#ifdef DEBUG
szs expected;
#endif

void initProblem() {
  // SORT
  complete = 1;
  inputClauses.n = 0;
  ///
#ifdef DEBUG
  expected = szs::none;
#endif
}

clause *inputClause(infer inf) {
  auto c = internClause(inf);
  if (c)
    inputClauses.push_back(c);
  return c;
}
