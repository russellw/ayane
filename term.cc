#include "main.h"

namespace {
bank_set<int_t> ints;
bank_set<rat_t> rats;
bank_map<w, cterm_t, w> cterms;
} // namespace

w int1(int_t *x) { return tag(ints.put(x), a_int); }
w rat(rat_t *x) { return tag(rats.put(x), a_rat); }
w real(rat_t *x) { return tag(rats.put(x), a_real); }

namespace {
fn_t *mkfn(type t) {
  auto r = (fn_t *)xmalloc(sizeof(fn_t));
  r->name = 0;
  r->t = t;
  return r;
}
} // namespace

w fn(type t) { return tag(mkfn(t), a_fn); }

w fn(type t, sym *name) {
  auto a = name->f;
  if (a)
    return a;
  auto r = mkfn(t);
  r->name = name;
  return name->f = tag(r, a_fn);
}

w term(const vec<w> &v) { return cterms.put(v.p, v.n); }

w term(w op, w a) {
  w v[2];
  v[0] = term(op);
  v[1] = a;
  return cterms.put(v, sizeof v / sizeof(w));
}

w term(w op, w a, w b) {
  w v[3];
  v[0] = term(op);
  v[1] = a;
  v[2] = b;
  return cterms.put(v, sizeof v / sizeof(w));
}
