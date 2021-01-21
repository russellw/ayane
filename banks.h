template <class T> class bank {
  int cap = 0x100;
  int count = 0;
  T **p = (T **)xcalloc(cap, sizeof(T *));

  bool eq(T *a, const void *s, int bytes) {
    if (a->n != bytes)
      return false;
    return !memcmp(a->s, s, bytes);
  }

  int slot(T **p, int cap, const void *s, int bytes) {
    auto mask = cap - 1;
    auto i = fnv(s, bytes) & mask;
    while (p[i] && !eq(p[i], s, bytes))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto p1 = (T **)xcalloc(cap1, sizeof(T *));
    for (int i = 0; i != cap; ++i) {
      auto a = p[i];
      if (a)
        p1[slot(p1, cap1, a->s, a->n)] = a;
    }
    free(p);
    cap = cap1;
    p = p1;
  }

public:
  void init(T *a) {
    ++count;
    assert(count <= cap * 3 / 4);
    auto i = slot(p, cap, a->s, a->n);
    assert(!p[i]);
    p[i] = a;
  }

  T *intern(const void *s, int bytes) {
    auto i = slot(p, cap, s, bytes);
    if (p[i])
      return p[i];
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(p, cap, s, bytes);
    }
    return p[i] = store(s, bytes);
  }
};
