#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
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
  }
  return complete ? s_Satisfiable : s_GaveUp;
}
