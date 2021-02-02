struct sym;

const w t_compound = 1 << 15;

enum {
  t_none,

  t_bool,
  t_int,
  t_rat,
  t_real,
  t_individual,

  basic_types
};

struct TCompound {
  uint16_t n;
  uint16_t v[0];
};

extern vec<TCompound *> tcompounds;

inline w istcompound(w t) { return t & t_compound; }

inline TCompound *tcompoundp(w t) {
  assert(istcompound(t));
  return tcompounds[t & ~t_compound];
}

w type(sym *name);
w type(const vec<uint16_t> &v);
w type(w r, w t1);
