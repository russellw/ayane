#include "main.h"

#ifdef DEBUG
namespace {
void test_vec() {
  vec<char> u;
  assert(u.n == 0);

  u.push('a');
  assert(u.n == 1);
  assert(u.back() == 'a');

  u.push('b');
  assert(u.n == 2);
  assert(u.back() == 'b');

  u.push('c');
  assert(u.n == 3);
  assert(u.back() == 'c');

  vec<char, 1> v;
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

void test_type() {
  auto bird = atype(intern("bird"));
  assert(bird == atype(intern("bird")));
  assert(!isctype(bird));

  auto plane = atype(intern("plane"));
  assert(plane == atype(intern("plane")));
  assert(!isctype(plane));

  assert(bird != plane);

  vec<type> v;
  v.push(t_bool);
  v.push(t_int);
  v.push(t_int);
  auto t_predicate_int_int = ctype(v.p, v.n);
  assert(t_predicate_int_int == ctype(v.p, v.n));
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
  auto t_predicate_rat_rat = ctype(v.p, v.n);
  assert(t_predicate_rat_rat == ctype(v.p, v.n));
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

void test_typeof() {
  assert(typeof(var(t_int, 13)) == t_int);

  int_t i1;
  mpz_init_set_ui(i1.val, 1);
  assert(typeof(int1(&i1)) == t_int);

  rat_t r1;
  mpq_init(r1.val);
  mpq_set_ui(r1.val, 1, 3);
  assert(typeof(rat(&r1)) == t_rat);
  assert(typeof(real(&r1)) == t_real);

  auto red = fn(t_bool, intern("red"));
  assert(typeof(red) == t_bool);
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

void test_term() {
  auto red = fn(t_bool, intern("red"));
  auto green = fn(t_bool, intern("green"));
  auto blue = fn(t_bool, intern("blue"));

  auto a = term(b_not, red);
  assert(a == term(b_not, red));

  a = term(b_and, red, green);
  assert(a == term(b_and, red, green));

  vec<w> v;
  v.push(term(b_and));
  v.push(red);
  v.push(green);
  v.push(blue);
  a = term(v);
  assert(a == term(v));
}

void test_fn() {
  auto blank = fn(t_bool);
  auto blankp = fnp(blank);
  assert(blankp->t == t_bool);

  auto red = fn(t_bool, intern("red"));
  auto redp = fnp(red);
  assert(redp->t == t_bool);

  auto green = fn(t_bool, intern("green"));
  auto greenp = fnp(green);
  assert(greenp->t == t_bool);

  auto blue = fn(t_bool, intern("blue"));
  auto bluep = fnp(blue);
  assert(bluep->t == t_bool);

  assert(!blankp->name);
  assert(redp->name == intern("red"));
  assert(greenp->name == intern("green"));
  assert(bluep->name == intern("blue"));
}

void test_var() {
  auto x = var(t_int, 0);
  assert(vari(x) == 0);

  auto y = var(t_int, 1);
  assert(vari(y) == 1);

  assert(x != y);
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
  test_typeof();
  test_vec();
  test_fn();
  test_term();
  test_var();
}
#endif
