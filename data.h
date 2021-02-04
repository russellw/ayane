// SORT
const w alt = (w)1 << (16 + 3);
const w t_compound = 1 << 15;
///

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

enum {
#define _(s) s_##s,
#include "szs.h"
#undef _
  n_szs
};

enum {
  t_none,

  t_bool,
  t_int,
  t_rat,
  t_real,
  t_individual,

  basic_types
};

// SORT
inline bool istcompound(w t) {
  assert(t < 0x10000);
  return t & t_compound;
}

inline w tag(void *p, w a) { return (w)p | a; }
///

// SORT
struct Compound {
  uint16_t n;
  w v[0];
};

struct Int {
  mpz_t val;

  unsigned hash() { return mpz_get_ui(val); }
  bool eq(Int *x) { return !mpz_cmp(val, x->val); }
  void clear() { mpz_clear(val); }
};

struct Rat {
  mpq_t val;

  unsigned hash() {
    return mpz_get_ui(mpq_numref(val)) ^ mpz_get_ui(mpq_denref(val));
  }
  bool eq(Rat *x) const { return mpq_equal(val, x->val); }
  void clear() { mpq_clear(val); }
};

struct Sym {
  // Type named by this symbol
  uint16_t t;

  // Type of function named by this symbol
  uint16_t ft;

  // For the keyword system to work, the size of the declared character array
  // must be large enough to hold the longest keyword

  // For the system to work efficiently, the size of the whole structure must be
  // a power of 2

  // When symbols are allocated on the heap, the code doing the allocation is
  // responsible for allocating enough space to hold the corresponding strings
  char v[0x20 - 2 * sizeof(uint16_t)];
};

struct TCompound {
  uint16_t n;
  uint16_t v[0];
};
///

// SORT
extern Sym keywords[];
extern bool conjecture;
extern const char *szs[];
extern vec<TCompound *> tcompounds;
extern vec<w> neg, pos;
///

#ifdef DEBUG
extern w status;
#endif

// SORT
void clause();
void clear();
w imp(w a, w b);
w int1(Int *x);
Sym *intern(const char *s, w n);
Int *intp(w a);
w rat(Rat *x);
Rat *ratp(w a);
w real(Rat *x);
w term(const vec<w> &v);
w term(w op, w a);
w term(w op, w a, w b);
w type(Sym *name);
w type(const vec<uint16_t> &v);
w type(w r, w t1);
///

inline Compound *compoundp(w a) {
  assert((a & 7) == a_compound);
  assert(!a_compound);
  return (Compound *)a;
}

// SORT
inline w at(w a, w i) { return compoundp(a)->v[i]; }

inline w basic(w op) { return op << 3 | a_basic; }

inline void fpr(FILE *f, const Sym *S) { fpr(f, S->v); }

inline Sym *intern(const char *s) { return intern(s, strlen(s)); }

inline Int *intp(w a) {
  assert((a & 7) == a_int);
  return (Int *)(a - a_int);
}

inline size_t keyword(const Sym *S) {
  // Turn a symbol into a keyword number by subtracting the base of the keyword
  // array and dividing by the declared size of a symbol structure (which is
  // efficient as long as that size is a power of 2)

  // It's okay if the symbol is not a keyword; that just means the resulting
  // number will not correspond to any keyword and will not match any case in a
  // switch statement
  size_t i = (const char *)S - (const char *)keywords;
  return i / sizeof(Sym);
}

inline Rat *ratp(w a) {
  assert((a & 7) == a_rat || (a & 7) == a_real);
  return (Rat *)(a & ~(w)7);
}

inline w size(w a) { return compoundp(a)->n; }

inline Sym *symp(w a) {
  assert((a & 7) == a_sym);
  return (Sym *)(a - a_sym);
}

inline TCompound *tcompoundp(w t) {
  assert(istcompound(t));
  return tcompounds[t & ~t_compound];
}

inline w var(w t, w i) {
  assert(!istcompound(t));
  if (sizeof(w) == 4 && i >= 1 << 12)
    throw "Too many variables";
  return i << (1 + 16 + 3) | t << 3 | a_var;
}

inline w vari(w a) {
  assert((a & 7) == a_var);
  return a >> (1 + 16 + 3);
}
///
