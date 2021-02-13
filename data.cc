#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace syms {
// must be a power of 2, and large enough to hold the largest collection of
// entries that will be loaded at initialization time
w cap = 0x100;
w count;
sym **entries = (sym **)xcalloc(cap, sizeof(sym *));

bool strmemeq(const char *s, const char *p, w n) {
  while (n--)
    if (*s++ != *p++)
      return 0;
  return !*s;
}

w slot(sym **entries, w cap, const char *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n) & mask;
  while (entries[i] && !strmemeq(entries[i]->v, p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (sym **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto s = entries[i];
    if (s)
      entries1[slot(entries1, cap1, s->v, strlen(s->v))] = s;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

struct init {
  init() {
    for (w i = 0; i != nkeywords; ++i) {
      auto s = keywords + i;
      assert(strlen(s->v) < sizeof s->v);
      ++count;
      assert(count <= cap * 3 / 4);
      auto j = slot(entries, cap, s->v, strlen(s->v));
      assert(!entries[j]);
      entries[j] = s;
    }
  }
} init1;

sym *store(const char *s, w n) {
  auto r = (sym *)xmalloc(offsetof(sym, v) + n + 1);
  memset(r, 0, offsetof(sym, v));
  memcpy(r->v, s, n);
  r->v[n] = 0;
  return r;
}

sym *put(const char *p, w n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  return entries[i] = store(p, n);
}
} // namespace syms

namespace nums {
template <class T> class bank {
  w cap = 0x10;
  w count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  w slot(T **entries, w cap, const T &x) {
    auto mask = cap - 1;
    auto i = x.hash() & mask;
    while (entries[i] && !entries[i]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    assert(ispow2(cap));
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof *entries);
    for (w i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, *x)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *store(const T &x) {
    auto r = (T *)xmalloc(sizeof x);
    *r = x;
    return r;
  }

public:
  T *put(T &x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x.clear();
      return entries[i];
    }
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, x);
    }
    return entries[i] = store(x);
  }
};

bank<Int> ints;
bank<Rat> rats;
} // namespace nums

namespace compounds {
w cap = 0x1000;
w count;
compound **entries = (compound **)xcalloc(cap, sizeof(compound *));

bool eq(const compound *x, const w *p, w n) {
  if (x->n != n)
    return 0;
  return !memcmp(x->v, p, n * sizeof *p);
}

w slot(compound **entries, w cap, const w *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof *p) & mask;
  while (entries[i] && !eq(entries[i], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (compound **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto x = entries[i];
    if (x)
      entries1[slot(entries1, cap1, x->v, x->n)] = x;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

compound *store(const w *p, w n) {
  auto r = (compound *)xmalloc(offsetof(compound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

w put(const w *p, w n) {
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

// SORT
ary<w> freevars;
w skolemi;
///

namespace {
// SORT
ary<w> boundvars;
pool<> tmps;
///

void getfree1(w a) {
  switch (a & 7) {
  case a_compound: {
    auto n = size(a);
    if (at(a, 0) == basic(b_all) || at(a, 0) == basic(b_exists)) {
      auto old = boundvars.n;
      for (w i = 2; i != n; ++i)
        boundvars.push(at(a, i));
      getfree1(at(a, 1));
      boundvars.n = old;
      break;
    }
    for (w i = 1; i != n; ++i)
      getfree1(at(a, i));
    break;
  }
  case a_var:
    if (find(boundvars.begin(), boundvars.end(), a) != boundvars.end())
      break;
    if (find(freevars.begin(), freevars.end(), a) != freevars.end())
      break;
    freevars.push(a);
    break;
  }
}
} // namespace

// SORT
void clear() {
  for (auto i = syms::entries, e = syms::entries + syms::cap; i != e; ++i) {
    auto s = *i;
    if (s)
      s->ft = 0;
  }
  skolemi = 0;
  tmps.clear();
}

void getfree(w a) {
  assert(!boundvars.n);
  freevars.n = 0;
  getfree1(a);
}

w imp(w a, w b) { return term(basic(b_or), term(basic(b_not), a), b); }

w int1(Int &x) { return tag(nums::ints.put(x), a_int); }

sym *intern(const char *s, w n) { return syms::put(s, n); }

w rat(Rat &x) { return tag(nums::rats.put(x), a_rat); }

w real(Rat &x) { return tag(nums::rats.put(x), a_real); }

w term(const ary<w> &v) { return compounds::put(v.p, v.n); }

w term(const vec<w> &v) { return compounds::put(v.p, v.n); }

w term(w op, w a) {
  w v[2];
  v[0] = op;
  v[1] = a;
  return compounds::put(v, sizeof v / sizeof *v);
}

w term(w op, w a, w b) {
  w v[3];
  v[0] = op;
  v[1] = a;
  v[2] = b;
  return compounds::put(v, sizeof v / sizeof *v);
}

w tmp(w op, const vec<w> &v) {
  auto n = v.n;
  auto r = (compound *)tmps.alloc(offsetof(compound, v) + (n + 1) * sizeof op);
  r->n = n + 1;
  r->v[0] = op;
  memcpy(r->v + 1, v.p, n * sizeof op);
  return tag(r, a_compound);
}
///

#ifdef DEBUG
namespace {
void ckptr(void *p) {
  assert(p);
  if (sizeof p > 4)
    assert((w)p < (w)1 << 50);
  *buf = *(char *)p;
}

void cktype(w t) {
  if (t >= t_compound) {
    auto p = tcompoundp(t);
    ckptr(p);
    auto n = p->n;
    assert(1 < n);
    assert(n < 1000000);
    for (w i = 0; i != n; ++i)
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

void ckterm(w a) {
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
    for (w i = 1; i != n; ++i)
      ckterm(p->v[i]);
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
