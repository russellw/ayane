#include "main.h"

#ifdef DEBUG
namespace {
void test_vec() {
  vec<char> v;
  assert(v.n == 0);

  v.push('a');
  assert(v.n == 1);
  assert(v.back() == 'a');

  v.push('b');
  assert(v.n == 2);
  assert(v.back() == 'b');

  v.push('c');
  assert(v.n == 3);
  assert(v.back() == 'c');
}

void test_sym() {
  assert(keyword(intern("ax")) == k_ax);
  assert(keyword(intern("cnf")) == k_cnf);
  assert(keyword(intern("p")) == k_p);
  assert(intern("xyz") == intern("xyz", 3));
}

void test_gmp() {
  mpz_t a;
  mpz_init_set_ui(a, 1);
  mpz_t b;
  mpz_init_set_ui(b, 2);
  mpz_t r;
  mpz_init(r);
  mpz_add(r, a, b);
  assert(mpz_get_ui(r) == 3);
  mpz_clear(a);
  mpz_clear(b);
  mpz_clear(r);
}
} // namespace

void test() {
  test_gmp();
  test_sym();
  test_vec();
}
#endif
