#include "main.h"

namespace {
template <class T> class bank {
  size_t cap = 0x10;
  size_t count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  size_t slot(T **entries, size_t cap, T *x) {
    auto mask = cap - 1;
    auto i = x->hash() & mask;
    while (entries[i] && !entries[i]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (size_t i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, x)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *store(T *x) {
    auto r = (T *)xmalloc(sizeof(T));
    *r = *x;
    return r;
  }

public:
  T *put(T *x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x->clear();
      return entries[i];
    }
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, x);
    }
    return entries[i] = store(x);
  }
};

bank<int_t> ints;
bank<rat_t> rats;
} // namespace

term int1(int_t *x) { return tag(ints.put(x), a_int); }
term rat(rat_t *x) { return tag(rats.put(x), a_rat); }
term real(rat_t *x) { return tag(rats.put(x), a_real); }

namespace {
fn_t *mkfn(ty t) {
  auto r = (fn_t *)xmalloc(sizeof(fn_t));
  r->name = 0;
  r->t = t;
  return r;
}
} // namespace

term fn(ty t) { return tag(mkfn(t), a_fn); }

term fn(ty t, sym *name) {
  auto a = name->f;
  if (a)
    return a;
  auto r = mkfn(t);
  r->name = name;
  return name->f = tag(r, a_fn);
}

namespace cterms {
size_t cap = 0x10;
size_t count;
cterm_t **entries = (cterm_t **)xcalloc(cap, sizeof(cterm_t *));

bool eq(const cterm_t *x, const term *p, size_t n) {
  if (x->n != n)
    return false;
  return !memcmp(x->v, p, n * sizeof(term));
}

size_t slot(cterm_t **entries, size_t cap, const term *p, size_t n) {
  auto mask = cap - 1;
  auto i = fnv(p, n * sizeof(term)) & mask;
  while (entries[i] && !eq(entries[i], p, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  auto cap1 = cap * 2;
  auto entries1 = (cterm_t **)xcalloc(cap1, sizeof(cterm_t *));
  for (size_t i = 0; i != cap; ++i) {
    auto x = entries[i];
    if (x)
      entries1[slot(entries1, cap1, x->v, x->n)] = x;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

cterm_t *mk(const term *p, size_t n) {
  auto r = (cterm_t *)xmalloc(offsetof(cterm_t, v) + n * sizeof(term));
  r->n = n;
  memcpy(r->v, p, n * sizeof(term));
  return r;
}

term cterm(const term *p, size_t n) {
  auto i = slot(entries, cap, p, n);
  if (entries[i])
    return tag(entries[i], a_compound);
  if (++count >= cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, n);
  }
  return tag(entries[i] = mk(p, n), a_compound);
}
} // namespace cterms

term cterm(vec<term> &v) { return cterms::cterm(v.p, v.n); }

term cterm(int op, term a) {
  term v[2];
  v[0] = aterm(op);
  v[1] = a;
  return cterms::cterm(v, sizeof v / sizeof(term));
}

term cterm(int op, term a, term b) {
  term v[3];
  v[0] = aterm(op);
  v[1] = a;
  v[2] = b;
  return cterms::cterm(v, sizeof v / sizeof(term));
}
