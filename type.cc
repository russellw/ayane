#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

ary<tcompound *, 0x10000 - t_compound> tcompounds;

namespace {
// the number of types is expected to be small. it is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
si atoms = (si)type::max;

si cap = 0x10;
type *entries = (type *)xcalloc(cap, sizeof *entries);

bool eq(const tcompound *t, const type *p, si n) {
  if (t->n != n)
    return 0;
  return !memcmp(t->v, p, n * sizeof *p);
}

si slot(type *entries, si cap, const type *p, si n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof *p) & mask;
  while (entries[i] != type::none && !eq(tcompoundp(entries[i]), p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (type *)xcalloc(cap1, sizeof *entries);
  for (si i = 0; i != cap; ++i) {
    auto t = entries[i];
    if (t != type::none) {
      auto tp = tcompoundp(t);
      entries1[slot(entries1, cap1, tp->v, tp->n)] = t;
    }
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

tcompound *store(const type *p, si n) {
  auto r = (tcompound *)mmalloc(offsetof(tcompound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

type put(const type *p, si n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i] != type::none)
    return entries[i];
  if (tcompounds.n >= cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = entries[i] = type(t_compound + tcompounds.n);
  tcompounds.push(store(p, n));
  return t;
}
} // namespace

// SORT
void defaulttype(type t, w a) {
  assert(!iscompound(t));
  switch (a & 7) {
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) != a_sym)
      break;
    auto ft = symp(op)->ft;
    if (ft != type::none)
      break;
    auto n = size(a);
    vec<type> v;
    v.resize(n);
    v[0] = t;
    for (si i = 1; i != n; ++i)
      v[i] = typeof(at(a, i));
    symp(op)->ft = mktype(v);
    break;
  }
  case a_sym:
    if (symp(a)->ft == type::none)
      symp(a)->ft = t;
    break;
  }
}

type mktype(const vec<type> &v) { return put(v.p, v.n); }

type mktype(sym *name) {
  if (name->t != type::none)
    return name->t;
  if (atoms >= t_compound)
    err("too many atomic types");
  return name->t = (type)atoms++;
}

type mktype(type r, type t1) {
  type v[2];
  v[0] = r;
  v[1] = t1;
  return put(v, sizeof v / sizeof *v);
}

type numtype(w a) {
  auto t = typeof(a);
  switch (t) {
  case type::Int:
  case type::Rat:
  case type::Real:
    return t;
  }
  throw "expected number term";
}

void requiretype(type t, w a) {
  defaulttype(t, a);
  if (t != typeof(a))
    throw "type mismatch";
}

type typeof(w a) {
  switch (a & 7) {
  case a_basic:
    assert(a >> 3 == b_false || a >> 3 == b_true);
    return type::Bool;
  case a_compound: {
    auto op = at(a, 0);
    switch (op & 7) {
    case a_basic:
      switch (op >> 3) {
      case b_all:
      case b_and:
      case b_eq:
      case b_eqv:
      case b_exists:
      case b_isint:
      case b_israt:
      case b_le:
      case b_lt:
      case b_not:
      case b_or:
        return type::Bool;
      case b_toint:
        return type::Int;
      case b_torat:
        return type::Rat;
      case b_toreal:
        return type::Real;
      }
      return typeof(at(a, 1));
    case a_sym: {
      auto ft = symp(op)->ft;
      if (ft == type::none)
        return ft;
      assert(iscompound(ft));
      auto ftp = tcompoundp(ft);
      assert(size(a) == ftp->n);
      return ftp->v[0];
    }
    }
    unreachable;
  }
  case a_distinctobj:
    return type::Individual;
  case a_int:
    return type::Int;
  case a_rat:
    return type::Rat;
  case a_real:
    return type::Real;
  case a_sym:
    return symp(a)->ft;
  case a_var:
    return vartype(a);
  }
  unreachable;
  return type::none;
}
///
