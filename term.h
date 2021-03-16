enum class term : uint64_t {
  False,
  True,

  // atomic terms
  // SORT
  DistinctObj,
  Int,
  Rat,
  Real,
  Sym,
  Var,
  ///

  // compound terms
  // SORT
  Add,
  All,
  And,
  Call,
  Ceil,
  Div,
  DivE,
  DivF,
  DivT,
  Eq,
  Eqv,
  Exists,
  Floor,
  Imp,
  IsInt,
  IsRat,
  Le,
  Lt,
  Minus,
  Mul,
  Not,
  Or,
  RemE,
  RemF,
  RemT,
  Round,
  Sub,
  ToInt,
  ToRat,
  ToReal,
  Trunc,
  ///
};

const si tagBits = 6;

// SORT
extern ary<pair<term, term>> pairv;
extern ary<term> termv;
extern si skolemi;
///

void initTerms();

// make a term from a pointer
inline term tag(term a, void *p) {
  // this assumes there is a limit to how much address space will actually be
  // used, such that pointers don't actually use the full 64 bits, leaving a few
  // spare for tag
  assert((uint64_t)p < (uint64_t)1 << (64 - tagBits));
  return term((uint64_t)p << tagBits | (uint64_t)a);
}

// decompose term into components
inline term tag(term a) { return term((uint64_t)a & (1 << tagBits) - 1); }

inline uint64_t rest(term a) { return (uint64_t)a >> tagBits; }

// requires atomic term tag numbers to be contiguous
inline bool isCompound(term a) { return tag(a) > term::Var; }

// compound terms
struct compound {
  si n;
  term v[0];
};

inline si size(term a) {
  auto p = (compound *)rest(a);
  return p->n;
}

inline term *begin(term a) {
  auto p = (compound *)rest(a);
  return p->v;
}

inline term *end(term a) { return begin(a) + size(a); }

inline term at(term a, si i) {
  assert(0 <= i);
  assert(i < size(a));
  return begin(a)[i];
}

// temporary compound terms
compound *comp(si n);
term comp(term op, const vec<term> &v);
term comp(term op, term a);
term comp(term op, term a, term b);

// permanent/interned compound terms
term intern(term op, const ary<term> &v);
term intern(term op, const vec<term> &v);
term intern(term op, term a);
term intern(term op, term a, term b);
term intern(term op, term a, term b, term c);

// variables
inline term var(type t, si i) {
  assert(!isCompound(t));
  // variable is composed of:
  // 6 bits tag
  // 16 bits type
  // 1 bit flag e.g. for renamed variables
  // and the rest of the bits are for a number that identifies the variable
  return term((uint64_t)i << (1 + 16 + tagBits) | (uint64_t)t << tagBits |
              (uint64_t)term::Var);
}

inline si vari(term a) {
  assert(tag(a) == term::Var);
  return (uint64_t)a >> (1 + 16 + tagBits);
}

inline type varType(term a) {
  assert(tag(a) == term::Var);
  return type((uint64_t)a >> tagBits);
}

void getFreeVars(term a);

// check terms for structural consistency
#ifdef DEBUG
void check(term a);
#else
#define check(a)
#endif
