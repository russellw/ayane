#include "main.h"

namespace {
bank_set<Int> ints;
bank_set<Rat> rats;
bank_map<w, Compound, w> compounds;
} // namespace

w int1(Int *x) { return tag(ints.put(x), a_int); }
w rat(Rat *x) { return tag(rats.put(x), a_rat); }
w real(Rat *x) { return tag(rats.put(x), a_real); }

namespace {
Fn *mkfn(ty t) {
  auto r = (Fn *)xmalloc(sizeof(Fn));
  r->name = 0;
  r->t = t;
  return r;
}
} // namespace

w fn(ty t) { return tag(mkfn(t), a_fn); }

w fn(ty t, sym *name) {
  auto a = name->f;
  if (a)
    return a;
  auto r = mkfn(t);
  r->name = name;
  return name->f = tag(r, a_fn);
}

w term(const vec<w> &v) { return compounds.put(v.p, v.n); }

w term(w op, w a) {
  w v[2];
  v[0] = op;
  v[1] = a;
  return compounds.put(v, sizeof v / sizeof(w));
}

w term(w op, w a, w b) {
  w v[3];
  v[0] = op;
  v[1] = a;
  v[2] = b;
  return compounds.put(v, sizeof v / sizeof(w));
}
