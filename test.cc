#include "main.h"

#ifdef DEBUG
namespace {
void test_static_vec() {
  static static_vec<char> v;
  assert(v.n == 1);

  v.push('a');
  assert(v.n == 2);
  assert(v.back() == 'a');

  v.push('b');
  assert(v.n == 3);
  assert(v.back() == 'b');

  v.push('c');
  assert(v.n == 4);
  assert(v.back() == 'c');
}

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

  vec<char, 1> w;
  assert(w.n == 0);

  w.push('a');
  assert(w.n == 1);
  assert(w.back() == 'a');

  w.push('b');
  assert(w.n == 2);
  assert(w.back() == 'b');

  w.push('c');
  assert(w.n == 3);
  assert(w.back() == 'c');
}

void test_type() {
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
  auto t = ctype(t_predicate_int_int);
  assert(t->n == 3);
  assert(t->v[0] == t_bool);
  assert(t->v[1] == t_int);
  assert(t->v[2] == t_int);

  v.n = 0;
  v.push(t_bool);
  v.push(t_rat);
  v.push(t_rat);
  auto t_predicate_rat_rat = type(v.p, v.n);
  assert(t_predicate_rat_rat == type(v.p, v.n));
  assert(isctype(t_predicate_rat_rat));
  t = ctype(t_predicate_rat_rat);
  assert(t->n == 3);
  assert(t->v[0] == t_bool);
  assert(t->v[1] == t_rat);
  assert(t->v[2] == t_rat);
}

void test_sym() {
  assert(keyword(intern("ax")) == k_ax);
  assert(keyword(intern("cnf")) == k_cnf);
  assert(keyword(intern("p")) == k_p);
  assert(intern("xyz") == intern("xyz", 3));
}

void test_int() {
  int_t x1;
  mpz_init_set_ui(x1.val, 1);
  auto a1 = int1(&x1);
  auto y1 = intp(a1);
  assert(!mpz_cmp_ui(y1->val, 1));

  int_t x2;
  mpz_init_set_ui(x2.val, 2);
  auto a2 = int1(&x2);
  auto y2 = intp(a2);
  assert(!mpz_cmp_ui(y2->val, 2));

  int_t x3;
  mpz_init(x3.val);
  mpz_add(x3.val, y1->val, y2->val);
  auto a3 = int1(&x3);
  auto y3 = intp(a3);
  assert(!mpz_cmp_ui(y3->val, 3));
}

void test_rat() {
  rat_t x1;
  mpq_init(x1.val);
  mpq_set_ui(x1.val, 1, 1);
  auto a1 = rat(&x1);
  auto y1 = ratp(a1);
  assert(!mpq_cmp_ui(y1->val, 1, 1));

  rat_t x2;
  mpq_init(x2.val);
  mpq_set_ui(x2.val, 2, 1);
  auto a2 = rat(&x2);
  auto y2 = ratp(a2);
  assert(!mpq_cmp_ui(y2->val, 2, 1));

  rat_t x3;
  mpq_init(x3.val);
  mpq_add(x3.val, y1->val, y2->val);
  auto a3 = rat(&x3);
  auto y3 = ratp(a3);
  assert(!mpq_cmp_ui(y3->val, 3, 1));
}
} // namespace

void test() {
  test_sym();
  test_int();
  test_rat();
  test_type();
  test_vec();
  test_static_vec();
}
#endif
