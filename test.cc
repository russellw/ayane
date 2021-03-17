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
  return tag(term::Sym, s);
}

type internType(type rt, type param1, type param2) {
  vec<type> v(3);
  v[0] = rt;
  v[1] = param1;
  v[2] = param2;
  return internType(v);
}

clause *makeClause() {
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

bool match(term a, term b) {
  pairv.n = 0;
  return matchMore(a, b);
}

term replace(term a) {
  if (tag(a) == term::Var)
    for (auto &i : pairv)
      if (i.first == a)
        return replace(i.second);
  if (!isCompound(a))
    return a;
  auto n = size(a);
  vec<term> v;
  v.resize(n);
  for (si i = 0; i != n; ++i)
    v[i] = replace(at(a, i));
  return intern(tag(a), v);
}
///

// SORT
void testClause() {
  auto x = var(type::Int, 0);
  auto y = var(type::Int, 1);

  // a simple clause, x!=y
  neg.n = pos.n = 0;
  neg.push_back(intern(term::Eq, x, y));
  auto c = internClause(infer::none);

  // duplicate returns null
  neg.n = pos.n = 0;
  neg.push_back(intern(term::Eq, x, y));
  auto d = internClause(infer::none);
  assert(!d);

  // the duplicate check distinguishes between negative and positive literals
  neg.n = pos.n = 0;
  pos.push_back(intern(term::Eq, x, y));
  d = internClause(infer::none);
  assert(c != d);
}

void testCnf() {
  // true
  initClauses();
  cnf(term::True, 0);
  assert(inputClauses.n == 0);

  // false
  initClauses();
  cnf(term::False, 0);
  assert(inputClauses.n == 1);
  neg.n = 0;
  pos.n = 0;
  assert(!internClause(infer::none));
}

void testFn() {
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

void testFreeVars() {
  auto a = fn(type::Individual, intern("a"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);

  getFreeVars(a);
  assert(termv.n == 0);

  getFreeVars(x);
  assert(termv.n == 1);
  assert(termv[0] == x);

  getFreeVars(intern(term::Eq, x, x));
  assert(termv.n == 1);
  assert(termv[0] == x);

  getFreeVars(intern(term::Eq, x, y));
  assert(termv.n == 2);
  assert(termv[0] == x);
  assert(termv[1] == y);

  getFreeVars(intern(term::Eq, a, a));
  assert(termv.n == 0);

  getFreeVars(intern(term::All, intern(term::Eq, x, y), x));
  assert(termv.n == 1);
  assert(termv[0] == y);
}

void testInt() {
  Int x1;
  mpz_init_set_ui(x1.val, 1);
  auto a1 = tag(term::Int, intern(x1));
  auto y1 = (Int *)rest(a1);
  assert(!mpz_cmp_ui(y1->val, 1));

  Int x2;
  mpz_init_set_ui(x2.val, 2);
  auto a2 = tag(term::Int, intern(x2));
  auto y2 = (Int *)rest(a2);
  assert(!mpz_cmp_ui(y2->val, 2));

  Int x3;
  mpz_init(x3.val);
  mpz_add(x3.val, y1->val, y2->val);
  auto a3 = tag(term::Int, intern(x3));
  auto y3 = (Int *)rest(a3);
  assert(!mpz_cmp_ui(y3->val, 3));
}

void testIntern() {
  auto red = fn(type::Bool, intern("red"));
  auto green = fn(type::Bool, intern("green"));
  auto blue = fn(type::Bool, intern("blue"));

  auto a = intern(term::Not, red);
  assert(a == intern(term::Not, red));

  a = intern(term::And, red, green);
  assert(a == intern(term::And, red, green));

  vec<term> v;
  v.push_back(red);
  v.push_back(green);
  v.push_back(blue);
  a = intern(term::And, v);
  assert(a == intern(term::And, v));
}

void testMatch() {
  // subset of unify where only the first argument can be treated as a variable
  // to be matched against the second argument. applied to the same test cases
  // as unify, gives the same results in some cases, but different results in
  // others. in particular, has no notion of an occurs check; in actual use, it
  // is assumed that the arguments will have disjoint variables
  auto a = fn(type::Individual, intern("a"));
  auto b = fn(type::Individual, intern("b"));
  auto f1 = fn(internType(type::Individual, type::Individual), intern("f1"));
  auto f2 = fn(internType(type::Individual, type::Individual, type::Individual),
               intern("f2"));
  auto g1 = fn(internType(type::Individual, type::Individual), intern("g1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  auto z = var(type::Individual, 2);

  // Succeeds. (tautology)
  assert(match(a, a));
  assert(pairv.n == 0);

  // a and b do not match
  assert(!match(a, b));

  // Succeeds. (tautology)
  assert(match(x, x));
  assert(pairv.n == 0);

  // x is not matched with the constant a, because the variable is on the
  // right-hand side
  assert(!match(a, x));

  // x and y are aliased
  assert(match(x, y));
  assert(pairv.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(match(intern(term::Call, f2, a, x), intern(term::Call, f2, a, b)));
  assert(pairv.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!match(intern(term::Call, f1, a), intern(term::Call, g1, a)));

  // x and y are aliased
  assert(match(intern(term::Call, f1, x), intern(term::Call, f1, y)));
  assert(pairv.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!match(intern(term::Call, f1, x), intern(term::Call, g1, y)));

  // Fails. The f function symbols have different arity
  assert(!match(intern(term::Call, f1, x), intern(term::Call, f2, y, z)));

  // Does not match y with the term g1(x), because the variable is on the
  // right-hand side
  assert(!match(intern(term::Call, f1, intern(term::Call, g1, x)),
                intern(term::Call, f1, y)));

  // Does not match, because the variable is on the right-hand side
  assert(!match(intern(term::Call, f2, intern(term::Call, g1, x), x),
                intern(term::Call, f2, y, a)));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check) but returns true here because match has no
  // notion of an occurs check
  assert(match(x, intern(term::Call, f1, x)));
  assert(pairv.n == 1);

  // Both x and y are unified with the constant a
  assert(match(x, y));
  assert(matchMore(y, a));
  assert(pairv.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails this time, because the variable is on the right-hand side
  assert(!match(a, y));

  // Fails. a and b do not match, so x can't be unified with both
  assert(match(x, a));
  assert(!matchMore(b, x));
}

void testRat() {
  Rat x1;
  mpq_init(x1.val);
  mpq_set_ui(x1.val, 1, 1);
  auto a1 = tag(term::Rat, intern(x1));
  auto y1 = (Rat *)rest(a1);
  assert(!mpq_cmp_ui(y1->val, 1, 1));

  Rat x2;
  mpq_init(x2.val);
  mpq_set_ui(x2.val, 2, 1);
  auto a2 = tag(term::Rat, intern(x2));
  auto y2 = (Rat *)rest(a2);
  assert(!mpq_cmp_ui(y2->val, 2, 1));

  Rat x3;
  mpq_init(x3.val);
  mpq_add(x3.val, y1->val, y2->val);
  auto a3 = tag(term::Rat, intern(x3));
  auto y3 = (Rat *)rest(a3);
  assert(!mpq_cmp_ui(y3->val, 3, 1));
}

void testSubsume() {
  auto a = fn(type::Individual, intern("a"));
  auto a1 = fn(internType(type::Individual, type::Individual), intern("a1"));
  auto b = fn(type::Individual, intern("b"));
  auto p = fn(type::Bool, intern("p"));
  auto p1 = fn(internType(type::Bool, type::Individual), intern("p1"));
  auto p2 = fn(internType(type::Bool, type::Individual, type::Individual),
               intern("p2"));
  auto q = fn(type::Bool, intern("q"));
  auto q1 = fn(internType(type::Bool, type::Individual), intern("q1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  clause *c;
  clause *d;

  // false <= false
  c = makeClause();
  d = c;
  assert(subsumes(c, d));

  // false <= p
  c = makeClause();
  pos.push_back(p);
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p <= p
  pos.push_back(p);
  c = makeClause();
  d = c;
  assert(subsumes(c, d));

  // !p <= !p
  neg.push_back(p);
  c = makeClause();
  d = c;
  assert(subsumes(c, d));

  // p <= p | p
  pos.push_back(p);
  c = makeClause();
  pos.push_back(p);
  pos.push_back(p);
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p !<= !p
  pos.push_back(p);
  c = makeClause();
  neg.push_back(p);
  d = makeClause();
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p | q <= q | p
  pos.push_back(p);
  pos.push_back(q);
  c = makeClause();
  pos.push_back(q);
  pos.push_back(p);
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p | q <= p | q | p
  pos.push_back(p);
  pos.push_back(q);
  c = makeClause();
  pos.push_back(p);
  pos.push_back(q);
  pos.push_back(p);
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(a) | p(b) | q(a) | q(b) | <= p(a) | q(a) | p(b) | q(b)
  pos.push_back(intern(term::Call, p1, a));
  pos.push_back(intern(term::Call, p1, b));
  pos.push_back(intern(term::Call, q1, a));
  pos.push_back(intern(term::Call, q1, b));
  c = makeClause();
  pos.push_back(intern(term::Call, p1, a));
  pos.push_back(intern(term::Call, q1, a));
  pos.push_back(intern(term::Call, p1, b));
  pos.push_back(intern(term::Call, q1, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x,y) <= p(a,b)
  pos.push_back(intern(term::Call, p2, x, y));
  c = makeClause();
  pos.push_back(intern(term::Call, p2, a, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,x) !<= p(a,b)
  pos.push_back(intern(term::Call, p2, x, x));
  c = makeClause();
  pos.push_back(intern(term::Call, p2, a, b));
  d = makeClause();
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) <= p(y)
  pos.push_back(intern(term::Call, p1, x));
  c = makeClause();
  pos.push_back(intern(term::Call, p1, y));
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(y) | p(a(y)) | p(a(a(y)))
  pos.push_back(intern(term::Call, p1, x));
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, x)));
  pos.push_back(intern(term::Call, p1,
                       intern(term::Call, a1, intern(term::Call, a1, x))));
  c = makeClause();
  pos.push_back(intern(term::Call, p1, y));
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, y)));
  pos.push_back(intern(term::Call, p1,
                       intern(term::Call, a1, intern(term::Call, a1, y))));
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a) <= p(a) | p(b)
  pos.push_back(intern(term::Call, p1, x));
  pos.push_back(intern(term::Call, p1, a));
  c = makeClause();
  pos.push_back(intern(term::Call, p1, a));
  pos.push_back(intern(term::Call, p1, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x) | p(a(x)) <= p(a(y)) | p(y)
  pos.push_back(intern(term::Call, p1, x));
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, x)));
  c = makeClause();
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, y)));
  pos.push_back(intern(term::Call, p1, y));
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // p(x) | p(a(x)) | p(a(a(x))) <= p(a(a(y))) | p(a(y)) | p(y)
  pos.push_back(intern(term::Call, p1, x));
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, x)));
  pos.push_back(intern(term::Call, p1,
                       intern(term::Call, a1, intern(term::Call, a1, x))));
  c = makeClause();
  pos.push_back(intern(term::Call, p1,
                       intern(term::Call, a1, intern(term::Call, a1, y))));
  pos.push_back(intern(term::Call, p1, intern(term::Call, a1, y)));
  pos.push_back(intern(term::Call, p1, y));
  d = makeClause();
  assert(subsumes(c, d));
  assert(subsumes(d, c));

  // (a = x) <= (a = b)
  pos.push_back(intern(term::Eq, a, x));
  c = makeClause();
  pos.push_back(intern(term::Eq, a, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // (x = a) <= (a = b)
  pos.push_back(intern(term::Eq, x, a));
  c = makeClause();
  pos.push_back(intern(term::Eq, a, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(y) | !p(x) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push_back(intern(term::Call, p1, y));
  neg.push_back(intern(term::Call, p1, x));
  pos.push_back(intern(term::Call, q1, x));
  c = makeClause();
  neg.push_back(intern(term::Call, p1, a));
  neg.push_back(intern(term::Call, p1, b));
  pos.push_back(intern(term::Call, q1, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // !p(x) | !p(y) | q(x) <= !p(a) | !p(b) | q(b)
  neg.push_back(intern(term::Call, p1, x));
  neg.push_back(intern(term::Call, p1, y));
  pos.push_back(intern(term::Call, q1, x));
  c = makeClause();
  neg.push_back(intern(term::Call, p1, a));
  neg.push_back(intern(term::Call, p1, b));
  pos.push_back(intern(term::Call, q1, b));
  d = makeClause();
  assert(subsumes(c, d));
  assert(!subsumes(d, c));

  // p(x,a(x)) !<= p(a(y),a(y))
  pos.push_back(intern(term::Call, p2, x, intern(term::Call, a1, x)));
  c = makeClause();
  pos.push_back(intern(term::Call, p2, intern(term::Call, a1, y),
                       intern(term::Call, a1, y)));
  d = makeClause();
  assert(!subsumes(c, d));
  assert(!subsumes(d, c));
}

void testSym() {
  assert(keyword(intern("cnf")) == k_cnf);
  assert(keyword(intern("cnf....", 3)) == k_cnf);
  assert(keyword(intern("fof")) == k_fof);
  assert(keyword(intern("tff")) == k_tff);
  assert(intern("") == intern("", 0));
  assert(intern("xyz") == intern("xyz", 3));
}

void testType() {
  auto bird = internType(intern("bird"));
  assert(bird == internType(intern("bird")));
  assert(!isCompound(bird));

  auto plane = internType(intern("plane"));
  assert(plane == internType(intern("plane")));
  assert(!isCompound(plane));

  assert(bird != plane);

  vec<type> v;
  v.push_back(type::Bool);
  v.push_back(type::Int);
  v.push_back(type::Int);
  auto t_predicate_int_int = internType(v);
  assert(t_predicate_int_int == internType(v));
  assert(isCompound(t_predicate_int_int));
  auto t = tcompoundp(t_predicate_int_int);
  assert(t->n == 3);
  assert(t->v[0] == type::Bool);
  assert(t->v[1] == type::Int);
  assert(t->v[2] == type::Int);

  v.n = 0;
  v.push_back(type::Bool);
  v.push_back(type::Rat);
  v.push_back(type::Rat);
  auto t_predicate_rat_rat = internType(v);
  assert(t_predicate_rat_rat == internType(v));
  assert(isCompound(t_predicate_rat_rat));
  t = tcompoundp(t_predicate_rat_rat);
  assert(t->n == 3);
  assert(t->v[0] == type::Bool);
  assert(t->v[1] == type::Rat);
  assert(t->v[2] == type::Rat);
}

void testTypeof() {
  assert(typeof(var(type::Int, 13)) == type::Int);

  Int i1;
  mpz_init_set_ui(i1.val, 1);
  assert(typeof(tag(term::Int, intern(i1))) == type::Int);

  Rat r1;
  mpq_init(r1.val);
  mpq_set_ui(r1.val, 1, 3);
  assert(typeof(tag(term::Rat, intern(r1))) == type::Rat);
  assert(typeof(tag(term::Real, intern(r1))) == type::Real);

  auto red = fn(type::Bool, intern("red"));
  assert(typeof(red) == type::Bool);
}

void testUnify() {
  // https://en.wikipedia.org/wiki/Unification_(computer_science)#Examples_of_syntactic_unification_of_first-order_terms
  auto a = fn(type::Individual, intern("a"));
  auto b = fn(type::Individual, intern("b"));
  auto f1 = fn(internType(type::Individual, type::Individual), intern("f1"));
  auto f2 = fn(internType(type::Individual, type::Individual, type::Individual),
               intern("f2"));
  auto g1 = fn(internType(type::Individual, type::Individual), intern("g1"));
  auto x = var(type::Individual, 0);
  auto y = var(type::Individual, 1);
  auto z = var(type::Individual, 2);

  // Succeeds. (tautology)
  assert(unify(a, a));
  assert(pairv.n == 0);

  // a and b do not match
  assert(!unify(a, b));

  // Succeeds. (tautology)
  assert(unify(x, x));
  assert(pairv.n == 0);

  // x is unified with the constant a
  assert(unify(a, x));
  assert(pairv.n == 1);
  assert(replace(x) == a);

  // x and y are aliased
  assert(unify(x, y));
  assert(pairv.n == 1);
  assert(replace(x) == replace(y));

  // Function and constant symbols match, x is unified with the constant b
  assert(unify(intern(term::Call, f2, a, x), intern(term::Call, f2, a, b)));
  assert(pairv.n == 1);
  assert(replace(x) == b);

  // f and g do not match
  assert(!unify(intern(term::Call, f1, a), intern(term::Call, g1, a)));

  // x and y are aliased
  assert(unify(intern(term::Call, f1, x), intern(term::Call, f1, y)));
  assert(pairv.n == 1);
  assert(replace(x) == replace(y));

  // f and g do not match
  assert(!unify(intern(term::Call, f1, x), intern(term::Call, g1, y)));

  // Fails. The f function symbols have different arity
  assert(!unify(intern(term::Call, f1, x), intern(term::Call, f2, y, z)));

  // Unifies y with the term g1(x)
  assert(unify(intern(term::Call, f1, intern(term::Call, g1, x)),
               intern(term::Call, f1, y)));
  assert(pairv.n == 1);
  assert(replace(y) == intern(term::Call, g1, x));

  // Unifies x with constant a, and y with the term g1(a)
  assert(unify(intern(term::Call, f2, intern(term::Call, g1, x), x),
               intern(term::Call, f2, y, a)));
  assert(pairv.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == intern(term::Call, g1, a));

  // Returns false in first-order logic and many modern Prolog dialects
  // (enforced by the occurs check).
  assert(!unify(x, intern(term::Call, f1, x)));

  // Both x and y are unified with the constant a
  assert(unify(x, y));
  assert(unifyMore(y, a));
  assert(pairv.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // As above (order of equations in set doesn't matter)
  assert(unify(a, y));
  assert(unifyMore(x, y));
  assert(pairv.n == 2);
  assert(replace(x) == a);
  assert(replace(y) == a);

  // Fails. a and b do not match, so x can't be unified with both
  assert(unify(x, a));
  assert(!unifyMore(b, x));
}

void testVar() {
  auto x = var(type::Int, 0);
  assert(vari(x) == 0);

  auto y = var(type::Int, 1);
  assert(vari(y) == 1);

  assert(x != y);
}

void testVec() {
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
  testClause();
  testCnf();
  testFn();
  testFreeVars();
  testInt();
  testIntern();
  testMatch();
  testRat();
  testSubsume();
  testSym();
  testType();
  testTypeof();
  testUnify();
  testVar();
  testVec();
  ///
}
#endif
