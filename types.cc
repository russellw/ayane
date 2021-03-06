#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

ary<tcompound *> tcompounds(0);

namespace {
// the number of types is expected to be small. it is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
int atoms = nbasictypes;

int cap = 0x10;
uint16_t *entries = (uint16_t *)xcalloc(cap, sizeof *entries);

bool eq(const tcompound *t, const uint16_t *p, int n) {
  if (t->n != n)
    return 0;
  return !memcmp(t->v, p, n * sizeof *p);
}

int slot(uint16_t *entries, int cap, const uint16_t *p, int n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof *p) & mask;
  while (entries[i] && !eq(tcompounds[entries[i]], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (uint16_t *)xcalloc(cap1, sizeof *entries);
  for (int i = 0; i != cap; ++i) {
    auto t = entries[i];
    if (t) {
      auto tp = tcompounds[t];
      entries1[slot(entries1, cap1, tp->v, tp->n)] = t;
    }
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

tcompound *store(const uint16_t *p, int n) {
  auto r = (tcompound *)mmalloc(offsetof(tcompound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

w put(const uint16_t *p, int n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i] | t_compound;
  if (tcompounds.n >= cap * 3 / 4) {
    if (tcompounds.n >= t_compound)
      err("too many compound types");
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = tcompounds.n;
  entries[i] = t;
  tcompounds.push(store(p, n));
  return t | t_compound;
}
} // namespace

// SORT
void defaulttype(w t, w a) {
  assert(t < t_compound);
  switch (a & 7) {
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) != a_sym)
      break;
    auto ft = symp(op)->ft;
    if (ft)
      break;
    auto n = size(a);
    vec<uint16_t> v;
    v.resize(n);
    v[0] = t;
    for (int i = 1; i != n; ++i)
      v[i] = typeof(at(a, i));
    symp(op)->ft = type(v);
    break;
  }
  case a_sym:
    if (!symp(a)->ft)
      symp(a)->ft = t;
    break;
  }
}

w numtype(w a) {
  auto t = typeof(a);
  switch (t) {
  case t_int:
  case t_rat:
  case t_real:
    return t;
  }
  throw "expected number term";
}

void requiretype(w t, w a) {
  defaulttype(t, a);
  if (t != typeof(a))
    throw "type mismatch";
}

w type(const vec<uint16_t> &v) { return put(v.p, v.n); }

w type(sym *name) {
  if (name->t)
    return name->t;
  if (atoms >= t_compound)
    err("too many atomic types");
  return name->t = atoms++;
}

w type(w r, w t1) {
  uint16_t v[2];
  v[0] = r;
  v[1] = t1;
  return put(v, sizeof v / sizeof *v);
}

w typeof(w a) {
  switch (a & 7) {
  case a_basic:
    assert(a >> 3 == b_false || a >> 3 == b_true);
    return t_bool;
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
        return t_bool;
      case b_toint:
        return t_int;
      case b_torat:
        return t_rat;
      case b_toreal:
        return t_real;
      }
      return typeof(at(a, 1));
    case a_sym: {
      auto ft = symp(op)->ft;
      if (!ft)
        return 0;
      assert(ft & t_compound);
      auto ftp = tcompoundp(ft);
      assert(size(a) == ftp->n);
      return ftp->v[0];
    }
    }
    unreachable;
  }
  case a_distinctobj:
    return t_individual;
  case a_int:
    return t_int;
  case a_rat:
    return t_rat;
  case a_real:
    return t_real;
  case a_sym:
    return symp(a)->ft;
  case a_var:
    return vartype(a);
  }
  unreachable;
  return 0;
}
///
