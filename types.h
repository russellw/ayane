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

extern ary<tcompound *> tcompounds;

inline tcompound *tcompoundp(w t) {
  assert(t & t_compound);
  return tcompounds[t & ~t_compound];
}

// SORT
void defaulttype(w t, w a);
w numtype(w a);
void requiretype(w t, w a);
w type(const vec<uint16_t> &v);
w type(sym *name);
w type(w r, w t1);
w typeof(w a);
///
