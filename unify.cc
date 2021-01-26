#include "main.h"

namespace {
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
} // namespace

vec<std::pair<w, w>> unified;

bool unify1(w a, w b) {
  // Same term
  if (a == b)
    return true;

  // Type mismatch
  if (typeof(a) != typeof(b))
    return false;

  // Variables
  if ((a & 7) == a_var)
    return unify_var(a, b);
  if ((b & 7) == a_var)
    return unify_var(b, a);

  // Atoms
  if ((a & 7) != a_compound)
    return false;
  if ((b & 7) != a_compound)
    return false;

  // Compounds
  auto n = size(a);
  if (n != size(b))
    return false;
  if (at(a, 0) != at(b, 0))
    return false;
  for (w i = 1; i != n; ++i)
    if (!unify1(at(a, i), at(b, i)))
      return false;
  return true;
}

bool unify(w a, w b) {
  unified.n = 0;
  return unify1(a, b);
}

w replace(w a) {
  switch (a & 7) {
  case a_var:
    for (auto p : unified)
      if (p.first == a)
        return replace(p.second);
    break;
  case a_compound: {
    auto n = size(a);
    vec<w> v;
    v.resize(n);
    for (w i = 0; i != n; ++i)
      v[i] = replace(at(a, i));
    return term(v);
  }
  }
  return a;
}
