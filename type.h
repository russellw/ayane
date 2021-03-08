enum {
  t_none,

  t_bool,
  t_int,
  t_rat,
  t_real,
  t_individual,

  nbasictypes
};

struct tcompound {
  uint16_t n;
  uint16_t v[0];
};

class type {
  uint16_t x;

public:
  explicit type(si x) : x(x) {}

  // SORT
  bool operator==(type b) { return x == b.x; }

  w operator[](si i) {
    assert(0 <= i);
    assert(i < size());
    return compoundp()->v[i];
  }

  tcompound *compoundp() {
    assert(iscompound());
    return tcompounds[x & ~t_compound];
  }

  bool iscompound() { return x >= t_compound; }

  si size() { return compoundp()->n; }
  ///
};

extern ary<tcompound *> tcompounds;

inline tcompound *tcompoundp(w t) {
  assert(t & t_compound);
  return tcompounds[t & ~t_compound];
}

// SORT
void defaulttype(w t, w a);
w mktype(const vec<uint16_t> &v);
w mktype(sym *name);
w mktype(w r, w t1);
w numtype(w a);
void requiretype(w t, w a);
w typeof(w a);
///
