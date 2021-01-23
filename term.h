enum {
  a_false,
  a_true,

  a_all,

  a_bits = 28,
  a_tags = 7 << a_bits,

  a_distinct_object = 1 << a_bits,
  a_fn = 2 << a_bits,
  a_int = 3 << a_bits,
  a_rat = 4 << a_bits,
  a_real = 5 << a_bits,
  a_var = 6 << a_bits,

  a_compound = 8 << a_bits
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

term int1(int_t *x);
term rat(rat_t *x);

int_t *intp(term a);
rat_t *ratp(term a);

struct fn_t {
  ty t;
  sym *name;
};

extern static_vec<fn_t *, 1 << a_bits> fns;

term fn(sym *name);

inline fn_t *fnp(term a) {
  assert((a & a_tags) == a_fn);
  auto r = fns[a & (1 << a_bits) - 1];
  assert(r);
  return r;
}
