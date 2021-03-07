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

Int *intern(Int &x);
Rat *intern(Rat &x);
