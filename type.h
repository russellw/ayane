enum {
  t_none,

  t_bool,
  t_int,
  t_rat,
  t_real,
  t_individual,

  nbasictypes
};

const w t_compound = 1 << 15;

struct sym;

struct tcompound {
  uint16_t n;
  uint16_t v[0];
};

extern ary<tcompound *, t_compound> tcompounds;

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
