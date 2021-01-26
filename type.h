struct sym;

typedef uint16_t ty;
const int t_bits = 15;
const ty t_compound = 1 << t_bits;

enum { t_bool, t_int, t_rat, t_real, t_individual, basic_types };

struct ctype_t {
  ty n;
  ty v[0];
};

extern vec<ctype_t *> ctypes;

inline ty isctype(ty t) { return t & t_compound; }

inline ctype_t *ctypep(ty t) {
  assert(isctype(t));
  return ctypes[t & ~t_compound];
}

ty atype(sym *name);
ty ctype(const vec<ty> &v);
ty typeof(w a);
