#include "main.h"

// The number of types is expected to be small. It is therefore possible to fit
// a type reference into 16 bits, and desirable because this allows the type of
// a variable to be read without an extra memory access. Compound types are
// therefore tracked with an unusual kind of memory bank in which entries are
// 16-bit words rather than pointers
namespace {
ty atypes = basic_types;

w cap = 0x10;
ty *entries = (ty *)xcalloc(cap, sizeof(ty));

bool eq(const TCompound *t, const ty *p, w n) {
  if (t->n != n)
    return false;
  return !memcmp(t->v, p, n * sizeof(ty));
}

w slot(ty *entries, w cap, const ty *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(ty)) & mask;
  while (entries[i] && !eq(tcompounds[entries[i]], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (ty *)xcalloc(cap1, sizeof(ty));
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

TCompound *store(const ty *p, w n) {
  auto r = (TCompound *)xmalloc(offsetof(TCompound, v) + n * sizeof(ty));
  r->n = n;
  memcpy(r->v, p, n * sizeof(ty));
  return r;
}

ty put(const ty *p, w n) {
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
} // namespace

vec<TCompound *> tcompounds(0);

ty type(const vec<ty> &v) { return put(v.p, v.n); }

ty type(ty r, ty t1) {
  ty v[2];
  v[0] = r;
  v[1] = t1;
  return put(v, sizeof v / sizeof *v);
}

ty type(sym *name) {
  if (name->t)
    return name->t;
  if (atypes >= t_compound)
    throw "Too many atomic types";
  return name->t = atypes++;
}

ty typeof(w a) {
  switch (a & 7) {
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) == a_sym) {
      auto t = symp(op)->ft;
      assert(istcompound(t));
      auto t1 = tcompoundp(t);
      assert(size(a) == t1->n);
      return t1->v[0];
    }
    unreachable;
  }
  case a_int:
    return t_int;
  case a_rat:
    return t_rat;
  case a_real:
    return t_real;
  case a_sym:
    return symp(a)->ft;
  case a_var:
    return a >> 3 & (1 << 8 * sizeof(ty)) - 1;
  }
  unreachable;
  return 0;
}
