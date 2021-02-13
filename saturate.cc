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
pool<> tmps;

w tmp(w op, const vec<w> &v) {
  auto n = v.n;
  auto r = (compound *)tmps.alloc(offsetof(compound, v) + (n + 1) * sizeof op);
  r->n = n + 1;
  r->v[0] = op;
  memcpy(r->v + 1, v.p, n * sizeof op);
  return tag(r, a_compound);
}
} // namespace

w saturate() {
  while (!passive.empty())
    passive.pop();
  for (auto c : clauses)
    passive.push(c);
  while (!passive.empty()) {
    auto g = passive.top();
    passive.pop();
    if (!g->n)
      return s_Unsatisfiable;
    tmps.clear();
  }
  return complete ? s_Satisfiable : s_GaveUp;
}
