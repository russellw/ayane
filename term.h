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
  // END
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
  // END
};

inline w tag(void *p, w a) { return (w)p | a; }

// SORT
struct Compound {
  uint16_t n;
  w v[0];

  bool eq(const w *p, w m) const {
    if (n != m)
      return false;
    return !memcmp(v, p, m * sizeof(w));
  }

  static Compound *store(const w *p, w n) {
    auto r = (Compound *)xmalloc(offsetof(Compound, v) + n * sizeof(w));
    r->n = n;
    memcpy(r->v, p, n * sizeof(w));
    return r;
  }

  static w process(Compound *x) { return tag(x, a_compound); }
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
// END

inline Int *intp(w a) {
  assert((a & 7) == a_int);
  return (Int *)(a & ~(w)7);
}

inline Rat *ratp(w a) {
  assert((a & 7) == a_rat || (a & 7) == a_real);
  return (Rat *)(a & ~(w)7);
}

w int1(Int *x);
w rat(Rat *x);
w real(Rat *x);

Int *intp(w a);
Rat *ratp(w a);

inline sym *symp(w a) {
  assert((a & 7) == a_sym);
  return (sym *)(a & ~(w)7);
}

inline Compound *compoundp(w a) {
  assert((a & 7) == a_compound);
  assert(!a_compound);
  return (Compound *)a;
}

inline w at(w a, w i) { return compoundp(a)->v[i]; }

inline w size(w a) { return compoundp(a)->n; }

inline w basic(w op) { return op << 3 | a_basic; }

w term(const vec<w> &v);
w term(w op, w a);
w term(w op, w a, w b);

const w alt = (w)1 << (16 + 3);

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

w imp(w a, w b);
