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

// SORT
struct compound {
  si n;
  w v[0];
};

struct sym {
  // type named by this symbol
  uint16_t t;

  // type of function named by this symbol
  uint16_t ft;

  // for the keyword system to work, the size of the declared character array
  // must be large enough to hold the longest keyword

  // for the system to work efficiently, the size of the whole structure must be
  // a power of 2

  // when symbols are allocated on the heap, the code doing the allocation is
  // responsible for allocating enough space to hold the corresponding strings
  char v[0x20 - 2 * sizeof(uint16_t)];
};
///

extern sym keywords[];

// SORT
extern ary<w> freevars;
extern si skolemi;
///

// SORT
void getfree(w a);
w imp(w a, w b);
void init_terms();
w int1(Int &x);
sym *intern(const char *s, si n);
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

// this logically belongs in types.h but is moved up here for the benefit of
// inline functions that want to use it
const w t_compound = 1 << 15;

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

inline sym *intern(const char *s) { return intern(s, strlen(s)); }

inline Int *intp(w a) {
  assert((a & 7) == a_int);
  return (Int *)(a - a_int);
}

inline si keyword(const sym *s) {
  // turn a symbol into a keyword number by subtracting the base of the keyword
  // array and dividing by the declared size of a symbol structure (which is
  // efficient as long as that size is a power of 2)

  // it's okay if the symbol is not a keyword; that just means the resulting
  // number will not correspond to any keyword and will not match any case in a
  // switch statement
  size_t i = (const char *)s - (const char *)keywords;
  return i / sizeof(sym);
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

inline w var(w t, si i) {
  assert(t < t_compound);
  if (sizeof(w) == 4 && i >= 1 << 12)
    err("too many variables");
  // variable is composed of:
  // 3 bits tag
  // 16 bits type
  // 1 bit flag e.g. for renamed variables
  // and the rest of the bits in the word are for a number that identifies the
  // variable
  return i << (1 + 16 + 3) | t << 3 | a_var;
}

inline si vari(w a) {
  assert((a & 7) == a_var);
  return a >> (1 + 16 + 3);
}

inline w vartype(w a) {
  assert((a & 7) == a_var);
  return a >> 3 & 0xffff;
}
///

#ifdef DEBUG
void ckterm(w a);
#else
#define ckterm(a)
#endif
