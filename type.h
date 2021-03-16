enum class type : uint16_t {
  none,

  Bool,
  Int,
  Rat,
  Real,
  Individual,

  max
};

const uint16_t tcompoundOffset = 1000;

inline bool isCompound(type t) { return (uint16_t)t >= tcompoundOffset; }

struct tcompound {
  uint16_t n;
  type v[0];
};

extern ary<tcompound *, 0x10000 - tcompoundOffset> tcompounds;

inline tcompound *tcompoundp(type t) {
  assert(isCompound(t));
  return tcompounds[(uint16_t)t - tcompoundOffset];
}

struct sym;

type internType(sym *name);
type internType(const vec<type> &v);
type internType(type r, type t1);

enum class term : uint64_t;

void defaultType(type t, term a);
void requireType(type t, term a);

type typeof(term a);
type typeofNum(term a);
