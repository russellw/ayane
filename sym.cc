#include "main.h"

namespace syms {
// Must be a power of 2, and large enough to hold the largest collection of
// entries that will be loaded at initialization time
w cap = 0x100;
w count;
Sym **entries = (Sym **)xcalloc(cap, sizeof(Sym *));

w slot(Sym **entries, w cap, const char *p, w n) {
  auto mask = cap - 1;
  auto i = fnv(p, n) & mask;
  while (entries[i] && !entries[i]->eq(p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (Sym **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
    auto x = entries[i];
    if (x)
      entries1[slot(entries1, cap1, x->v, x->n)] = x;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

struct init {
  init() {
    for (w i = 0; i != nkeywords; ++i) {
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

Sym *put(const char *p, w n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return Sym::process(entries[i]);
  if (++count >= cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  return Sym::process(entries[i] = Sym::store(p, n));
}
} // namespace syms

Sym *intern(const char *s, w n) {
  if (n > 0xffff)
    throw "Symbol too long";
  return syms::put(s, n);
}
