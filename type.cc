#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

ary<tcompound *, 0x10000 - tcompoundoffset> tcompounds;

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
  auto t = entries[i] = type(tcompoundoffset + tcompounds.n);
  tcompounds.push(store(p, n));
  return t;
}
} // namespace

type mktype(sym *name) {
  if (name->t != type::none)
    return name->t;
  if (atoms >= tcompoundoffset)
    err("too many atomic types");
  return name->t = (type)atoms++;
}

type mktype(const vec<type> &v) {
  if (v.n > 0xffff)
    throw "type too complex";
  return put(v.p, v.n);
}

type mktype(type rt, type param1) {
  type v[2];
  v[0] = rt;
  v[1] = param1;
  return put(v, sizeof v / sizeof *v);
}

void defaulttype(type t, term a) {
  assert(!iscompound(t));
  switch (tag(a)) {
  case term::Call: {
    auto op = at(a, 0);
    assert(tag(op) == term::Sym);
    auto s = (sym *)rest(op);
    auto ft = s->ft;
    if (ft != type::none)
      break;
    auto n = size(a);
    vec<type> v;
    v.resize(n);
    v[0] = t;
    for (si i = 1; i != n; ++i) {
      auto u = typeof(at(a, i));
      assert(u != type::none);
      v[i] = u;
    }
    s->ft = mktype(v);
    break;
  }
  case term::Sym: {
    auto s = (sym *)rest(a);
    if (s->ft == type::none)
      s->ft = t;
    break;
  }
  }
}

void requiretype(type t, term a) {
  defaulttype(t, a);
  if (t != typeof(a))
    throw "type mismatch";
}

type typeof(term a) {
  switch (tag(a)) {
  case term::All:
  case term::And:
  case term::Eq:
  case term::Eqv:
  case term::Exists:
  case term::IsInt:
  case term::IsRat:
  case term::Le:
  case term::Lt:
  case term::Not:
  case term::Or:
    return type::Bool;
  case term::ToInt:
    return type::Int;
  case term::ToRat:
    return type::Rat;
  case term::ToReal:
    return type::Real;
  case term::False:
  case term::True:
    return type::Bool;
  case term::Call: {
    auto op = at(a, 0);
    assert(tag(op) == term::Sym);
    auto s = (sym *)rest(op);
    auto ft = s->ft;
    if (ft == type::none)
      return ft;
    assert(iscompound(ft));
    auto ftp = tcompoundp(ft);
    assert(size(a) == ftp->n);
    return ftp->v[0];
  }
  case term::DistinctObj:
    return type::Individual;
  case term::Int:
    return type::Int;
  case term::Rat:
    return type::Rat;
  case term::Real:
    return type::Real;
  case term::Sym:
    return ((sym *)rest(a))->ft;
  case term::Var:
    return vartype(a);
  }
  assert(iscompound(a));
  return typeof(at(a, 0));
}

type numtypeof(term a) {
  auto t = typeof(a);
  switch (t) {
  case type::Int:
  case type::Rat:
  case type::Real:
    return t;
  }
  throw "expected number term";
}
