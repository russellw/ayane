enum {
  a_compound,

  // SORT
  a_basic,
  a_distinctobj,
  a_int,
  a_rat,
  a_real,
  a_sym,
  a_var,
  ///
};

enum {
  b_false,
  b_true,

  // SORT
  b_add,
  b_all,
  b_and,
  b_ceil,
  b_div,
  b_dive,
  b_divf,
  b_divt,
  b_eq,
  b_eqv,
  b_exists,
  b_floor,
  b_isint,
  b_israt,
  b_le,
  b_lt,
  b_minus,
  b_mul,
  b_not,
  b_or,
  b_reme,
  b_remf,
  b_remt,
  b_round,
  b_sub,
  b_toint,
  b_torat,
  b_toreal,
  b_trunc,
  ///
};

struct compound {
  si n;
  w v[0];
};

struct sym;

class term {
  si x;

public:
  explicit term(si x) : x(x) {}

  // SORT
  bool operator==(term b) { return x == b.x; }

  w operator[](si i) {
    assert(0 <= i);
    assert(i < size());
    return compoundp()->v[i];
  }

  w *begin() { return compoundp()->v; }

  compound *compoundp() {
    assert(tag() == a_compound);
    return (compound *)(x - a_compound);
  }

  sym *distinctobjp() {
    assert(tag() == a_distinctobj);
    return (sym *)(x - a_distinctobj);
  }

  w *end() {
    auto p = compoundp();
    return p->v + p->n;
  }

  Int *intp() {
    assert(tag() == a_int);
    return (Int *)(x - a_int);
  }

  Rat *ratp() {
    assert(tag() == a_rat || tag() == a_real);
    return (Rat *)(x - tag());
  }

  si size() { return compoundp()->n; }

  sym *symp() {
    assert(tag() == a_sym);
    return (sym *)(x - a_sym);
  }

  si tag() { return x & 7; }

  si vari() {
    assert(tag() == a_var);
    return x >> (1 + 16 + 3);
  }

  w vartype() {
    assert(tag() == a_var);
    return x >> 3 & 0xffff;
  }
  ///
};

// SORT
extern ary<w> freevars;
extern si skolemi;
///

// SORT
void getfree(w a);
w imp(w a, w b);
void init_terms();
w int1(Int &x);
Int *intp(w a);
w mk(const ary<w> &v);
w mk(const vec<w> &v);
w mk(w op, w a);
w mk(w op, w a, w b);
w rat(Rat &x);
Rat *ratp(w a);
w real(Rat &x);
///

inline compound *compoundp(w a) {
  assert((a & 7) == a_compound);
  assert(!a_compound);
  return (compound *)a;
}

// SORT
inline w at(w a, si i) { return compoundp(a)->v[i]; }

inline w basic(w op) { return op << 3 | a_basic; }

inline w *beginp(w a) { return compoundp(a)->v; }

inline sym *distinctobjp(w a) {
  assert((a & 7) == a_distinctobj);
  return (sym *)(a - a_distinctobj);
}

inline w *endp(w a) {
  auto p = compoundp(a);
  return p->v + p->n;
}

inline Int *intp(w a) {
  assert((a & 7) == a_int);
  return (Int *)(a - a_int);
}

inline Rat *ratp(w a) {
  assert((a & 7) == a_rat || (a & 7) == a_real);
  return (Rat *)(a & ~(w)7);
}

inline si size(w a) { return compoundp(a)->n; }

inline sym *symp(w a) {
  assert((a & 7) == a_sym);
  return (sym *)(a - a_sym);
}

inline w tag(void *p, w a) { return (w)p + a; }

inline w var(type t, si i) {
  assert(!iscompound(t));
  if (sizeof(w) == 4 && i >= 1 << 12)
    err("too many variables");
  // variable is composed of:
  // 3 bits tag
  // 16 bits type
  // 1 bit flag e.g. for renamed variables
  // and the rest of the bits in the word are for a number that identifies the
  // variable
  return i << (1 + 16 + 3) | (si)t << 3 | a_var;
}

inline si vari(w a) {
  assert((a & 7) == a_var);
  return a >> (1 + 16 + 3);
}

inline type vartype(w a) {
  assert((a & 7) == a_var);
  return type(a >> 3);
}
///

#ifdef DEBUG
void ck(w a);
#else
#define ck(a)
#endif
