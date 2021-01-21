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

void test_types() {
  auto bird = type(intern("bird"));
  assert(bird == type(intern("bird")));
  assert(!isctype(bird));

  auto plane = type(intern("plane"));
  assert(plane == type(intern("plane")));
  assert(!isctype(plane));

  assert(bird != plane);

  vec<ty> v;
  v.push(t_bool);
  v.push(t_int);
  v.push(t_int);
  auto t_predicate_int_int = type(v.p, v.n);
  assert(t_predicate_int_int == type(v.p, v.n));
  assert(isctype(t_predicate_int_int));
  auto T = ctype(t_predicate_int_int);
  assert(T->n == 3);
  assert(T->v[0] == t_bool);
  assert(T->v[1] == t_int);
  assert(T->v[2] == t_int);

  v.n = 0;
  v.push(t_bool);
  v.push(t_rat);
  v.push(t_rat);
  auto t_predicate_rat_rat = type(v.p, v.n);
  assert(t_predicate_rat_rat == type(v.p, v.n));
  assert(isctype(t_predicate_rat_rat));
  T = ctype(t_predicate_rat_rat);
  assert(T->n == 3);
  assert(T->v[0] == t_bool);
  assert(T->v[1] == t_rat);
  assert(T->v[2] == t_rat);
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
  test_types();
  test_vec();
}
#endif
