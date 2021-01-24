#include "main.h"

namespace {
std::unordered_map<const sym *, ty> atypes;

// hash table for structure sharing
size_t cap = 0x10;
ty *entries = (ty *)xcalloc(cap, sizeof(ty));

bool eq(const ctype_t *t, const ty *p, size_t n) {
  if (t->n != n)
    return false;
  return !memcmp(t->v, p, n * sizeof(ty));
}

size_t slot(ty *entries, size_t cap, const ty *p, size_t n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(ty)) & mask;
  while (entries[i] && !eq(ctypes[entries[i]], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (ty *)xcalloc(cap1, sizeof(ty));
  for (size_t i = 0; i != cap; ++i) {
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

ctype_t *mk(const ty *p, size_t n) {
  // Types have little enough information associated with them that we can reuse
  // type objects between problems, so we never need to free them, so we can use
  // monotonic allocation for minimal overhead
  auto r = (ctype_t *)mmalloc(offsetof(ctype_t, v) + n * sizeof(ty));
  r->n = n;
  memcpy(r->v, p, n * sizeof(ty));
  return r;
}
} // namespace

vec<ctype_t *> ctypes(0);

ty atype(const sym *name) {
  auto &t = atypes[name];
  if (t)
    return t;
  return t = atypes.size() + basic_types;
}

ty type(const ty *p, size_t n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i] | t_compound;
  if (ctypes.n >= cap * 3 / 4) {
    if (ctypes.n >= t_compound)
      err("Too many types");
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = ctypes.n;
  entries[i] = t;
  ctypes.push(mk(p, n));
  return t | t_compound;
}
