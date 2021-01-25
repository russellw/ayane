template <class T> class bank_set {
  size_t cap = 0x10;
  size_t count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  size_t slot(T **entries, size_t cap, T *t) {
    auto mask = cap - 1;
    auto i = t->hash() & mask;
    while (entries[i] && !entries[i]->eq(t))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (size_t i = 0; i != cap; ++i) {
      auto t = entries[i];
      if (t)
        entries1[slot(entries1, cap1, t)] = t;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

  T *store(T *t) {
    auto r = (T *)xmalloc(sizeof(T));
    *r = *t;
    return r;
  }

public:
  T *put(T *t) {
    auto i = slot(entries, cap, t);
    if (entries[i]) {
      t->clear();
      return entries[i];
    }
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, t);
    }
    return entries[i] = store(t);
  }
};

template <class K, class T, class R> class bank_map {
  size_t cap = 0x100;
  size_t count;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  size_t slot(T **entries, size_t cap, const K *p, size_t n) {
    auto mask = cap - 1;
    auto i = fnv(p, n * sizeof(K)) & mask;
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

public:
  void add(T *t) {
    ++count;
    assert(count <= cap * 3 / 4);
    auto i = slot(entries, cap, t->v, t->n);
    assert(!entries[i]);
    entries[i] = t;
  }

  R put(const K *p, size_t n) {
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
