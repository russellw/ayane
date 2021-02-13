#include "stdafx.h"
// stdafx.h must be left
#include "main.h"

// these are declared static rather than placed in the anonymous namespace
// because they will need to be disambiguated with the :: global scope operator
static clause *c;
static clause *d;

namespace {
struct eqn {
  w left, right;

  explicit eqn(w a) {
    assert(typeof(a) == t_bool);
    if ((a & 7) == a_compound && at(a, 0) == basic(b_eq)) {
      left = at(a, 1);
      right = at(a, 2);
    } else {
      left = a;
      right = basic(b_true);
    }
    assert(typeof(left) == typeof(right));
  }
};

bool matche(const eqn &a, const eqn &b) {
  if (typeof(a.left) != typeof(a.right))
    return 0;

  auto old = unified.n;

  if (match(a.left, b.left) && match(a.right, b.right))
    return 1;
  unified.n = old;

  if (match(a.left, b.right) && match(a.right, b.left))
    return 1;
  unified.n = old;

  return 0;
}

// subsumption of clauses breaks down into two subsumption problems, one for
// negative literals and one for positive. this data structure records one
// subsumption problem
struct subsumption {
  // for efficiency, refer to the subsuming clause literals directly with
  // pointers
  w *cbegin;
  w *cend;
  // refer to the subsumed clause literals with array indexes because we will
  // also need to index the array of flags recording which subsumed literals
  // have been used
  w dbegin;
  w dend;
};

// multiset avoids breaking completeness when factoring is used
bool used[0xffff];

bool subsume(subsumption *first, w *ci, subsumption *second) {
  if (ci == first->cend) {
    // fully subsumed one side
    // have we done the other side yet?
    if (second)
      return subsume(second, second->cbegin, 0);
    // if so, we are done
    return 1;
  }
  auto a = eqn(*ci++);
  for (auto di = first->dbegin; di != first->dend; ++di) {
    if (used[di])
      continue;
    auto b = eqn(d->v[di]);
    auto old = unified.n;
    if (!matche(a, b))
      continue;
    used[di] = 1;
    if (subsume(first, ci, second))
      return 1;
    unified.n = old;
    used[di] = 0;
  }
  return 0;
}
} // namespace

bool subsumes(clause *c, clause *d) {
  // it is impossible for a larger clause to subsume a smaller one
  if (c->nn > d->nn || c->np() > d->np())
    return 0;

  // initialize
  ::c = c;
  ::d = d;
  memset(used, 0, d->n);
  unified.n = 0;

  // negative literals
  subsumption first;
  first.cbegin = c->v;
  first.cend = c->v + c->nn;
  first.dbegin = 0;
  first.dend = d->nn;
  auto firstp = &first;

  // positive literals
  subsumption second;
  second.cbegin = c->v + c->nn;
  second.cend = c->v + c->n;
  second.dbegin = d->nn;
  second.dend = d->n;
  auto secondp = &second;

  // fewer literals are likely to fail faster, so if there are fewer positive
  // literals than negative, then swap them around and try the positive side
  // first
  if (d->np() < d->nn) {
    auto t = firstp;
    firstp = secondp;
    secondp = t;
  }

  // begin the search
  return subsume(firstp, firstp->cbegin, secondp);
}
