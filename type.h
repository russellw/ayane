enum class type : uint16_t {
  none,

  Bool,
  Int,
  Rat,
  Real,
  Individual,

  max
};

const uint16_t t_compound = 1 << 15;

inline bool iscompound(type t) { return (uint16_t)t >= t_compound; }

struct sym;

struct tcompound {
  uint16_t n;
  type v[0];
};

extern ary<tcompound *, 0x10000 - t_compound> tcompounds;

inline tcompound *tcompoundp(type t) {
  assert(iscompound(t));
  return tcompounds[(uint16_t)t - t_compound];
}

// SORT
void defaulttype(type t, w a);
type mktype(const vec<type> &v);
type mktype(sym *name);
type mktype(type r, type t1);
type numtype(w a);
void requiretype(type t, w a);
type typeof(w a);
///
