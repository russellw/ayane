struct sym;

typedef uint16_t type;
const int t_bits = 15;
const type t_compound = 1 << t_bits;

enum { t_bool, t_int, t_rat, t_real, t_individual, basic_types };

struct ctype_t {
  type n;
  type v[0];
};

extern vec<ctype_t *> ctypes;

inline type isctype(type t) { return t & t_compound; }

inline ctype_t *ctypep(type t) {
  assert(isctype(t));
  return ctypes[t & ~t_compound];
}

type atype(sym *name);
type ctype(const vec<type> &v);
type typeof(w a);
