#include "main.h"

namespace {
BankMap<char, Sym, Sym *> syms;

struct init {
  init() {
    for (w i = 0; i != nkeywords; ++i) {
      auto S = keywords + i;
      assert(S->n <= sizeof S->v);
      syms.add(S);
    }
  }
} init1;
} // namespace

Sym *intern(const char *s, w n) {
  if (n > 0xffff)
    throw "Symbol too long";
  return syms.put(s, n);
}
