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
w *ci;
w c0, c1;

clause *d;
w *di;
w d0, d1;

/*
equality resolution
    c | c0 != c1
->
    c/s
where
    s = unify(c0, c1)
*/

// substitute and make new clause
void resolveq() {
  assert(!neg.n);
  for (auto i = c->v, e = c->v + c->nn; i != e; ++i)
    if (i != ci)
      neg.push(replace(*i));

  assert(!pos.n);
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i)
    pos.push(replace(*i));

  qclause(0);
}

// for each negative equation
void resolve() {
  for (auto i = c->v, e = c->v + c->nn; i != e; ++i) {
    eqn ce(*i);
    if (unify(ce.left, ce.right)) {
      ci = i;
      resolveq();
    }
  }
}

/*
equality factoring
    c | c0 = c1 | d0 = d1
->
    (c | c0 = c1 | c1 != d1)/s
where
    s = unify(c0, d0)
*/

bool equatable(w a, w b) {
  if (typeof(a) != typeof(b))
    return false;
  if (typeof(a) == t_bool)
    return a == basic(b_true) || b == basic(b_true);
  return false;
  return true;
}

w equate(w a, w b) {
  assert(equatable(a, b));
  if (a == basic(b_true))
    return b;
  if (b == basic(b_true))
    return a;
  return term(basic(b_eq), a, b);
}

// substitute and make new clause
void factorq() {
  if (!equatable(c1, d1))
    return;
  if (!unify(c0, d0))
    return;

  assert(!neg.n);
  for (auto i = c->v, e = c->v + c->nn; i != e; ++i)
    neg.push(replace(*i));
  neg.push(equate(replace(c1), replace(d1)));

  assert(!pos.n);
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i)
    if (i != di)
      pos.push(replace(*i));

  qclause(0);
}

// for each positive equation (both directions) again
void factor1() {
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i) {
    if (i == ci)
      continue;
    eqn de(*i);
    di = i;

    d0 = de.left;
    d1 = de.right;
    factorq();

    d0 = de.right;
    d1 = de.left;
    factorq();
  }
}

// for each positive equation (both directions)
void factor() {
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i) {
    eqn ce(*i);
    ci = i;

    c0 = ce.left;
    c1 = ce.right;
    factor1();

    c0 = ce.right;
    c1 = ce.left;
    factor1();
  }
}

/*
superposition
    c | c0 = c1, d | d0(x) ?= d1
->
    (c | d | d0(c1) ?= d1)/s
where
    s = unify(c0, x)
    x not a variable
*/

// substitute and make new clause
void superpositionq(w d0c1) {
  assert(!neg.n);
  for (auto i = c->v, e = c->v + c->nn; i != e; ++i)
    neg.push(replace(*i));
  for (auto i = d->v, e = d->v + d->nn; i != e; ++i)
    if (i != di)
      neg.push(replace(*i));

  assert(!pos.n);
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i)
    if (i != ci)
      pos.push(replace(*i));
  for (auto i = d->v + d->nn, e = d->v + d->n; i != e; ++i)
    if (i != di)
      pos.push(replace(*i));

  // negative and positive superposition
  auto &v = di < (d->v + d->nn) ? neg : pos;
  v.push(equate(replace(d0c1), replace(d1)));

  qclause(0);
}

vec<w> position;

inline w *beginp(w a) {
  auto p = compoundp(a);
  return p->v;
}

inline w *endp(w a) {
  auto p = compoundp(a);
  return p->v + p->n;
}

w splice(w x, w *i) {
  if (i == position.end())
    return c1;
  assert((x & 7) == a_compound);
  vec<w> v;
  v.insert(v.p, beginp(x), endp(x));
  auto j = *i++;
  v[j] = splice(v[j], i);
  return term(v);
}

void descend(w x) {
  if ((x & 7) == a_var)
    return;
  if (unify(c0, x))
    superpositionq(splice(d0, position.p));
  if ((x & 7) == a_compound)
    for (w j = 1, e = size(x); j != e; ++j) {
      position.push(j);
      descend(at(x, j));
      --position.n;
    }
}

// for each equation in d (both directions)
void superposition2() {
  if (c0 == basic(b_true))
    return;
  for (auto i = d->v, e = d->v + d->n; i != e; ++i) {
    auto de = eqn(*i);
    di = i;

    d0 = de.left;
    d1 = de.right;
    position.n = 0;
    descend(d0);

    d0 = de.right;
    d1 = de.left;
    position.n = 0;
    descend(d0);
  }
}

// for each positive equation in c (both directions)
void superposition1() {
  for (auto i = c->v + c->nn, e = c->v + c->n; i != e; ++i) {
    auto ce = eqn(*i);
    ci = i;

    c0 = ce.left;
    c1 = ce.right;
    superposition2();

    c0 = ce.right;
    c1 = ce.left;
    superposition2();
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
