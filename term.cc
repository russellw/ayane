#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

// temporary compound terms
pool<> tmppool;

// permanent/interned compound terms
namespace compounds {
si cap = 0x1000;
si count;
compound **entries = (compound **)xcalloc(cap, sizeof *entries);

bool eq(const compound *x, const term *p, si n) {
  if (x->n != n)
    return 0;
  return !memcmp(x->v, p, n * sizeof *p);
}

si slot(compound **entries, si cap, const term *p, si n) {
  auto mask = cap - 1;
  auto i = XXH64(p, n * sizeof *p, 0) & mask;
  while (entries[i] && !eq(entries[i], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (compound **)xcalloc(cap1, sizeof *entries);
  for (si i = 0; i != cap; ++i) {
    auto x = entries[i];
    if (x)
      entries1[slot(entries1, cap1, x->v, x->n)] = x;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

compound *store(const term *p, si n) {
  auto r = (compound *)xmalloc(offsetof(compound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

compound *put(const term *p, si n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  return entries[i] = store(p, n);
}
} // namespace compounds

term intern(term op, const ary<term> &args) {
  assert(op == tag(op));
  return tag(compounds::put(args.p, args.n), op);
}

term intern(term op, const vec<term> &args) {
  assert(op == tag(op));
  return tag(compounds::put(args.p, args.n), op);
}

term intern(term op, term a) {
  assert(op == tag(op));
  return tag(compounds::put(&a, 1), op);
}

term intern(term op, term a, term b) {
  assert(op == tag(op));
  term v[2];
  v[0] = a;
  v[1] = b;
  return tag(compounds::put(v, sizeof v / sizeof *v), op);
}

term intern(term op, term a, term b, term c) {
  assert(op == tag(op));
  term v[3];
  v[0] = a;
  v[1] = b;
  v[2] = c;
  return tag(compounds::put(v, sizeof v / sizeof *v), op);
}

term imp(term a, term b) { return intern(term::Or, intern(term::Not, a), b); }

// variables
namespace {
ary<term> boundvars;

void getfree1(term a) {
  switch (tag(a)) {
  case term::All:
  case term::Exists: {
    auto old = boundvars.n;
    for (auto i = begin(a) + 1, e = end(a); i != e; ++i)
      boundvars.push(*i);
    getfree1(at(a, 0));
    boundvars.n = old;
    return;
  }
  case term::Var:
    if (find(boundvars.p, boundvars.end(), a) != boundvars.end())
      return;
    if (find(termv.p, termv.end(), a) != termv.end())
      return;
    termv.push(a);
    return;
  }
  if (!iscompound(a))
    return;
  for (auto b : a)
    getfree1(b);
}
} // namespace

void getfree(term a) {
  assert(!boundvars.n);
  termv.n = 0;
  getfree1(a);
}

// etc
// SORT
ary<term> termv;
si skolemi;
///

void init_terms() { skolemi = 0; }

#ifdef DEBUG
namespace {
void ckptr(void *p) {
  // a valid pointer will not point to the first page of address space
  assert(0xfff < (si)p);

  // a valid pointer is unlikely to point past the first petabyte of address
  // space
  assert((uint64_t)p < (uint64_t)1 << 50);

  // a valid pointer should be aligned to 64 bits
  assert(!((si)p & 7));

  // testing the validity of a pointer by trying to read a byte is not
  // guaranteed to give useful diagnostics (if p is not in fact valid then it is
  // undefined behavior) but for debug-build checking code, heuristic usefulness
  // is all that's expected
  *buf = *(char *)p;
}

void cktype(type t) {
  if (!iscompound(t))
    return;
  auto p = tcompoundp(t);
  ckptr(p);
  auto n = p->n;
  assert(1 < n);
  for (si i = 0; i != n; ++i) {
    assert(p->v[i] != type::none);
    cktype(p->v[i]);
  }
}

void cksym(sym *s) {
  ckptr(s);
  assert(*s->v);
  assert(strlen(s->v) < sizeof buf);
  cktype(s->t);
  cktype(s->ft);
}
} // namespace

void ck(term a) {
  cktype(typeof(a));
  if (iscompound(a)) {
    auto n = size(a);
    assert(0 < n);
    assert(n < 1000000);
    for (auto b : a)
      ck(b);
  }
  switch (tag(a)) {
  case term::Add:
  case term::Div:
  case term::DivE:
  case term::DivF:
  case term::DivT:
  case term::Eq:
  case term::Eqv:
  case term::Le:
  case term::Lt:
  case term::Mul:
  case term::RemE:
  case term::RemF:
  case term::RemT:
  case term::Sub:
    assert(size(a) == 2);
    break;
  case term::Call:
    assert(1 < size(a));
    assert(tag(at(a, 0)) == term::Sym);
    break;
  case term::Ceil:
  case term::Floor:
  case term::IsInt:
  case term::IsRat:
  case term::Minus:
  case term::Not:
  case term::Round:
  case term::ToInt:
  case term::ToRat:
  case term::ToReal:
  case term::Trunc:
    assert(size(a) == 1);
    break;
  case term::DistinctObj:
  case term::Sym:
    cksym((sym *)rest(a));
    break;
  case term::Int:
    ckptr((Int *)rest(a));
    break;
  case term::Rat:
  case term::Real: {
    auto p = (Rat *)rest(a);
    ckptr(p);
    ckptr(mpq_numref(p->val));
    ckptr(mpq_denref(p->val));
    assert(mpz_cmp_ui(mpq_denref(p->val), 0) > 0);
    break;
  }
  case term::Var:
    assert(!iscompound(vartype(a)));
    assert(0 <= vari(a));
    assert(vari(a) < 1000000);
    break;
  }
}
#endif
