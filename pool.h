template <w cap = 300000000> class pool {
  char *p;
  char v[cap];

public:
  void *alloc(w n) {
    assert(!((w)p & 7));
    assert(0 <= n);
    n = n + 7 & ~(w)7;
    auto r = p;
    p += n;
    if (p > v + cap)
      err("pool overflow");
#ifdef DEBUG
    memset(r, 0xcc, n);
#endif
    return r;
  }

  void clear() { p = v; }
};
