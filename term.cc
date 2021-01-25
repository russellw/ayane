#include "main.h"

namespace {
bank_set<int_t> ints;
bank_set<rat_t> rats;
bank_map<term, cterm_t, term> cterms;
} // namespace

term int1(int_t *x) { return tag(ints.put(x), a_int); }
term rat(rat_t *x) { return tag(rats.put(x), a_rat); }
term real(rat_t *x) { return tag(rats.put(x), a_real); }

namespace {
fn_t *mkfn(type t) {
  auto r = (fn_t *)xmalloc(sizeof(fn_t));
  r->name = 0;
  r->t = t;
  return r;
}
} // namespace

term fn(type t) { return tag(mkfn(t), a_fn); }

term fn(type t, sym *name) {
  auto a = name->f;
  if (a)
    return a;
  auto r = mkfn(t);
  r->name = name;
  return name->f = tag(r, a_fn);
}

term cterm(vec<term> &v) { return cterms.put(v.p, v.n); }

term cterm(int op, term a) {
  term v[2];
  v[0] = aterm(op);
  v[1] = a;
  return cterms.put(v, sizeof v / sizeof(term));
}

term cterm(int op, term a, term b) {
  term v[3];
  v[0] = aterm(op);
  v[1] = a;
  v[2] = b;
  return cterms.put(v, sizeof v / sizeof(term));
}
