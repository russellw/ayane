template <class T> class bank {
  int cap = 0x100;
  int count = 0;
  T **entries = (T **)xcalloc(cap, sizeof(T *));

  bool eq(T *a, const char *s, int n) {
    if (a->n != n)
      return false;
    return !memcmp(a->s, s, n);
  }

  int slot(T **entries, int cap, const char *s, int n) {
    auto mask = cap - 1;
    auto i = fnv(s, n) & mask;
    while (entries[i] && !eq(entries[i], s, n))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto entries1 = (T **)xcalloc(cap1, sizeof(T *));
    for (int i = 0; i != cap; ++i) {
      auto a = entries[i];
      if (a)
        entries1[slot(entries1, cap1, a->s, a->n)] = a;
    }
    free(entries);
    cap = cap1;
    entries = entries1;
  }

public:
  void init(T *a) {
    ++count;
    assert(count <= cap * 3 / 4);
    auto i = slot(entries, cap, a->s, a->n);
    assert(!entries[i]);
    entries[i] = a;
  }

  T *intern(const char *s, int n) {
    auto i = slot(entries, cap, s, n);
    if (entries[i])
      return entries[i];
    if (++count > cap * 3 / 4) {
      expand();
      i = slot(entries, cap, s, n);
    }
    return entries[i] = store(s, n);
  }
};
