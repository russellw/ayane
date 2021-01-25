#include "main.h"

namespace {
size_t cap = 0x100;
size_t count;
sym **entries = (sym **)xcalloc(cap, sizeof(sym *));

bool eq(const sym *S, const char *s, size_t n) {
  if (S->n != n)
    return false;
  return !memcmp(S->v, s, n);
}

size_t slot(sym **entries, size_t cap, const char *s, size_t n) {
  auto mask = cap - 1;
  auto i = fnv(s, n) & mask;
  while (entries[i] && !eq(entries[i], s, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (sym **)xcalloc(cap1, sizeof(sym *));
  for (size_t i = 0; i != cap; ++i) {
    auto S = entries[i];
    if (S)
      entries1[slot(entries1, cap1, S->v, S->n)] = S;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

sym *mk(const char *s, size_t n) {
  auto r = (sym *)xmalloc(offsetof(sym, v) + n);
  memset(r, 0, offsetof(sym, v));
  r->n = n;
  memcpy(r->v, s, n);
  return r;
}

struct init {
  init() {
    for (size_t i = 0; i != nkeywords; ++i) {
      auto S = keywords + i;
      assert(S->n <= sizeof S->v);
      ++count;
      assert(count <= cap * 3 / 4);
      auto j = slot(entries, cap, S->v, S->n);
      assert(!entries[j]);
      entries[j] = S;
    }
  }
} init1;
} // namespace

sym *intern(const char *s, size_t n) {
  if (n > 0xffff)
    err("Symbol too long");
  auto i = slot(entries, cap, s, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, s, n);
  }
  return entries[i] = mk(s, n);
}
