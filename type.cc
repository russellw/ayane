#include "main.h"

namespace {
std::unordered_map<const sym *, type> atypes;

// hash table for structure sharing
size_t cap = 0x10;
type *entries = (type *)xcalloc(cap, sizeof(type));

bool eq(const ctype_t *t, const type *p, size_t n) {
  if (t->n != n)
    return false;
  return !memcmp(t->v, p, n * sizeof(type));
}

size_t slot(type *entries, size_t cap, const type *p, size_t n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(type)) & mask;
  while (entries[i] && !eq(ctypes[entries[i]], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (type *)xcalloc(cap1, sizeof(type));
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

ctype_t *mk(const type *p, size_t n) {
  // Types have little enough information associated with them that we can reuse
  // type objects between problems, so we never need to free them, so we can use
  // monotonic allocation for minimal overhead
  auto r = (ctype_t *)mmalloc(offsetof(ctype_t, v) + n * sizeof(type));
  r->n = n;
  memcpy(r->v, p, n * sizeof(type));
  return r;
}
} // namespace

vec<ctype_t *> ctypes(0);

type atype(const sym *name) {
  auto &t = atypes[name];
  if (t)
    return t;
  return t = atypes.size() + basic_types;
}

type ctype(const type *p, size_t n) {
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
