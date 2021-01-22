#include "main.h"

namespace {
std::unordered_map<const sym *, ty> atypes;

// hash table for structure sharing
int cap = 0x10;
ty *entries = (ty *)xcalloc(cap, sizeof(ty));

bool eq(const ctype_t *t, const ty *p, int n) {
  if (t->n != n)
    return false;
  return !memcmp(t->v, p, n * sizeof(ty));
}

int slot(ty *entries, int cap, const ty *p, int n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(ty)) & mask;
  while (entries[i] && !eq(ctype(entries[i]), p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (ty *)xcalloc(cap1, sizeof(ty));
  for (int i = 0; i != cap; ++i) {
    auto t = entries[i];
    if (t) {
      auto t1 = ctype(t);
      entries1[slot(entries1, cap1, t1->v, t1->n)] = t;
    }
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

ctype_t *store(const ty *p, int n) {
  auto r = (ctype_t *)mmalloc(offsetof(ctype_t, v) + n * sizeof(ty));
  r->n = n;
  memcpy(r->v, p, n * sizeof(ty));
  return r;
}
} // namespace

vec<ctype_t *> ctypes;

ty type(const sym *name) {
  auto &t = atypes[name];
  if (t)
    return t;
  auto i = atypes.size() + basic_types;
  if (i >= ctype_tag)
    err("Too many types");
  t = i;
  return i;
}

ty type(const ty *p, int n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i];
  if (ctypes.n >= cap * 3 / 4) {
    if (ctypes.n >= ctype_tag)
      err("Too many types");
    expand();
    i = slot(entries, cap, p, n);
  }
  auto t = ctypes.n | ctype_tag;
  entries[i] = t;
  ctypes.push(store(p, n));
  return t;
}
