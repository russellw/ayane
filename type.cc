#include "main.h"

// The number of types is expected to be small. It is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. Compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
namespace {
type atypes = basic_types;

w cap = 0x10;
type *entries = (type *)xcalloc(cap, sizeof(type));

bool eq(const ctype_t *t, const type *p, w n) {
  if (t->n != n)
    return false;
  return !memcmp(t->v, p, n * sizeof(type));
}

w slot(type *entries, w cap, const type *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(type)) & mask;
  while (entries[i] && !eq(ctypes[entries[i]], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (type *)xcalloc(cap1, sizeof(type));
  for (w i = 0; i != cap; ++i) {
    auto t = entries[i];
    if (t) {
      auto t1 = ctypes[t];
      entries1[slot(entries1, cap1, t1->v, t1->n)] = t;
    }
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

ctype_t *store(const type *p, w n) {
  auto r = (ctype_t *)xmalloc(offsetof(ctype_t, v) + n * sizeof(type));
  r->n = n;
  memcpy(r->v, p, n * sizeof(type));
  return r;
}
} // namespace

vec<ctype_t *> ctypes(0);

type atype(sym *name) {
  if (name->t)
    return name->t;
  if (atypes >= t_compound)
    err("Too many atomic types");
  return name->t = atypes++;
}

type ctype(const type *p, w n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i] | t_compound;
  if (ctypes.n >= cap * 3 / 4) {
    if (ctypes.n >= t_compound)
      err("Too many compound types");
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = ctypes.n;
  entries[i] = t;
  ctypes.push(store(p, n));
  return t | t_compound;
}

type typeof(w a) {
  switch (a & 7) {
  case a_var:
    return a >> 3 & (1 << 8 * sizeof(type)) - 1;
  case a_int:
    return t_int;
  case a_rat:
    return t_rat;
  case a_real:
    return t_real;
  case a_fn:
    return fnp(a)->t;
  }
  __builtin_unreachable();
}
