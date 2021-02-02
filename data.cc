#include "main.h"

template <class T> class bank {
  // Must be a power of 2
  w cap = 0x10;
  w count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  w slot(T **entries, w cap, T *x) {
    auto mask = cap - 1;
    auto i = x->hash() & mask;
    while (entries[i] && !entries[i]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof *entries);
    for (w i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, x)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *store(T *x) {
    auto r = (T *)xmalloc(sizeof *x);
    *r = *x;
    return r;
  }

public:
  T *put(T *x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x->clear();
      return entries[i];
    }
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, x);
    }
    return entries[i] = store(x);
  }
};

namespace types {
// The number of types is expected to be small. It is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. Compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
w atoms = basic_types;

w cap = 0x10;
uint16_t *entries = (uint16_t *)xcalloc(cap, sizeof(uint16_t));

bool eq(const TCompound *t, const uint16_t *p, w n) {
  if (t->n != n)
    return false;
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
  auto cap1 = cap * 2;
  auto entries1 = (uint16_t *)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto t = entries[i];
    if (t) {
      auto t1 = tcompounds[t];
      entries1[slot(entries1, cap1, t1->v, t1->n)] = t;
    }
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

TCompound *store(const uint16_t *p, w n) {
  auto r = (TCompound *)xmalloc(offsetof(TCompound, v) + n * sizeof *p);
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
      throw "Too many compound types";
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = tcompounds.n;
  entries[i] = t;
  tcompounds.push(store(p, n));
  return t | t_compound;
}
} // namespace types

vec<TCompound *> tcompounds(0);

w type(const vec<uint16_t> &v) { return types::put(v.p, v.n); }

w type(w r, w t1) {
  uint16_t v[2];
  v[0] = r;
  v[1] = t1;
  return types::put(v, sizeof v / sizeof *v);
}

w type(Sym *name) {
  if (name->t)
    return name->t;
  if (types::atoms >= t_compound)
    throw "Too many atomic types";
  return name->t = types::atoms++;
}

vec<w> neg, pos;

void clause() {}

namespace {
bank<Int> ints;
bank<Rat> rats;
} // namespace

namespace compounds {
// Must be a power of 2
w cap = 0x1000;
w count;
Compound **entries = (Compound **)xcalloc(cap, sizeof(Compound *));

bool eq(const Compound *x, const w *p, w n) {
  if (x->n != n)
    return false;
  return !memcmp(x->v, p, n * sizeof *p);
}

w slot(Compound **entries, w cap, const w *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof *p) & mask;
  while (entries[i] && !eq(entries[i], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (Compound **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto x = entries[i];
    if (x)
      entries1[slot(entries1, cap1, x->v, x->n)] = x;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

Compound *store(const w *p, w n) {
  auto r = (Compound *)xmalloc(offsetof(Compound, v) + n * sizeof *p);
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

w int1(Int *x) { return tag(ints.put(x), a_int); }
w rat(Rat *x) { return tag(rats.put(x), a_rat); }
w real(Rat *x) { return tag(rats.put(x), a_real); }

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

w imp(w a, w b) { return term(basic(b_or), term(basic(b_not), a), b); }
