#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

ary<pair<w, w>> unified;

bool match(w a, w b) {
  // equal
  if (a == b)
    return 1;

  // type mismatch
  if (typeof(a) != typeof(b))
    return 0;

  // variable
  if ((a & 7) == a_var) {
    // existing mapping
    for (auto p : unified) {
      if (p.first == a)
        return p.second == b;
    }

    // new mapping
    unified.push(make_pair(a, b));
    return 1;
  }

  // atoms
  if ((a & 7) != a_compound)
    return 0;
  if ((b & 7) != a_compound)
    return 0;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return 0;
  if (at(a, 0) != at(b, 0))
    return 0;
  for (w i = 1; i != n; ++i)
    if (!match(at(a, i), at(b, i)))
      return 0;
  return 1;
}

namespace {
bool occurs(w a, w b) {
  assert((a & 7) == a_var);
  switch (b & 7) {
  case a_compound: {
    auto n = size(b);
    for (w i = 0; i != n; ++i)
      if (occurs(a, at(b, i)))
        return 1;
    break;
  }
  case a_var:
    if (a == b)
      return 1;
    for (auto p : unified)
      if (p.first == b)
        return occurs(a, p.second);
    break;
  }
  return 0;
}

bool unifyvar(w a, w b) {
  assert((a & 7) == a_var);
  assert(typeof(a) == typeof(b));

  // existing mappings
  for (auto p : unified) {
    if (p.first == a)
      return unify1(p.second, b);
    if (p.first == b)
      return unify1(a, p.second);
  }

  // occurs check
  if (occurs(a, b))
    return 0;

  // new mapping
  unified.push(make_pair(a, b));
  return 1;
}
} // namespace

bool unify1(w a, w b) {
  // equal
  if (a == b)
    return 1;

  // type mismatch
  if (typeof(a) != typeof(b))
    return 0;

  // variables
  if ((a & 7) == a_var)
    return unifyvar(a, b);
  if ((b & 7) == a_var)
    return unifyvar(b, a);

  // atoms
  if ((a & 7) != a_compound)
    return 0;
  if ((b & 7) != a_compound)
    return 0;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return 0;
  if (at(a, 0) != at(b, 0))
    return 0;
  for (w i = 1; i != n; ++i)
    if (!unify1(at(a, i), at(b, i)))
      return 0;
  return 1;
}

bool unify(w a, w b) {
  unified.n = 0;
  return unify1(a, b);
}

w replace(w a) {
  switch (a & 7) {
  case a_compound: {
    auto n = size(a);
    vec<w> v;
    v.resize(n);
    for (w i = 0; i != n; ++i)
      v[i] = replace(at(a, i));
    return term(v);
  }
  case a_var:
    for (auto p : unified)
      if (p.first == a)
        return replace(p.second);
    break;
  }
  return a;
}
