#include "main.h"

namespace {
template <class T> class bank_set {
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

bank_set<int_t> ints;
bank_set<rat_t> rats;
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

template <class T> class bank_map {
  size_t cap = 0x10;
  size_t count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  size_t slot(T **entries, size_t cap, const term *p, size_t n) {
    auto mask = cap - 1;
    auto i = fnv(p, n * sizeof(term)) & mask;
    while (entries[i] && !entries[i]->eq(p, n))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (size_t i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, x->v, x->n)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *mk(const term *p, size_t n) {
    auto r = (T *)xmalloc(offsetof(T, v) + n * sizeof(term));
    r->n = n;
    memcpy(r->v, p, n * sizeof(term));
    return r;
  }

public:
  term put(const term *p, size_t n) {
    auto i = slot(entries, cap, p, n);
    if (entries[i])
      return tag(entries[i], a_compound);
    if (++count >= cap * 3 / 4) {
      expand();
      i = slot(entries, cap, p, n);
    }
    return tag(entries[i] = mk(p, n), a_compound);
  }
};

static bank_map<cterm_t> cterms;

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
