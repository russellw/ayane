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
  case a_compound: {
    auto p = compoundp(a);
    auto n = p->n;
    auto q = (compound *)alts.alloc(offsetof(compound, v) + n * sizeof a);
    q->n = p->n;
    for (w i = 0; i != n; ++i)
      q->v[i] = altvars(p->v[i]);
    return tag(q, a_compound);
  }
  case a_var:
    return a | altvar;
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

// make a clause and if successful (not a tautology or duplicate) add it to the
// passive queue
void qclause(w infer) {
  auto c = mkclause(infer);
  if (c)
    passive.push(c);
}

// inputs
clause *c;
w ci;
w c0, c1;

clause *d;
w di;
w d0, d1;

// substitute and make new clause
void resolve1() {
  assert(!neg.n);
  for (w i = 0, e = c->nn; i != e; ++i)
    if (i != ci)
      neg.push(replace(c->v[i]));

  assert(!pos.n);
  for (w i = c->nn, e = c->n; i != e; ++i)
    pos.push(replace(c->v[i]));

  qclause(0);
}

// for each negative equation
void resolve() {
  for (w i = 0, e = c->nn; i != e; ++i) {
    eqn ce(c->v[i]);
    if (unify0(ce.left, ce.right)) {
      ci = i;
    }
  }
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
loop:
  while (!passive.empty()) {
    // given clause
    auto g = passive.top();
    passive.pop();

    // empty clause = derived false = unsatisfiable
    if (!g->n)
      return s_Unsatisfiable;

    // alternate variables
    alts.init();
    auto c = altvars(g);

    // this is the Discount loop (in which only active clauses participate in
    // subsumption checks); in tests, it performed slightly better than the
    // alternative Otter loop (in which passive clauses also participate)

    // forward subsumption
    for (auto d : active) {
      if (d->subsumed)
        continue;
      if (subsumes(d, c))
        goto loop;
    }

    // backward subsumption
    for (auto d : active) {
      if (d->subsumed)
        continue;
      if (subsumes(c, d))
        d->subsumed = 1;
    }

    // add g to active clauses before inference because we will sometimes need
    // to combine g with (the alternate variable version of) itself
    active.push(g);
  }

  // if a complete saturation proof procedure finds no more possible
  // derivations, then the problem is satisfiable; in practice, this almost
  // never happens for nontrivial problems, but serves as a good way to test the
  // completeness of the prover on some trivial problems. however, if
  // completeness was lost for any reason, such as having to discard some
  // clauses because they were too big, then report failure
  return complete ? s_Satisfiable : s_GaveUp;
}
