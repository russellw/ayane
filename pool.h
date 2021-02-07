template <w cap = 1000000> class pool {
  w p = v;
  char v[cap];

public:
  void *alloc(w n) {
    assert(!((w)p & 7));
    assert(0 <= n);
    n = n + 7 & ~(w)7;
    auto r = p;
    p += n;
    if (p > v + cap) {
      fprintf(stderr, "pool overflow");
      stacktrace();
      exit(1);
    }
#ifdef DEBUG
    memset(r, 0xcc, n);
#endif
    return r;
  }

  void clear() { p = v; }
};
