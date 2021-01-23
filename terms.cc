#include "main.h"

namespace {
template <class T> struct bank {
  vec<T *> ptrs;

private:
  size_t cap = 0x10;
  term *entries = (term *)xcalloc(cap, sizeof(term));

  size_t slot(term *entries, size_t cap, T *x) {
    auto mask = cap - 1;
    auto i = x->hash() & mask;
    while (entries[i] && !ptrs[entries[i]]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (term *)xcalloc(cap1, sizeof(term));
    for (size_t i = 0; i != cap; ++i) {
      auto j = entries[i];
      if (j)
        entries1[slot(entries1, cap1, ptrs[j])] = j;
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
  bank() { ptrs.push(0); }

  term put(T *x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x->clear();
      return entries[i];
    }
    if (ptrs.n >= cap * 3 / 4) {
      if (ptrs.n >= 1 << a_bits)
        err("Too many numbers");
      expand();
      i = slot(entries, cap, x);
    }
    auto j = ptrs.n;
    entries[i] = j;
    ptrs.push(store(x));
    return j;
  }
};

bank<int_t> ints;
bank<rat_t> rats;
} // namespace

term int1(int_t *x) { return ints.put(x) | a_int; }
term rat(rat_t *x) { return rats.put(x) | a_rat; }
term real(rat_t *x) { return rats.put(x) | a_real; }

int_t *intp(term a) {
  assert((a & a_tags) == a_int);
  auto r = ints.ptrs[a & (1 << a_bits) - 1];
  assert(r);
  return r;
}

rat_t *ratp(term a) {
  assert((a & a_tags) == a_rat);
  auto r = rats.ptrs[a & (1 << a_bits) - 1];
  assert(r);
  return r;
}
