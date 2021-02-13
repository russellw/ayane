#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

#ifdef DEBUG
namespace {
// SORT
w fn(w t, sym *s) {
  if (!s->ft)
    s->ft = t;
  assert(s->ft == t);
  return tag(s, a_sym);
}

bool match0(w a, w b) {
  unified.n = 0;
  return match(a, b);
}

w type(w r, w t1, w t2) {
  vec<uint16_t> v(r, t1, t2);
  return type(v);
}
///

// SORT
void test_clause() {
  auto x = var(t_int, 0);
  auto y = var(t_int, 1);

  neg.n = pos.n = 0;
  neg.push(term(basic(b_eq), x, y));
  auto c = mkclause(0);

  neg.n = pos.n = 0;
  neg.push(term(basic(b_eq), x, y));
  auto d = mkclause(0);
  assert(c == d);

  neg.n = pos.n = 0;
  pos.push(term(basic(b_eq), x, y));
  d = mkclause(0);
  assert(c != d);
}

void test_fn() {
  auto red = fn(t_bool, intern("red"));
  auto redp = symp(red);
  assert(redp->ft == t_bool);

  auto green = fn(t_bool, intern("green"));
  auto greenp = symp(green);
  assert(greenp->ft == t_bool);

  auto blue = fn(t_bool, intern("blue"));
  auto bluep = symp(blue);
  assert(bluep->ft == t_bool);

  assert(redp == intern("red"));
  assert(greenp == intern("green"));
  assert(bluep == intern("blue"));
}

void test_getfree() {
  auto a = fn(t_individual, intern("a"));
  auto x = var(t_individual, 0);
  auto y = var(t_individual, 1);

  getfree(a);
  assert(freevars.n == 0);

  getfree(x);
  assert(freevars.n == 1);
  assert(freevars[0] == x);

  getfree(term(basic(b_eq), x, x));
  assert(freevars.n == 1);
  assert(freevars[0] == x);

  getfree(term(basic(b_eq), x, y));
  assert(freevars.n == 2);
  assert(freevars[0] == x);
  assert(freevars[1] == y);

  getfree(term(basic(b_eq), a, a));
  assert(freevars.n == 0);

  getfree(term(basic(b_all), term(basic(b_eq), x, y), x));
  assert(freevars.n == 1);
  assert(freevars[0] == y);
}

void test_int() {
  Int x1;
  mpz_init_set_ui(x1.val, 1);
  auto a1 = int1(x1);
  auto y1 = intp(a1);
  assert(!mpz_cmp_ui(y1->val, 1));

  Int x2;
  mpz_init_set_ui(x2.val, 2);
  auto a2 = int1(x2);
  auto y2 = intp(a2);
  assert(!mpz_cmp_ui(y2->val, 2));

  Int x3;
  mpz_init(x3.val);
  mpz_add(x3.val, y1->val, y2->val);
  auto a3 = int1(x3);
  auto y3 = intp(a3);
  assert(!mpz_cmp_ui(y3->val, 3));
}

void test_match() {
  // subset of unify where only the first argument can be treated as a variable
  // to be matched against the second argument. applied to the same test cases
  // as unify, gives the same results in some cases, but different results in
  // others. in particular, has no notion of an occurs check; in actual use, it
  // is assumed that the arguments will have disjoint variables
  auto a = fn(t_individual, intern("a"));
  auto b = fn(t_individual, intern("b"));
  auto f1 = fn(type(t_individual, t_individual), intern("f1"));
  auto f2 = fn(type(t_individual, t_individual, t_individual), intern("f2"));
  auto g1 = fn(type(t_individual, t_individual), intern("g1"));
  auto x = var(t_individual, 0);
  auto y = var(t_individual, 1);
  auto z = var(t_individual, 2);

  // Succeeds. (tautology)
  assert(match0(a, a));
  assert(unified.n == 0);

  // a and b do not match
  assert(!match0(a, b));

  // Succeeds. (tautology)
  assert(match0(x, x));
  assert(unified.n == 0);

  // x is not matched with the constant a, because the variable is on the
  // right-hand side
  assert(!match0(a, x));

  // x and y are aliased
  assert(match0(x, y));
  assert(unified.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(match0(term(f2, a, x), term(f2, a, b)));
  assert(unified.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!match0(term(f1, a), term(g1, a)));

  // x and y are aliased
  assert(match0(term(f1, x), term(f1, y)));
  assert(unified.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!match0(term(f1, x), term(g1, y)));

  // Fails. The f function symbols have different arity
  assert(!match0(term(f1, x), term(f2, y, z)));

  // Does not match y with the term g1(x), because the variable is on the
  // right-hand side
  assert(!match0(term(f1, term(g1, x)), term(f1, y)));

  // Does not match, because the variable is on the right-hand side
  assert(!match0(term(f2, term(g1, x), x), term(f2, y, a)));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check) but returns true here because match has no
  // notion of an occurs check
  assert(match0(x, term(f1, x)));
  assert(unified.n == 1);

  // Both x and y are unified with the constant a
  assert(match0(x, y));
  assert(match(y, a));
  assert(unified.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails this time, because the variable is on the right-hand side
  assert(!match0(a, y));

  // Fails. a and b do not match, so x can't be unified with both
  assert(match0(x, a));
  assert(!match(b, x));
}

void test_rat() {
  Rat x1;
  mpq_init(x1.val);
  mpq_set_ui(x1.val, 1, 1);
  auto a1 = rat(x1);
  auto y1 = ratp(a1);
  assert(!mpq_cmp_ui(y1->val, 1, 1));

  Rat x2;
  mpq_init(x2.val);
  mpq_set_ui(x2.val, 2, 1);
  auto a2 = rat(x2);
  auto y2 = ratp(a2);
  assert(!mpq_cmp_ui(y2->val, 2, 1));

  Rat x3;
  mpq_init(x3.val);
  mpq_add(x3.val, y1->val, y2->val);
  auto a3 = rat(x3);
  auto y3 = ratp(a3);
  assert(!mpq_cmp_ui(y3->val, 3, 1));
}

void test_subsume() {
  auto a = fn(t_individual, intern("a"));
  auto a1 = fn(type(t_individual, t_individual), intern("a1"));
  auto b = fn(t_individual, intern("b"));
  auto p = fn(t_bool, intern("p"));
  auto p1 = fn(type(t_bool, t_individual), intern("p1"));
  auto p2 = fn(type(t_bool, t_individual, t_individual), intern("p2"));
  auto q = fn(t_bool, intern("q"));
  auto q1 = fn(type(t_bool, t_individual), intern("q1"));
  auto x = var(t_individual, 0);
  auto y = var(t_individual, 1);
  clause *c;
  clause *d;

  // false <= false
  c = mkclause(0);
  d = mkclause(0);
  assert(subsumes(c, d));

  // false <= p
  c = mkclause(0);
  pos.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p <= p
  pos.push(p);
  c = mkclause(0);
  pos.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));

