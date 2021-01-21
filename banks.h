template <class Key, class T> class bank {
  int cap = 0x100;
  int count = 0;
  T **p = (T **)xcalloc(cap, sizeof(T *));

  int slot(T **p, int cap, const Key &k) {
    auto mask = cap - 1;
    auto i = k.hash() & mask;
    while (p[i] && !(k == p[i]))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto p1 = (T **)xcalloc(cap1, sizeof(T *));
    for (int i = 0; i != cap; ++i) {
      auto a = p[i];
      if (a) {
        Key k(a);
        p1[slot(p1, cap1, k)] = a;
      }
    }
    free(p);
    cap = cap1;
    p = p1;
  }

public:
  void init(const Key &k, T *a) {
    ++count;
    assert(count <= cap * 3 / 4);
    auto i = slot(p, cap, k);
    assert(!p[i]);
    p[i] = a;
  }

  T *operator[](const Key &k) {
    auto i = slot(p, cap, k);
    if (p[i])
      return p[i];
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(p, cap, k);
    }
    return p[i] = k.store();
  }
};
