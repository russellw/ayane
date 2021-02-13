#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
// passive clauses are stored in a priority queue with smaller clauses first
w weight(w a) {
  if ((a & 7) == a_compound) {
    auto p = compoundp(a);
    w n = 0;
    for (auto i = p->v, e = p->v + p->n; i != e; ++i)
      n += weight(*i);
    return n;
  }
  return 1;
}

w weight(clause *c) {
  w n = 0;
  for (auto i = c->v, e = c->v + c->n; i != e; ++i)
    n += weight(*i);
  return n;
}

struct cmp {
  bool operator()(clause *c, clause *d) { return weight(c) > weight(d); }
};

priority_queue<clause *, vector<clause *>, cmp> passive;

// unification and matching procedures assume the two sides have distinct
// variable names, so when a given clause is selected, it must go through
// variable renaming before these procedures are applied. it would have been
// possible to redesign the procedures to eliminate the need for this, but the
// variable renaming makes the rest of the code simpler, and because it only
// needs to be done once each time around the outer loop, it is probably also
// more efficient
const w altvar = (w)1 << (16 + 3);
pool<> alts;

w altvars(w a) {
  switch (a & 7) {
  case a_var:
    return a | altvar;
  case a_compound: {
    auto p = compoundp(a);
    auto n = p->n;
    auto q = (compound *)alts.alloc(offsetof(compound, v) + n * sizeof a);
    q->n = p->n;
    for (w i = 0; i != n; ++i)
      q->v[i] = altvars(p->v[i]);
    return tag(q, a_compound);
  }
  }
  return a;
}

clause *altvars(clause *c) {
  auto n = c->n;
  auto d = (clause *)alts.alloc(offsetof(clause, v) + n * sizeof(w));
  memcpy(d, c, offsetof(clause, v));
  for (w i = 0; i != n; ++i)
    d->v[i] = altvars(c->v[i]);
  return d;
}
} // namespace

w saturate() {
  // passive clauses
  while (!passive.empty())
    passive.pop();
  for (auto c : clauses)
    passive.push(c);

  // active clauses
  vec<clause *> active;

  // saturation proof procedure tries to perform all possible derivations until
  // it derives false
  while (!passive.empty()) {
    // given clause
    auto g = passive.top();
    passive.pop();

    // empty clause = derived false = unsatisfiable
    if (!g->n)
      return s_Unsatisfiable;

    // alternate variables
    alts.clear();
    auto c = altvars(g);
  }

  // if a complete saturation proof procedure finds no more possible
  // derivations, then the problem is satisfiable; in practice, this almost never
  // happens for nontrivial problems, but serves as a good way to test the
  // completeness of the prover on some trivial problems. however, if
  // completeness was lost for any reason, such as having to discard some clauses
  // because they were too big, then report failure
  return complete ? s_Satisfiable : s_GaveUp;
}
