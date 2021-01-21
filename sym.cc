#include "main.h"

namespace {
sym *store(const void *s, int n) {
  auto r = (sym *)mmalloc(offsetof(sym, s) + n);
  r->n = n;
  memcpy(r->s, s, n);
  return r;
}

bank<sym> syms;

struct init {
  init() {
    for (int i = 0; i != nkeywords; ++i) {
      auto k = keywords + i;
      assert(k->n < sizeof k->s);
      syms.init(k);
    }
  }
} init1;
} // namespace

sym *intern(const char *s, int n) {
  if (n > 0xffff)
    err("Symbol too long");
  return syms.intern(s, n);
}
