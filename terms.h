typedef uint32_t term;

enum {
  a_false,
  a_true,

  a_all,

  a_shift = 28,
  a_mask = 7 << a_shift,

  a_distinct_object = 1 << a_shift,
  a_fn = 2 << a_shift,
  a_int = 3 << a_shift,
  a_rat = 4 << a_shift,
  a_real = 5 << a_shift,
  a_var = 6 << a_shift,

  a_compound = 1 << 31,
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

term fn(sym *name);
