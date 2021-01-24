enum {
  a_distinct_object,
  a_fn,
  a_int,
  a_rat,
  a_real,
  a_var,
  a_compound,
  a_basic,
};

enum {
  b_false,
  b_true,

  b_all,
  b_and,
  b_not,
};

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

  bool eq(rat_t *x) { return mpq_equal(val, x->val); }

  void clear() { mpq_clear(val); }
};

struct cterm_t {
  uint16_t n;
  term v[0];
};

struct fn_t {
  ty t;
  sym *name;
};

inline size_t tag(term a) { return a & 7; }

inline term tag(void *p, int a) { return (size_t)p | a; }

inline int_t *intp(term a) {
  assert(tag(a) == a_int);
  return (int_t *)(a & ~(size_t)7);
}

inline rat_t *ratp(term a) {
  assert(tag(a) == a_rat || tag(a) == a_real);
  return (rat_t *)(a & ~(size_t)7);
}

term int1(int_t *x);
term rat(rat_t *x);

int_t *intp(term a);
rat_t *ratp(term a);

term fn(ty t);
term fn(ty t, sym *name);

inline fn_t *fnp(term a) {
  assert(tag(a) == a_fn);
  return (fn_t *)(a & ~(size_t)7);
}

inline cterm_t *ctermp(term a) {
  assert(tag(a) == a_compound);
  return (cterm_t *)(a & ~(size_t)7);
}

inline term aterm(int op) { return op << 3 | a_basic; }

term cterm(vec<term> &v);
term cterm(int op, term a);
term cterm(int op, term a, term b);
