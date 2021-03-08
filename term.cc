#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

// SORT
ary<w> freevars;
si skolemi;
///

namespace compounds {
si cap = 0x1000;
si count;
compound **entries = (compound **)xcalloc(cap, sizeof *entries);

bool eq(const compound *x, const w *p, si n) {
  if (x->n != n)
    return 0;
  return !memcmp(x->v, p, n * sizeof *p);
}

w slot(compound **entries, si cap, const w *p, si n) {
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

compound *store(const w *p, si n) {
  auto r = (compound *)xmalloc(offsetof(compound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

w put(const w *p, si n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return tag(entries[i], a_compound);
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  entries[i] = store(p, n);
  return tag(entries[i], a_compound);
}
} // namespace compounds

namespace {
ary<w> boundvars;

void getfree1(w a) {
  switch (a & 7) {
  case a_compound: {
    auto e = endp(a);
    if (at(a, 0) == basic(b_all) || at(a, 0) == basic(b_exists)) {
      auto old = boundvars.n;
      for (auto i = beginp(a) + 2; i != e; ++i)
        boundvars.push(*i);
      getfree1(at(a, 1));
      boundvars.n = old;
      break;
    }
    for (auto i = beginp(a) + 1; i != e; ++i)
      getfree1(*i);
    break;
  }
  case a_var:
    if (find(boundvars.p, boundvars.end(), a) != boundvars.end())
      break;
    if (find(freevars.p, freevars.end(), a) != freevars.end())
      break;
    freevars.push(a);
    break;
  }
}
} // namespace

// SORT
void getfree(w a) {
  assert(!boundvars.n);
  freevars.n = 0;
  getfree1(a);
}

w imp(w a, w b) { return mk(basic(b_or), mk(basic(b_not), a), b); }

void init_terms() { skolemi = 0; }

w int1(Int &x) { return tag(intern(x), a_int); }

w mk(const ary<w> &v) { return compounds::put(v.p, v.n); }

w mk(const vec<w> &v) { return compounds::put(v.p, v.n); }

w mk(w op, w a) {
  w v[2];
  v[0] = op;
  v[1] = a;
  return compounds::put(v, sizeof v / sizeof *v);
}

w mk(w op, w a, w b) {
  w v[3];
  v[0] = op;
  v[1] = a;
  v[2] = b;
  return compounds::put(v, sizeof v / sizeof *v);
}

w rat(Rat &x) { return tag(intern(x), a_rat); }

w real(Rat &x) { return tag(intern(x), a_real); }
///

#ifdef DEBUG
namespace {
void ckptr(void *p) {
  assert(p);
  if (sizeof p > 4)
    assert((size_t)p < (size_t)1 << 50);
  *buf = *(char *)p;
}

void cktype(w t) {
  if (t >= t_compound) {
    auto p = tcompoundp(t);
    ckptr(p);
    auto n = p->n;
    assert(1 < n);
    for (si i = 0; i != n; ++i)
      cktype(p->v[i]);
    return;
  }
  assert(t);
}

void cksym(sym *s) {
  ckptr(s);
  assert(*s->v);
  assert(strlen(s->v) < sizeof buf);
  if (s->t)
    cktype(s->t);
  if (s->ft)
    cktype(s->ft);
}
} // namespace

void ck(w a) {
  cktype(typeof(a));
  switch (a & 7) {
  case a_basic:
    assert(a >> 3 <= b_true);
    return;
  case a_compound: {
    auto p = compoundp(a);
    ckptr(p);
    auto n = p->n;
    assert(1 < n);
    assert(n < 1000000);
    for (si i = 1; i != n; ++i)
      ck(p->v[i]);
    return;
  }
  case a_distinctobj:
    cksym(distinctobjp(a));
    return;
  case a_int:
    ckptr(intp(a));
    return;
  case a_rat:
  case a_real: {
    auto p = ratp(a);
    ckptr(p);
    ckptr(mpq_numref(p->val));
    ckptr(mpq_denref(p->val));
    assert(mpz_cmp_ui(mpq_denref(p->val), 0) > 0);
    return;
  }
  case a_sym:
    cksym(symp(a));
    return;
  case a_var: {
    assert(vartype(a) < t_compound);
    auto i = vari(a);
    assert(0 <= i);
    assert(i < 1000000);
    return;
  }
  }
}
#endif