  // !p <= !p
  neg.push(p);
  c = mkclause(0);
  neg.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));

  // p <= p | p
  pos.push(p);
  c = mkclause(0);
  pos.push(p);
  pos.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p !<= !p
  pos.push(p);
  c = mkclause(0);
  neg.push(p);
  d = mkclause(0);
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p | q <= q | p
  pos.push(p);
  pos.push(q);
  c = mkclause(0);
  pos.push(q);
  pos.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p | q <= p | q | p
  pos.push(p);
  pos.push(q);
  c = mkclause(0);
  pos.push(p);
  pos.push(q);
  pos.push(p);
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(a) | p(b) | q(a) | q(b) | <= p(a) | q(a) | p(b) | q(b)
  pos.push(term(p1, a));
  pos.push(term(p1, b));
  pos.push(term(q1, a));
  pos.push(term(q1, b));
  c = mkclause(0);
  pos.push(term(p1, a));
  pos.push(term(q1, a));
  pos.push(term(p1, b));
  pos.push(term(q1, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x,y) <= p(a,b)
  pos.push(term(p2, x, y));
  c = mkclause(0);
  pos.push(term(p2, a, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,x) !<= p(a,b)
  pos.push(term(p2, x, x));
  c = mkclause(0);
  pos.push(term(p2, a, b));
  d = mkclause(0);
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) <= p(y)
  pos.push(term(p1, x));
  c = mkclause(0);
  pos.push(term(p1, y));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(y) | p(a(y)) | p(a(a(y)))
  pos.push(term(p1, x));
  pos.push(term(p1, term(a1, x)));
  pos.push(term(p1, term(a1, term(a1, x))));
  c = mkclause(0);
  pos.push(term(p1, y));
  pos.push(term(p1, term(a1, y)));
  pos.push(term(p1, term(a1, term(a1, y))));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a) <= p(a) | p(b)
  pos.push(term(p1, x));
  pos.push(term(p1, a));
  c = mkclause(0);
  pos.push(term(p1, a));
  pos.push(term(p1, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) | p(a(x)) <= p(a(y)) | p(y)
  pos.push(term(p1, x));
  pos.push(term(p1, term(a1, x)));
  c = mkclause(0);
  pos.push(term(p1, term(a1, y)));
  pos.push(term(p1, y));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(a(a(y))) | p(a(y)) | p(y)
  pos.push(term(p1, x));
  pos.push(term(p1, term(a1, x)));
  pos.push(term(p1, term(a1, term(a1, x))));
  c = mkclause(0);
  pos.push(term(p1, term(a1, term(a1, y))));
  pos.push(term(p1, term(a1, y)));
  pos.push(term(p1, y));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // (a = x) <= (a = b)
  pos.push(term(basic(b_eq), a, x));
  c = mkclause(0);
  pos.push(term(basic(b_eq), a, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // (x = a) <= (a = b)
  pos.push(term(basic(b_eq), x, a));
  c = mkclause(0);
  pos.push(term(basic(b_eq), a, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(y) | !p(x) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push(term(p1, y));
  neg.push(term(p1, x));
  pos.push(term(q1, x));
  c = mkclause(0);
  neg.push(term(p1, a));
  neg.push(term(p1, b));
  pos.push(term(q1, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(x) | !p(y) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push(term(p1, x));
  neg.push(term(p1, y));
  pos.push(term(q1, x));
  c = mkclause(0);
  neg.push(term(p1, a));
  neg.push(term(p1, b));
  pos.push(term(q1, b));
  d = mkclause(0);
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,a(x)) !<= p(a(y),a(y))
  pos.push(term(p2, x, term(a1, x)));
  c = mkclause(0);
  pos.push(term(p2, term(a1, y), term(a1, y)));
  d = mkclause(0);
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));
}

void test_sym() {
  assert(keyword(intern("cnf")) == k_cnf);
  assert(keyword(intern("cnf....", 3)) == k_cnf);
  assert(keyword(intern("fof")) == k_fof);
  assert(keyword(intern("tff")) == k_tff);
  assert(intern("") == intern("", 0));
  assert(intern("xyz") == intern("xyz", 3));
}

void test_term() {
  auto red = fn(t_bool, intern("red"));
  auto green = fn(t_bool, intern("green"));
  auto blue = fn(t_bool, intern("blue"));

  auto a = term(basic(b_not), red);
  assert(a == term(basic(b_not), red));

  a = term(basic(b_and), red, green);
  assert(a == term(basic(b_and), red, green));

  vec<w> v;
  v.push(basic(b_and));
  v.push(red);
  v.push(green);
  v.push(blue);
  a = term(v);
  assert(a == term(v));
}

void test_type() {
  auto bird = type(intern("bird"));
  assert(bird == type(intern("bird")));
  assert(bird < t_compound);

  auto plane = type(intern("plane"));
  assert(plane == type(intern("plane")));
  assert(plane < t_compound);

  assert(bird != plane);

  vec<uint16_t> v;
  v.push(t_bool);
  v.push(t_int);
  v.push(t_int);
  auto t_predicate_int_int = type(v);
  assert(t_predicate_int_int == type(v));
  assert(t_predicate_int_int & t_compound);
  auto t = tcompoundp(t_predicate_int_int);
  assert(t->n == 3);
  assert(t->v[0] == t_bool);
  assert(t->v[1] == t_int);
  assert(t->v[2] == t_int);

  v.n = 0;
  v.push(t_bool);
  v.push(t_rat);
  v.push(t_rat);
  auto t_predicate_rat_rat = type(v);
  assert(t_predicate_rat_rat == type(v));
  assert(t_predicate_rat_rat & t_compound);
  t = tcompoundp(t_predicate_rat_rat);
  assert(t->n == 3);
  assert(t->v[0] == t_bool);
  assert(t->v[1] == t_rat);
  assert(t->v[2] == t_rat);
}

void test_typeof() {
  assert(typeof(var(t_int, 13)) == t_int);

  Int i1;
  mpz_init_set_ui(i1.val, 1);
  assert(typeof(int1(i1)) == t_int);

  Rat r1;
  mpq_init(r1.val);
  mpq_set_ui(r1.val, 1, 3);
  assert(typeof(rat(r1)) == t_rat);
  assert(typeof(real(r1)) == t_real);

  auto red = fn(t_bool, intern("red"));
  assert(typeof(red) == t_bool);
}

void test_unify() {
  // https://en.wikipedia.org/wiki/Unification_(computer_science)#Examples_of_syntactic_unification_of_first-order_terms
  auto a = fn(t_individual, intern("a"));
  auto b = fn(t_individual, intern("b"));
  auto f1 = fn(type(t_individual, t_individual), intern("f1"));
  auto f2 = fn(type(t_individual, t_individual, t_individual), intern("f2"));
  auto g1 = fn(type(t_individual, t_individual), intern("g1"));
  auto x = var(t_individual, 0);
  auto y = var(t_individual, 1);
  auto z = var(t_individual, 2);

  // Succeeds. (tautology)
  assert(unify0(a, a));
  assert(unified.n == 0);

  // a and b do not match
  assert(!unify0(a, b));

  // Succeeds. (tautology)
  assert(unify0(x, x));
  assert(unified.n == 0);

  // x is unified with the constant a
  assert(unify0(a, x));
  assert(unified.n == 1);
  assert(replace(x) == a);

  // x and y are aliased
  assert(unify0(x, y));
  assert(unified.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(unify0(term(f2, a, x), term(f2, a, b)));
  assert(unified.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!unify0(term(f1, a), term(g1, a)));

  // x and y are aliased
  assert(unify0(term(f1, x), term(f1, y)));
  assert(unified.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!unify0(term(f1, x), term(g1, y)));

  // Fails. The f function symbols have different arity
  assert(!unify0(term(f1, x), term(f2, y, z)));

  // Unifies y with the term g1(x)
  assert(unify0(term(f1, term(g1, x)), term(f1, y)));
  assert(unified.n == 1);
  assert(replace(y) == term(g1, x));

  // Unifies x with constant a, and y with the term g1(a)
  assert(unify0(term(f2, term(g1, x), x), term(f2, y, a)));
  assert(unified.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == term(g1, a));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check).
  assert(!unify0(x, term(f1, x)));

  // Both x and y are unified with the constant a
  assert(unify0(x, y));
  assert(unify(y, a));
  assert(unified.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // As above (order of equations in set doesn't matter)
  assert(unify0(a, y));
  assert(unify(x, y));
  assert(unified.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails. a and b do not match, so x can't be unified with both
  assert(unify0(x, a));
  assert(!unify(b, x));
}

void test_var() {
  auto x = var(t_int, 0);
  assert(vari(x) == 0);

  auto y = var(t_int, 1);
  assert(vari(y) == 1);

  assert(x != y);
}

void test_vec() {
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
///
} // namespace

void test() {
  // SORT
  test_clause();
  test_fn();
  test_getfree();
  test_int();
  test_match();
  test_rat();
  test_subsume();
  test_sym();
  test_term();
  test_type();
  test_typeof();
  test_unify();
  test_var();
  test_vec();
  ///
}
#endif
