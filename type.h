enum { t_bool, t_int, t_rat, t_real, t_individual, basic_types };

// compound types
const int t_bits = 15;
const ty t_compound = 1 << t_bits;

struct ctype_t {
  ty n;
  ty v[999];
};

extern static_vec<ctype_t *, t_compound> ctypes;

inline ty isctype(ty t) { return t & t_compound; }

inline ctype_t *ctype(ty t) {
  assert(isctype(t));
  return ctypes[t & ~t_compound];
}

ty type(const sym *name);
ty type(const ty *p, size_t n);
