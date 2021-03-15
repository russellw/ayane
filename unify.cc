#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

bool match(term a, term b) {
  // equal
  if (a == b)
    return 1;

  // type mismatch
  if (typeof(a) != typeof(b))
    return 0;

  // variable
  if (tag(a) == term::Var) {
    // existing mapping
    for (auto &i : pairv) {
      if (i.first == a)
        return i.second == b;
    }

    // new mapping
    pairv.push(make_pair(a, b));
    return 1;
  }

  // atoms
  if (!isCompound(a))
    return 0;
  if (tag(a) != tag(b))
    return 0;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return 0;
  for (si i = 0; i != n; ++i)
    if (!match(at(a, i), at(b, i)))
      return 0;
  return 1;
}

namespace {
bool occurs(term a, term b) {
  assert(tag(a) == term::Var);
  if (tag(b) == term::Var) {
    if (a == b)
      return 1;
    for (auto &i : pairv)
      if (i.first == b)
        return occurs(a, i.second);
  }
  if (!isCompound(b))
    return 0;
  for (auto x : b)
    if (occurs(a, x))
      return 1;
  return 0;
}

bool unifyVar(term a, term b) {
  assert(tag(a) == term::Var);
  assert(typeof(a) == typeof(b));

  // existing mappings
  for (auto &i : pairv) {
    if (i.first == a)
      return unifyMore(i.second, b);
    if (i.first == b)
      return unifyMore(a, i.second);
  }

  // occurs check
  if (occurs(a, b))
    return 0;

  // new mapping
  pairv.push(make_pair(a, b));
  return 1;
}
} // namespace

bool unifyMore(term a, term b) {
  // equal
  if (a == b)
    return 1;

  // type mismatch
  if (typeof(a) != typeof(b))
    return 0;

  // variables
  if (tag(a) == term::Var)
    return unifyVar(a, b);
  if (tag(b) == term::Var)
    return unifyVar(b, a);

  // atoms
  if (!isCompound(a))
    return 0;
  if (tag(a) != tag(b))
    return 0;

  // compounds
  auto n = size(a);
  if (n != size(b))
    return 0;
  for (si i = 0; i != n; ++i)
    if (!unifyMore(at(a, i), at(b, i)))
      return 0;
  return 1;
}

bool unify(term a, term b) {
  pairv.n = 0;
  return unifyMore(a, b);
}
