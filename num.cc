#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
template <class T> class bank {
  si cap = 0x10;
  si count;
  T **entries = (T **)xcalloc(cap, sizeof *entries);

  si slot(T **entries, si cap, const T &x) {
    auto mask = cap - 1;
    auto i = x.hash() & mask;
    while (entries[i] && !entries[i]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    assert(ispow2(cap));
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof *entries);
    for (si i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, *x)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *store(const T &x) {
    auto r = (T *)xmalloc(sizeof x);
    *r = x;
    return r;
  }

public:
  T *put(T &x) {
    auto i = slot(entries, cap, x);
    if (entries[i]) {
      x.clear();
      return entries[i];
    }
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, x);
    }
    return entries[i] = store(x);
  }
};

bank<Int> ints;
bank<Rat> rats;
} // namespace

Int *intern(Int &x) { return ints.put(x); }
Rat *intern(Rat &x) { return rats.put(x); }
