#include "main.h"

namespace syms {
// Must be a power of 2, and large enough to hold the largest collection of
// entries that will be loaded at initialization time
w cap = 0x100;
w count;
Sym **entries = (Sym **)xcalloc(cap, sizeof(Sym *));

bool strmemeq(const char *s, const char *p, w n) {
  while (n--)
    if (*s++ != *p++)
      return false;
  return !*s;
}

w slot(Sym **entries, w cap, const char *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n) & mask;
  while (entries[i] && !strmemeq(entries[i]->v, p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (Sym **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto S = entries[i];
    if (S)
      entries1[slot(entries1, cap1, S->v, strlen(S->v))] = S;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

struct init {
  init() {
    for (w i = 0; i != nkeywords; ++i) {
      auto S = keywords + i;
      assert(strlen(S->v) < sizeof S->v);
      ++count;
      assert(count <= cap * 3 / 4);
      auto j = slot(entries, cap, S->v, strlen(S->v));
      assert(!entries[j]);
      entries[j] = S;
    }
  }
} init1;

Sym *store(const char *s, w n) {
  auto r = (Sym *)xmalloc(offsetof(Sym, v) + n + 1);
  memset(r, 0, offsetof(Sym, v));
  memcpy(r->v, s, n);
  r->v[n] = 0;
  return r;
}

Sym *put(const char *p, w n) {
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

Sym *intern(const char *s, w n) { return syms::put(s, n); }
