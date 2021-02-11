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
  i_none,
#define _(s) i_##s,
#include "infer.h"
#undef _
};

enum {
  s_none,
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

inline w tag(void *p, w a) { return (w)p + a; }
///

// SORT
struct Int {
  mpz_t val;

  unsigned hash() const { return mpz_get_ui(val); }
  bool eq(const Int &x) const { return !mpz_cmp(val, x.val); }
  void clear() { mpz_clear(val); }
};

struct Rat {
  mpq_t val;

  unsigned hash() const {
    return mpz_get_ui(mpq_numref(val)) ^ mpz_get_ui(mpq_denref(val));
  }
  bool eq(const Rat &x) const { return mpq_equal(val, x.val); }
  void clear() { mpq_clear(val); }
};

struct clause {
  // is this a first-order formula rather than an actual clause?
  // a first-order formula is represented like a clause with just one positive
  // literal, but the literal can be any first-order predicate
  bool fof;

  // which inference rule derived it?
  uint8_t infer;

  // number of negative and total literals
  // the literals are laid out in an array, negative then positive
  // cannot represent a clause with more than 255 literals; this is okay because
  // such a clause would make no useful contribution to a proof search anyway
  uint8_t nn, n;

  // the majority of ways for a clause to be made, result in it deriving from
  // zero or one other clauses, but the majority of clauses will be made by the
  // superposition rule, so derived from two other clauses
  clause *from[2];

  // literals
  w v[0];
};

struct compound {
  w n;
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

struct tcompound {
  uint16_t n;
  uint16_t v[0];
};
///

// SORT
extern ary<tcompound *> tcompounds;
extern ary<w> freevars;
extern ary<w> neg, pos;
extern bool complete;
extern clause *conjecture;
extern const char *infernames[];
extern const char *szs[];
extern sym keywords[];
extern unordered_map<const clause *, const char *> clausefiles;
extern unordered_map<const clause *, const char *> clausenames;
extern w skolemi;
///

#ifdef DEBUG
extern w status;
#endif

// SORT
clause *clause1(w infer, clause *from = 0, clause *from1 = 0);
const char *clausename(const clause *c);
void clear();
clause *formula(w infer, w a, clause *from = 0);
void getfree(w a);
w imp(w a, w b);
w int1(Int &x);
sym *intern(const char *s, w n);
Int *intp(w a);
w rat(Rat &x);
Rat *ratp(w a);
w real(Rat &x);
w term(const ary<w> &v);
w term(const vec<w> &v);
w term(w op, w a);
w term(w op, w a, w b);
w tmp(w op, const vec<w> &v);
w type(const vec<uint16_t> &v);
w type(sym *name);
w type(w r, w t1);
///

inline compound *compoundp(w a) {
  assert((a & 7) == a_compound);
  assert(!a_compound);
  return (compound *)a;
}

// SORT
inline w at(w a, w i) { return compoundp(a)->v[i]; }

inline w basic(w op) { return op << 3 | a_basic; }

inline void fpr(FILE *f, const sym *s) { fpr(f, s->v); }

inline sym *intern(const char *s) { return intern(s, strlen(s)); }

inline Int *intp(w a) {
  assert((a & 7) == a_int);
  return (Int *)(a - a_int);
}

inline size_t keyword(const sym *s) {
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

inline w size(w a) { return compoundp(a)->n; }

inline sym *symp(w a) {
  assert((a & 7) == a_sym);
  return (sym *)(a - a_sym);
}

inline tcompound *tcompoundp(w t) {
  assert(istcompound(t));
  return tcompounds[t & ~t_compound];
}

inline w var(w t, w i) {
  assert(!istcompound(t));
  if (sizeof(w) == 4 && i >= 1 << 12)
    err("too many variables");
  return i << (1 + 16 + 3) | t << 3 | a_var;
}

inline w vari(w a) {
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
