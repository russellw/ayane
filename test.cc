#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

#ifdef DEBUG
namespace {
// SORT
term fn(type t, sym *s) {
  if (s->ft == type::none)
    s->ft = t;
  assert(s->ft == t);
  return mk(s, term::Sym);
}

bool match0(term a, term b) {
  varmap.n = 0;
  return match(a, b);
}

clause *mkclause1() {
  auto nn = neg.n;
  auto pn = pos.n;
  auto n = nn + pn;
  neg.n = pos.n = 0;
  memcpy(neg.p + nn, pos.p, pn * sizeof *pos.p);

  auto c = (clause *)xmalloc(offsetof(clause, v) + n * sizeof *neg.p);
  memset(c, 0, offsetof(clause, v));
  c->nn = nn;
  c->n = n;
  memcpy(c->v, neg.p, n * sizeof *neg.p);
  return c;
}

type mktype(type rt, type param1, type param2) {
  vec<type> v(3);
  v[0] = rt;
  v[1] = param1;
  v[2] = param2;
  return mktype(v);
}

term replace(term a) {
  if (tag(a) == term::Var)
    for (auto &p : varmap)
      if (p.first == a)
        return replace(p.second);
  if (!iscompound(a))
    return a;
  auto n = size(a);
  vec<term> v;
  v.resize(n);
  for (si i = 0; i != n; ++i)
    v[i] = replace(at(a, i));
  return mk(tag(a), v);
}
///

// SORT
void test_clause() {
  auto x = var(type::Int, 0);
  auto y = var(type::Int, 1);

  // a simple clause, x!=y
  neg.n = pos.n = 0;
  neg.push(mk(term::Eq, x, y));
  auto c = mkclause(infer::none);

  // duplicate returns null
  neg.n = pos.n = 0;
  neg.push(mk(term::Eq, x, y));
  auto d = mkclause(infer::none);
  assert(!d);

  // the duplicate check distinguishes between negative and positive literals
  neg.n = pos.n = 0;
  pos.push(mk(term::Eq, x, y));
  d = mkclause(infer::none);
  assert(c != d);
}

void test_fn() {
  auto red = fn(type::Bool, intern("red"));
  auto redp = (sym *)rest(red);
  assert(redp->ft == type::Bool);

  auto green = fn(type::Bool, intern("green"));
  auto greenp = (sym *)rest(green);
  assert(greenp->ft == type::Bool);

  auto blue = fn(type::Bool, intern("blue"));
  auto bluep = (sym *)rest(blue);
  assert(bluep->ft == type::Bool);

  assert(redp == intern("red"));
  assert(greenp == intern("green"));
  assert(bluep == intern("blue"));
}

void test_getfree() {
  auto a = fn(type::Individual, intern("a"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);

  getfree(a);
  assert(terms.n == 0);

  getfree(x);
  assert(terms.n == 1);
  assert(terms[0] == x);

  getfree(mk(term::Eq, x, x));
  assert(terms.n == 1);
  assert(terms[0] == x);

  getfree(mk(term::Eq, x, y));
  assert(terms.n == 2);
  assert(terms[0] == x);
  assert(terms[1] == y);

  getfree(mk(term::Eq, a, a));
  assert(terms.n == 0);

  getfree(mk(term::All, mk(term::Eq, x, y), x));
  assert(terms.n == 1);
  assert(terms[0] == y);
}

void test_int() {
  Int x1;
  mpz_init_set_ui(x1.val, 1);
  auto a1 = mk(intern(x1), term::Int);
  auto y1 = (Int *)rest(a1);
  assert(!mpz_cmp_ui(y1->val, 1));

  Int x2;
  mpz_init_set_ui(x2.val, 2);
  auto a2 = mk(intern(x2), term::Int);
  auto y2 = (Int *)rest(a2);
  assert(!mpz_cmp_ui(y2->val, 2));

  Int x3;
  mpz_init(x3.val);
  mpz_add(x3.val, y1->val, y2->val);
  auto a3 = mk(intern(x3), term::Int);
  auto y3 = (Int *)rest(a3);
  assert(!mpz_cmp_ui(y3->val, 3));
}

void test_match() {
  // subset of unify where only the first argument can be treated as a variable
  // to be matched against the second argument. applied to the same test cases
  // as unify, gives the same results in some cases, but different results in
  // others. in particular, has no notion of an occurs check; in actual use, it
  // is assumed that the arguments will have disjoint variables
  auto a = fn(type::Individual, intern("a"));
  auto b = fn(type::Individual, intern("b"));
  auto f1 = fn(mktype(type::Individual, type::Individual), intern("f1"));
  auto f2 = fn(mktype(type::Individual, type::Individual, type::Individual),
               intern("f2"));
  auto g1 = fn(mktype(type::Individual, type::Individual), intern("g1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  auto z = var(type::Individual, 2);

  // Succeeds. (tautology)
  assert(match0(a, a));
  assert(varmap.n == 0);

  // a and b do not match
  assert(!match0(a, b));

  // Succeeds. (tautology)
  assert(match0(x, x));
  assert(varmap.n == 0);

  // x is not matched with the constant a, because the variable is on the
  // right-hand side
  assert(!match0(a, x));

  // x and y are aliased
  assert(match0(x, y));
  assert(varmap.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(match0(mk(term::Call, f2, a, x), mk(term::Call, f2, a, b)));
  assert(varmap.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!match0(mk(term::Call, f1, a), mk(term::Call, g1, a)));

  // x and y are aliased
  assert(match0(mk(term::Call, f1, x), mk(term::Call, f1, y)));
  assert(varmap.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!match0(mk(term::Call, f1, x), mk(term::Call, g1, y)));

  // Fails. The f function symbols have different arity
  assert(!match0(mk(term::Call, f1, x), mk(term::Call, f2, y, z)));

  // Does not match y with the term g1(x), because the variable is on the
  // right-hand side
  assert(!match0(mk(term::Call, f1, mk(term::Call, g1, x)),
                 mk(term::Call, f1, y)));

  // Does not match, because the variable is on the right-hand side
  assert(!match0(mk(term::Call, f2, mk(term::Call, g1, x), x),
                 mk(term::Call, f2, y, a)));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check) but returns true here because match has no
  // notion of an occurs check
  assert(match0(x, mk(term::Call, f1, x)));
  assert(varmap.n == 1);

  // Both x and y are unified with the constant a
  assert(match0(x, y));
  assert(match(y, a));
  assert(varmap.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails this time, because the variable is on the right-hand side
  assert(!match0(a, y));

  // Fails. a and b do not match, so x can't be unified with both
  assert(match0(x, a));
  assert(!match(b, x));
}

void test_mk() {
  auto red = fn(type::Bool, intern("red"));
  auto green = fn(type::Bool, intern("green"));
  auto blue = fn(type::Bool, intern("blue"));

  auto a = mk(term::Not, red);
  assert(a == mk(term::Not, red));

  a = mk(term::And, red, green);
  assert(a == mk(term::And, red, green));

  vec<term> v;
  v.push_back(red);
  v.push_back(green);
  v.push_back(blue);
  a = mk(term::And, v);
  assert(a == mk(term::And, v));
}

void test_mktype() {
  auto bird = mktype(intern("bird"));
  assert(bird == mktype(intern("bird")));
  assert(!iscompound(bird));

  auto plane = mktype(intern("plane"));
  assert(plane == mktype(intern("plane")));
  assert(!iscompound(plane));

  assert(bird != plane);

  vec<type> v;
  v.push_back(type::Bool);
  v.push_back(type::Int);
  v.push_back(type::Int);
  auto t_predicate_int_int = mktype(v);
  assert(t_predicate_int_int == mktype(v));
  assert(iscompound(t_predicate_int_int));
  auto t = tcompoundp(t_predicate_int_int);
  assert(t->n == 3);
  assert(t->v[0] == type::Bool);
  assert(t->v[1] == type::Int);
  assert(t->v[2] == type::Int);

  v.n = 0;
  v.push_back(type::Bool);
  v.push_back(type::Rat);
  v.push_back(type::Rat);
  auto t_predicate_rat_rat = mktype(v);
  assert(t_predicate_rat_rat == mktype(v));
  assert(iscompound(t_predicate_rat_rat));
  t = tcompoundp(t_predicate_rat_rat);
  assert(t->n == 3);
  assert(t->v[0] == type::Bool);
  assert(t->v[1] == type::Rat);
  assert(t->v[2] == type::Rat);
}

void test_rat() {
  Rat x1;
  mpq_init(x1.val);
  mpq_set_ui(x1.val, 1, 1);
  auto a1 = mk(intern(x1), term::Rat);
  auto y1 = (Rat *)rest(a1);
  assert(!mpq_cmp_ui(y1->val, 1, 1));

  Rat x2;
  mpq_init(x2.val);
  mpq_set_ui(x2.val, 2, 1);
  auto a2 = mk(intern(x2), term::Rat);
  auto y2 = (Rat *)rest(a2);
  assert(!mpq_cmp_ui(y2->val, 2, 1));

  Rat x3;
  mpq_init(x3.val);
  mpq_add(x3.val, y1->val, y2->val);
  auto a3 = mk(intern(x3), term::Rat);
  auto y3 = (Rat *)rest(a3);
  assert(!mpq_cmp_ui(y3->val, 3, 1));
}

void test_subsume() {
  auto a = fn(type::Individual, intern("a"));
  auto a1 = fn(mktype(type::Individual, type::Individual), intern("a1"));
  auto b = fn(type::Individual, intern("b"));
  auto p = fn(type::Bool, intern("p"));
  auto p1 = fn(mktype(type::Bool, type::Individual), intern("p1"));
  auto p2 =
      fn(mktype(type::Bool, type::Individual, type::Individual), intern("p2"));
  auto q = fn(type::Bool, intern("q"));
  auto q1 = fn(mktype(type::Bool, type::Individual), intern("q1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  clause *c;
  clause *d;

  // false <= false
  c = mkclause1();
  d = c;
  assert(subsumes(c, d));

  // false <= p
  c = mkclause1();
  pos.push(p);
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p <= p
  pos.push(p);
  c = mkclause1();
  d = c;
  assert(subsumes(c, d));

  // !p <= !p
  neg.push(p);
  c = mkclause1();
  d = c;
  assert(subsumes(c, d));

  // p <= p | p
  pos.push(p);
  c = mkclause1();
  pos.push(p);
  pos.push(p);
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p !<= !p
  pos.push(p);
  c = mkclause1();
  neg.push(p);
  d = mkclause1();
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p | q <= q | p
  pos.push(p);
  pos.push(q);
  c = mkclause1();
  pos.push(q);
  pos.push(p);
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p | q <= p | q | p
  pos.push(p);
  pos.push(q);
  c = mkclause1();
  pos.push(p);
  pos.push(q);
  pos.push(p);
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(a) | p(b) | q(a) | q(b) | <= p(a) | q(a) | p(b) | q(b)
  pos.push(mk(term::Call, p1, a));
  pos.push(mk(term::Call, p1, b));
  pos.push(mk(term::Call, q1, a));
  pos.push(mk(term::Call, q1, b));
  c = mkclause1();
  pos.push(mk(term::Call, p1, a));
  pos.push(mk(term::Call, q1, a));
  pos.push(mk(term::Call, p1, b));
  pos.push(mk(term::Call, q1, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x,y) <= p(a,b)
  pos.push(mk(term::Call, p2, x, y));
  c = mkclause1();
  pos.push(mk(term::Call, p2, a, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,x) !<= p(a,b)
  pos.push(mk(term::Call, p2, x, x));
  c = mkclause1();
  pos.push(mk(term::Call, p2, a, b));
  d = mkclause1();
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) <= p(y)
  pos.push(mk(term::Call, p1, x));
  c = mkclause1();
  pos.push(mk(term::Call, p1, y));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(y) | p(a(y)) | p(a(a(y)))
  pos.push(mk(term::Call, p1, x));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, x)));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, mk(term::Call, a1, x))));
  c = mkclause1();
  pos.push(mk(term::Call, p1, y));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, y)));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, mk(term::Call, a1, y))));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a) <= p(a) | p(b)
  pos.push(mk(term::Call, p1, x));
  pos.push(mk(term::Call, p1, a));
  c = mkclause1();
  pos.push(mk(term::Call, p1, a));
  pos.push(mk(term::Call, p1, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) | p(a(x)) <= p(a(y)) | p(y)
  pos.push(mk(term::Call, p1, x));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, x)));
  c = mkclause1();
  pos.push(mk(term::Call, p1, mk(term::Call, a1, y)));
  pos.push(mk(term::Call, p1, y));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(a(a(y))) | p(a(y)) | p(y)
  pos.push(mk(term::Call, p1, x));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, x)));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, mk(term::Call, a1, x))));
  c = mkclause1();
  pos.push(mk(term::Call, p1, mk(term::Call, a1, mk(term::Call, a1, y))));
  pos.push(mk(term::Call, p1, mk(term::Call, a1, y)));
  pos.push(mk(term::Call, p1, y));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // (a = x) <= (a = b)
  pos.push(mk(term::Eq, a, x));
  c = mkclause1();
  pos.push(mk(term::Eq, a, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // (x = a) <= (a = b)
  pos.push(mk(term::Eq, x, a));
  c = mkclause1();
  pos.push(mk(term::Eq, a, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(y) | !p(x) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push(mk(term::Call, p1, y));
  neg.push(mk(term::Call, p1, x));
  pos.push(mk(term::Call, q1, x));
  c = mkclause1();
  neg.push(mk(term::Call, p1, a));
  neg.push(mk(term::Call, p1, b));
  pos.push(mk(term::Call, q1, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(x) | !p(y) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push(mk(term::Call, p1, x));
  neg.push(mk(term::Call, p1, y));
  pos.push(mk(term::Call, q1, x));
  c = mkclause1();
  neg.push(mk(term::Call, p1, a));
  neg.push(mk(term::Call, p1, b));
  pos.push(mk(term::Call, q1, b));
  d = mkclause1();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,a(x)) !<= p(a(y),a(y))
  pos.push(mk(term::Call, p2, x, mk(term::Call, a1, x)));
  c = mkclause1();
  pos.push(mk(term::Call, p2, mk(term::Call, a1, y), mk(term::Call, a1, y)));
  d = mkclause1();
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

void test_typeof() {
  assert(typeof(var(type::Int, 13)) == type::Int);

  Int i1;
  mpz_init_set_ui(i1.val, 1);
  assert(typeof(mk(intern(i1), term::Int)) == type::Int);

  Rat r1;
  mpq_init(r1.val);
  mpq_set_ui(r1.val, 1, 3);
  assert(typeof(mk(intern(r1), term::Rat)) == type::Rat);
  assert(typeof(mk(intern(r1), term::Real)) == type::Real);

  auto red = fn(type::Bool, intern("red"));
  assert(typeof(red) == type::Bool);
}

void test_unify() {
  // https://en.wikipedia.org/wiki/Unification_(computer_science)#Examples_of_syntactic_unification_of_first-order_terms
  auto a = fn(type::Individual, intern("a"));
  auto b = fn(type::Individual, intern("b"));
  auto f1 = fn(mktype(type::Individual, type::Individual), intern("f1"));
  auto f2 = fn(mktype(type::Individual, type::Individual, type::Individual),
               intern("f2"));
  auto g1 = fn(mktype(type::Individual, type::Individual), intern("g1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  auto z = var(type::Individual, 2);

  // Succeeds. (tautology)
  assert(unify(a, a));
  assert(varmap.n == 0);

  // a and b do not match
  assert(!unify(a, b));

  // Succeeds. (tautology)
  assert(unify(x, x));
  assert(varmap.n == 0);

  // x is unified with the constant a
  assert(unify(a, x));
  assert(varmap.n == 1);
  assert(replace(x) == a);

  // x and y are aliased
  assert(unify(x, y));
  assert(varmap.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(unify(mk(term::Call, f2, a, x), mk(term::Call, f2, a, b)));
  assert(varmap.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!unify(mk(term::Call, f1, a), mk(term::Call, g1, a)));

  // x and y are aliased
  assert(unify(mk(term::Call, f1, x), mk(term::Call, f1, y)));
  assert(varmap.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!unify(mk(term::Call, f1, x), mk(term::Call, g1, y)));

  // Fails. The f function symbols have different arity
  assert(!unify(mk(term::Call, f1, x), mk(term::Call, f2, y, z)));

  // Unifies y with the term g1(x)
  assert(
      unify(mk(term::Call, f1, mk(term::Call, g1, x)), mk(term::Call, f1, y)));
  assert(varmap.n == 1);
  assert(replace(y) == mk(term::Call, g1, x));

  // Unifies x with constant a, and y with the term g1(a)
  assert(unify(mk(term::Call, f2, mk(term::Call, g1, x), x),
               mk(term::Call, f2, y, a)));
  assert(varmap.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == mk(term::Call, g1, a));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check).
  assert(!unify(x, mk(term::Call, f1, x)));

  // Both x and y are unified with the constant a
  assert(unify(x, y));
  assert(unify1(y, a));
  assert(varmap.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // As above (order of equations in set doesn't matter)
  assert(unify(a, y));
  assert(unify1(x, y));
  assert(varmap.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails. a and b do not match, so x can't be unified with both
  assert(unify(x, a));
  assert(!unify1(b, x));
}

void test_var() {
  auto x = var(type::Int, 0);
  assert(vari(x) == 0);

  auto y = var(type::Int, 1);
  assert(vari(y) == 1);

  assert(x != y);
}

void test_vec() {
  vec<char, 1> v;
  assert(v.n == 0);

  v.push_back('a');
  assert(v.n == 1);
  assert(v.back() == 'a');

  v.push_back('b');
  assert(v.n == 2);
  assert(v.back() == 'b');

  v.push_back('c');
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
  test_mk();
  test_mktype();
  test_rat();
  test_subsume();
  test_sym();
  test_typeof();
  test_unify();
  test_var();
  test_vec();
  ///
}
#endif
