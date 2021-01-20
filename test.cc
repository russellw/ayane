#include "main.h"

#ifdef DEBUG
namespace {
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

void test() { test_gmp(); }
#endif
