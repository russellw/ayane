#include "stdafx.h"
// stdafx.h must be left
#include "main.h"

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

struct subsumption {
  w *cbegin;
  w *cend;
  w dbegin;
  w dend;
};

// multiset avoids breaking completeness when factoring is used
bool used[0xffff];

bool subsume(subsumption *first, w *ci, subsumption *second) {
  if (ci == first->cend) {
    if (second)
      return subsume(second, second->cbegin, 0);
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
  if (c->n > d->n)
    return 0;

  ::c = c;
  ::d = d;
  memset(used, 0, d->n);
  unified.n = 0;

  subsumption first;
  first.cbegin = c->v;
  first.cend = c->v + c->nn;
  first.dbegin = 0;
  first.dend = d->nn;

  subsumption second;
  second.cbegin = c->v + c->nn;
  second.cend = c->v + c->n;
  second.dbegin = d->nn;
  second.dend = d->n;

  return subsume(&first, first.cbegin, &second);
}
