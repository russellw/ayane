typedef uint32_t term;

enum {
  a_false,
  a_true,
  a_int,
  a_rat,
  a_real,
  a_fn,
  a_var,
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
};

term fn(sym *name);
