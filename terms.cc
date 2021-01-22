#include "main.h"

namespace {
template <class T, term tag> struct bank {
  vec<T *> ptrs;

  int cap = 0x10;
  term *entries = (term *)xcalloc(cap, sizeof(term));

  T *ptr(term a) { return ptrs[a & 0x3fffffff]; }

  int slot(term *entries, int cap, T *x) {
    auto mask = cap - 1;
    auto i = x->hash() & mask;
    while (entries[i] && !ptr(entries[i])->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (term *)xcalloc(cap1, sizeof(term));
    for (int i = 0; i != cap; ++i) {
      auto a = entries[i];
      if (a)
        entries1[slot(entries1, cap1, ptr(a))] = a;
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

  term put(T *x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x->clear();
      return entries[i];
    }
    if (ptrs.n >= cap * 3 / 4) {
      if (ptrs.n >= tag)
        err("Too many numbers");
      expand();
      i = slot(entries, cap, x);
    }
    auto a = ptrs.n | tag;
    entries[i] = a;
    ptrs.push(store(x));
    return a;
  }
};

bank<int_t, 0x80000000> ints;
} // namespace

term int1(int_t *x) { return ints.put(x); }
