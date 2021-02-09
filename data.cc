#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace types {
// the number of types is expected to be small. it is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
w atoms = basic_types;

w cap = 0x10;
uint16_t *entries = (uint16_t *)xcalloc(cap, sizeof(uint16_t));

bool eq(const tcompound *t, const uint16_t *p, w n) {
  if (t->n != n)
    return 0;
  return !memcmp(t->v, p, n * sizeof *p);
}

w slot(uint16_t *entries, w cap, const uint16_t *p, w n) {
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
  for (w i = 0; i != cap; ++i) {
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

tcompound *store(const uint16_t *p, w n) {
  auto r = (tcompound *)xmalloc(offsetof(tcompound, v) + n * sizeof *p);
  r->n = n;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

w put(const uint16_t *p, w n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i] | t_compound;
  if (tcompounds.n >= cap * 3 / 4) {
    if (tcompounds.n >= t_compound)
      throw "too many compound types";
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = tcompounds.n;
  entries[i] = t;
  tcompounds.push(store(p, n));
  return t | t_compound;
}
} // namespace types

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

namespace clauses {
w cap = 0x1000;
w count;
clause **entries = (clause **)xcalloc(cap, sizeof(clause *));

bool eq(const clause *c, const w *p, w nn, w n) {
  if (c->nn != nn)
    return 0;
  if (c->n != n)
    return 0;
  return !memcmp(c->v, p, n * sizeof *p);
}

w slot(clause **entries, w cap, const w *p, w nn, w n) {
  auto mask = cap - 1;
  auto i = (fnv(p, n * sizeof *p) ^ nn) & mask;
  while (entries[i] && !eq(entries[i], p, nn, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (clause **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto c = entries[i];
    if (c)
      entries1[slot(entries1, cap1, c->v, c->nn, c->n)] = c;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

clause *store(const w *p, w nn, w n, clause *from, clause *from1) {
  auto r = (clause *)xmalloc(offsetof(clause, v) + n * sizeof *p);
  memset(r, 0, offsetof(clause, v));
  r->nn = nn;
  r->n = n;
  r->from[0] = from;
  r->from[1] = from1;
  memcpy(r->v, p, n * sizeof *p);
  return r;
}

clause *put(const w *p, w nn, w n, clause *from, clause *from1) {
  auto i = slot(entries, cap, p, nn, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, nn, n);
  }
  return entries[i] = store(p, nn, n, from, from1);
}
} // namespace clauses

// SORT
ary<w> freevars;
ary<w> neg, pos;
bool complete;
clause *conjecture;
unordered_map<clause *, sym *> clausenames;
vec<tcompound *> tcompounds(0);
w skolemi;
///

const char *szs[] = {
#define _(s) #s,
#include "szs.h"
#undef _
};

#ifdef DEBUG
w status;
#endif

namespace {
// SORT
ary<w> boundvars;
pool<> formulas;
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
clause *clause1(clause *from, clause *from1) {
  auto nn = neg.n;
  auto n = nn + pos.n;
  // must be <= 0xff
  w v[0xff];
  if (n > sizeof v / sizeof *v) {
    complete = 0;
    return 0;
  }
  memcpy(v, neg.p, nn * sizeof *v);
  memcpy(v + nn, pos.p, pos.n * sizeof *v);
  return clauses::put(v, nn, n, from, from1);
}

void clear() {
  for (auto i = syms::entries, e = syms::entries + syms::cap; i != e; ++i) {
    auto s = *i;
    if (s)
      s->ft = 0;
  }
  clausenames.clear();
  complete = 1;
  conjecture = 0;
  formulas.clear();
  skolemi = 0;
  tmps.clear();
#ifdef DEBUG
  status = 0;
#endif
}

clause *formula(w a, clause *from) {
  auto r = (clause *)formulas.alloc(offsetof(clause, v) + sizeof(w));
  memset(r, 0, offsetof(clause, v));
  r->fof = 1;
  r->nn = 0;
  r->n = 1;
  r->from[0] = from;
  r->from[1] = 0;
  r->v[0] = a;
  return r;
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

w type(const vec<uint16_t> &v) { return types::put(v.p, v.n); }

w type(sym *name) {
  if (name->t)
    return name->t;
  if (istcompound(types::atoms))
    throw "too many atomic types";
  return name->t = types::atoms++;
}

w type(w r, w t1) {
  uint16_t v[2];
  v[0] = r;
  v[1] = t1;
  return types::put(v, sizeof v / sizeof *v);
}
///
