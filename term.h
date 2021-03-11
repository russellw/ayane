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

const si tagbits = 6;

inline term tag(term a) { return term((uint64_t)a & (1 << tagbits) - 1); }

inline uint64_t rest(term a) { return (uint64_t)a >> tagbits; }

inline bool iscompound(term a) { return tag(a) > term::Var; }

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

struct sym;

// SORT
extern ary<term> freevars;
extern si skolemi;
///

// SORT
void getfree(term a);
term imp(term a, term b);
void init_terms();
term mk(term op, const ary<term> &args);
term mk(term op, const vec<term> &args);
term mk(term op, term a);
term mk(term op, term a, term b);
term mk(term op, term a, term b, term c);
///

// SORT
inline term mk(void *p, term a) {
  // this assumes there is a limit to how much address space will actually be
  // used, such that pointers don't actually use the full 64 bits, leaving a few
  // spare for tag
  assert((uint64_t)p < (uint64_t)1 << 64 - tagbits);
  return term((uint64_t)p << tagbits | (uint64_t)a);
}

inline term var(type t, si i) {
  assert(!iscompound(t));
  // variable is composed of:
  // 6 bits tag
  // 16 bits type
  // 1 bit flag e.g. for renamed variables
  // and the rest of the bits are for a number that identifies the variable
  return term((uint64_t)i << (1 + 16 + tagbits) | (uint64_t)t << tagbits |
              (uint64_t)term::Var);
}

inline si vari(term a) {
  assert(tag(a) == term::Var);
  return (uint64_t)a >> (1 + 16 + tagbits);
}

inline type vartype(term a) {
  assert(tag(a) == term::Var);
  return type((uint64_t)a >> tagbits);
}
///

#ifdef DEBUG
void ck(term a);
#else
#define ck(a)
#endif
