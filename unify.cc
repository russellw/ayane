#include "main.h"

namespace {
vec<std::pair<w, w>> unified;

bool occurs(w a, w b) {
  assert((a & 7) == a_var);
  switch (b & 7) {
  case a_var:
    if (a == b)
      return true;
    for (auto p : unified)
      if (p.first == b)
        return occurs(a, p.second);
    break;
  case a_compound: {
    auto n = size(b);
    for (w i = 0; i != n; ++i)
      if (occurs(a, at(b, i)))
        return true;
    break;
  }
  }
  return false;
}

bool unify1(w a, w b);

bool unify_var(w a, w b) {
  assert((a & 7) == a_var);
  assert(typeof(a) == typeof(b));

  for (auto p : unified) {
    if (p.first == a)
      return unify1(p.second, b);
    if (p.first == b)
      return unify1(a, p.second);
  }

  if (occurs(a, b))
    return false;

  unified.push(std::make_pair(a, b));
  return true;
}

bool unify1(w a, w b) {
  assert(typeof(a) == typeof(b));

  // same term
  if (a == b)
    return true;

  // variables
  if ((a & 7) == a_var)
    return unify_var(a, b);
  if ((b & 7) == a_var)
    return unify_var(b, a);

  // atoms
  if ((a & 7) != a_compound)
    return false;
  if ((b & 7) != a_compound)
    return false;

  // compound terms
  auto n = size(a);
  if (n != size(b))
    return false;
  for (w i = 0; i != n; ++i)
    if (!unify1(at(a, i), at(b, i)))
      return false;
  return true;
}
} // namespace

bool unify(w a, w b) {
  if (typeof(a) != typeof(b))
    return false;
  unified.n = 0;
  return unify1(a, b);
}

w subst(w a) {
  if ((a & 7) == a_var)
    for (auto p : unified)
      if (p.first == a)
        return subst(p.second);

  if ((a & 7) != a_compound)
    return a;

  auto n = size(a);
  vec<w> v(n);
  for (w i = 0; i != n; ++i)
    v[i] = subst(at(a, i));
  return term(v);
}
