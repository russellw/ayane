#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
// must be a power of 2, and large enough to hold the largest collection of
// entries that will be loaded at initialization time
si cap = 0x100;
si count;
sym **entries = (sym **)xcalloc(cap, sizeof *entries);

bool strmemeq(const char *s, const char *p, si n) {
  while (n--)
    if (*s++ != *p++)
      return 0;
  return !*s;
}

w slot(sym **entries, si cap, const char *p, si n) {
  auto mask = cap - 1;
  auto i = fnv(p, n) & mask;
  while (entries[i] && !strmemeq(entries[i]->v, p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (sym **)xcalloc(cap1, sizeof *entries);
  for (si i = 0; i != cap; ++i) {
    auto s = entries[i];
    if (s)
      entries1[slot(entries1, cap1, s->v, strlen(s->v))] = s;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

struct init {
  init() {
    for (si i = 0; i != sizeof keywords / sizeof *keywords; ++i) {
      auto s = keywords + i;
      assert(strlen(s->v) < sizeof s->v);
      ++count;
      assert(count <= cap * 3 / 4);
      auto j = slot(entries, cap, s->v, strlen(s->v));
      assert(!entries[j]);
      entries[j] = s;
    }
  }
} init1;

sym *store(const char *s, si n) {
  auto r = (sym *)mmalloc(offsetof(sym, v) + n + 1);
  memset(r, 0, offsetof(sym, v));
  memcpy(r->v, s, n);
  r->v[n] = 0;
  return r;
}

sym *put(const char *p, si n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  return entries[i] = store(p, n);
}
} // namespace

void init_syms() {
  for (auto i = entries, e = entries + cap; i != e; ++i) {
    auto s = *i;
    if (s)
      s->ft = type::none;
  }
}

sym *intern(const char *s, si n) { return put(s, n); }
