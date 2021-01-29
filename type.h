struct sym;

typedef uint16_t ty;
const ty t_compound = 1 << 15;

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
  ty n;
  ty v[0];
};

extern vec<TCompound *> tcompounds;

inline ty istcompound(ty t) { return t & t_compound; }

inline TCompound *tcompoundp(ty t) {
  assert(istcompound(t));
  return tcompounds[t & ~t_compound];
}

ty type(sym *name);
ty type(const vec<ty> &v);
ty typeof(w a);
