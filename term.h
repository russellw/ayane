enum {
  a_compound,

  a_distinct_object,
  a_fn,
  a_int,
  a_rat,
  a_real,
  a_var,
  a_basic,
};

enum {
  b_false,
  b_true,

  b_all,
  b_and,
  b_not,
};

inline w tag(void *p, w a) { return (w)p | a; }

struct int_t {
  mpz_t val;

  unsigned hash() { return mpz_get_ui(val); }

  bool eq(int_t *x) { return !mpz_cmp(val, x->val); }

  void clear() { mpz_clear(val); }
};

struct rat_t {
  mpq_t val;

  unsigned hash() {
    return mpz_get_ui(mpq_numref(val)) ^ mpz_get_ui(mpq_denref(val));
  }

  bool eq(rat_t *x) const { return mpq_equal(val, x->val); }

  void clear() { mpq_clear(val); }
};

struct cterm_t {
  uint16_t n;
  w v[0];

  bool eq(const w *p, w n) const {
    if (this->n != n)
      return false;
    return !memcmp(v, p, n * sizeof(w));
  }

  static cterm_t *store(const w *p, w n) {
    auto r = (cterm_t *)xmalloc(offsetof(cterm_t, v) + n * sizeof(w));
    r->n = n;
    memcpy(r->v, p, n * sizeof(w));
    return r;
  }

  static w process(cterm_t *x) { return tag(x, a_compound); }
};

struct fn_t {
  type t;
  sym *name;
};

inline int_t *intp(w a) {
  assert((a & 7) == a_int);
  return (int_t *)(a & ~(w)7);
}

inline rat_t *ratp(w a) {
  assert((a & 7) == a_rat || (a & 7) == a_real);
  return (rat_t *)(a & ~(w)7);
}

w int1(int_t *x);
w rat(rat_t *x);
w real(rat_t *x);

int_t *intp(w a);
rat_t *ratp(w a);

w fn(type t);
w fn(type t, sym *name);

inline fn_t *fnp(w a) {
  assert((a & 7) == a_fn);
  return (fn_t *)(a & ~(w)7);
}

inline cterm_t *ctermp(w a) {
  assert((a & 7) == a_compound);
  assert(!a_compound);
  return (cterm_t *)a;
}

inline w at(w a, w i) { return ctermp(a)->v[i]; }

inline w size(w a) { return ctermp(a)->n; }

inline w term(w op) { return op << 3 | a_basic; }

w term(vec<w> &v);
w term(w op, w a);
w term(w op, w a, w b);

inline w var(type t, w i) {
  assert(!isctype(t));
  return i << (8 * sizeof(type) + 3) | t << 3 | a_var;
}

inline w vari(w a) {
  assert((a & 7) == a_var);
  return a >> (8 * sizeof(type) + 3);
}
