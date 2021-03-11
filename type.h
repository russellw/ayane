enum class type : uint16_t {
  none,

  Bool,
  Int,
  Rat,
  Real,
  Individual,

  max
};

const uint16_t tcompoundoffset = 1000;

inline bool iscompound(type t) { return (uint16_t)t >= tcompoundoffset; }

struct tcompound {
  uint16_t n;
  type v[0];
};

extern ary<tcompound *, 0x10000 - tcompoundoffset> tcompounds;

inline tcompound *tcompoundp(type t) {
  assert(iscompound(t));
  return tcompounds[(uint16_t)t - tcompoundoffset];
}

struct sym;

type mktype(sym *name);
type mktype(const vec<type> &v);
type mktype(type r, type t1);

enum class term : uint64_t;

void defaulttype(type t, term a);
void requiretype(type t, term a);

type typeof(term a);
type numtypeof(term a);
