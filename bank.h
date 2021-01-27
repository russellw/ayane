template <class T> class BankSet {
  // Must be a power of 2
  w cap = 0x10;
  w count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  w slot(T **entries, w cap, T *x) {
    auto mask = cap - 1;
    auto i = x->hash() & mask;
    while (entries[i] && !entries[i]->eq(x))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (w i = 0; i != cap; ++i) {
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

template <class K, class T, class R> class BankMap {
  // Must be a power of 2, and large enough to hold the largest collection of
  // entries that will be loaded at initialization time
  w cap = 0x100;
  w count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  w slot(T **entries, w cap, const K *p, w n) {
    auto mask = cap - 1;
    auto i = fnv(p, n * sizeof(K)) & mask;
    while (entries[i] && !entries[i]->eq(p, n))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (w i = 0; i != cap; ++i) {
      auto x = entries[i];
      if (x)
        entries1[slot(entries1, cap1, x->v, x->n)] = x;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

public:
  void add(T *x) {
    ++count;
    assert(count <= cap * 3 / 4);
    auto i = slot(entries, cap, x->v, x->n);
    assert(!entries[i]);
    entries[i] = x;
  }

  R put(const K *p, w n) {
    auto i = slot(entries, cap, p, n);
    if (entries[i])
      return T::process(entries[i]);
    if (++count >= cap * 3 / 4) {
      expand();
      i = slot(entries, cap, p, n);
    }
    return T::process(entries[i] = T::store(p, n));
  }
};
