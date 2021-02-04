#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

vec<std::pair<w, w>> unified;

bool match(w a, w b) {
  // equal
  if (a == b)
    return true;

  // type mismatch
  if (typeof(a) != typeof(b))
    return false;

  // variable
  if ((a & 7) == a_var) {
    // existing mapping
    for (auto p : unified) {
      if (p.first == a)
        return p.second == b;
    }

    // new mapping
    unified.push(std::make_pair(a, b));
    return true;
  }

  // atoms
  if ((a & 7) != a_compound)
    return false;
  if ((b & 7) != a_compound)
    return false;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return false;
  if (at(a, 0) != at(b, 0))
    return false;
  for (w i = 1; i != n; ++i)
    if (!match(at(a, i), at(b, i)))
      return false;
  return true;
}

namespace {
bool occurs(w a, w b) {
  assert((a & 7) == a_var);
  switch (b & 7) {
  case a_compound: {
    auto n = size(b);
    for (w i = 0; i != n; ++i)
      if (occurs(a, at(b, i)))
        return true;
    break;
  }
  case a_var:
    if (a == b)
      return true;
    for (auto p : unified)
      if (p.first == b)
        return occurs(a, p.second);
    break;
  }
  return false;
}

bool unifyvar(w a, w b) {
  assert((a & 7) == a_var);
  assert(typeof(a) == typeof(b));

  // existing mappings
  for (auto p : unified) {
    if (p.first == a)
      return unify(p.second, b);
    if (p.first == b)
      return unify(a, p.second);
  }

  // occurs check
  if (occurs(a, b))
    return false;

  // new mapping
  unified.push(std::make_pair(a, b));
  return true;
}
} // namespace

bool unify(w a, w b) {
  // equal
  if (a == b)
    return true;

  // type mismatch
  if (typeof(a) != typeof(b))
    return false;

  // variables
  if ((a & 7) == a_var)
    return unifyvar(a, b);
  if ((b & 7) == a_var)
    return unifyvar(b, a);

  // atoms
  if ((a & 7) != a_compound)
    return false;
  if ((b & 7) != a_compound)
    return false;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return false;
  if (at(a, 0) != at(b, 0))
    return false;
  for (w i = 1; i != n; ++i)
    if (!unify(at(a, i), at(b, i)))
      return false;
  return true;
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
