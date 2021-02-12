#include "stdafx.h"
// stdafx.h must be left
#include "main.h"

namespace {
struct eqn {
  w left, right;

  eqn(w a) {
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

bool match(const eqn &a, const eqn &b) {
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

clause *c1;
w *cend;

clause *d1;
w dbegin;
w dend;

// multiset avoids breaking completeness when factoring is used
bool used[0xffff];

bool subsume(w *ci) {
  if (ci == cend)
    return 1;
  auto a = eqn(*ci++);
  for (auto di = dbegin; di != dend; ++di) {
    if (used[di])
      continue;

    auto b = eqn(d1->v[di]);
    if (!match(a, b))
      continue;

    auto old = unified.n;
    used[di] = 1;
    if (subsume(ci))
      return 1;
    used[di] = 0;
    unified.n = old;
  }
  return 0;
}
} // namespace

bool subsumes(clause *c, clause *d) {
  if (c->n > d->n)
    return 0;

  c1 = c;
  d1 = d;
  memset(used, 0, d->n);
  unified.n = 0;

  cend = c->v + c->nn;
  dbegin = 0;
  dend = d->nn;
  if (!subsume(c->v))
    return 0;

  cend = c->v + c->n;
  dbegin = d->nn;
  dend = d->n;
  if (!subsume(c->v + c->nn))
    return 0;

  return 1;
}
