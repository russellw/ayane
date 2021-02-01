#include "main.h"

namespace {
BankSet<Int> ints;
BankSet<Rat> rats;
BankMap<w, Compound, w> compounds;
} // namespace

w int1(Int *x) { return tag(ints.put(x), a_int); }
w rat(Rat *x) { return tag(rats.put(x), a_rat); }
w real(Rat *x) { return tag(rats.put(x), a_real); }

w term(const vec<w> &v) { return compounds.put(v.p, v.n); }

w term(w op, w a) {
  w v[2];
  v[0] = op;
  v[1] = a;
  return compounds.put(v, sizeof v / sizeof *v);
}

w term(w op, w a, w b) {
  w v[3];
  v[0] = op;
  v[1] = a;
  v[2] = b;
  return compounds.put(v, sizeof v / sizeof *v);
}

w imp(w a, w b) { return term(basic(b_or), term(basic(b_not), a), b); }
