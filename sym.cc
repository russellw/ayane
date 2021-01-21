#include "main.h"

namespace {
int cap = 0x100;
int count;
sym **entries = (sym **)xcalloc(cap, sizeof(sym *));

bool eq(const sym *S, const char *s, int n) {
  if (S->n != n)
    return false;
  return !memcmp(S->s, s, n);
}

int slot(sym **entries, int cap, const char *s, int n) {
  auto mask = cap - 1;
  auto i = fnv(s, n) & mask;
  while (entries[i] && !eq(entries[i], s, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (sym **)xcalloc(cap1, sizeof(sym *));
  for (int i = 0; i != cap; ++i) {
    auto S = entries[i];
    if (S)
      entries1[slot(entries1, cap1, S->s, S->n)] = S;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

sym *store(const char *s, int n) {
  auto r = (sym *)mmalloc(offsetof(sym, s) + n);
  r->n = n;
  memcpy(r->s, s, n);
  return r;
}

struct init {
  init() {
    for (int i = 0; i != nkeywords; ++i) {
      auto S = keywords + i;
      assert(S->n < sizeof S->s);
      ++count;
      assert(count <= cap * 3 / 4);
      auto j = slot(entries, cap, S->s, S->n);
      assert(!entries[j]);
      entries[j] = S;
    }
  }
} init1;
} // namespace

sym *intern(const char *s, int n) {
  if (n > 0xffff)
    err("Symbol too long");
  auto i = slot(entries, cap, s, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, s, n);
  }
  return entries[i] = store(s, n);
}
