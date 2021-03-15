template <si cap = 300000000> class pool {
  char *p;
  char v[cap];

public:
  void init() { p = v; }

  void *alloc(si n) {
    assert(!((si)p & 7));
    assert(0 <= n);
    assert(!(n & 7));
    auto r = p;
    p += n;
    if (p > v + cap)
      err("pool overflow");
#ifdef DEBUG
    memset(r, 0xcc, n);
#endif
    return r;
  }
};

extern pool<> pool1;
