typedef uint16_t ty;

enum { t_bool, t_int, t_rat, t_real, t_individual, basic_types };

// compound types
const ty ctype_tag = 1 << 15;

struct ctype_t {
  ty n;
  ty v[999];
};

extern vec<ctype_t *> ctypes;

inline int isctype(ty t) { return t & ctype_tag; }

inline ctype_t *ctype(ty t) {
  assert(isctype(t));
  return ctypes[t & ~ctype_tag];
}

ty type(const sym *name);
ty type(const ty *p, int n);
