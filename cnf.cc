#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
// estimate how many clauses a term will expand into, for the purpose of
// deciding when subformulas need to be renamed; the answer could exceed 2^31,
// but then we don't actually need the number, we only need to know whether it
// went over the threshold
const si many = 10;
si nclauses(bool pol, term a);

si nclauses_and(bool pol, term a) {
  si r = 0;
  for (auto b : a) {
    r += nclauses(pol, b);
    if (r >= many)
      return many;
  }
  return r;
}

si nclauses_or(bool pol, term a) {
  si r = 1;
  for (auto b : a) {
    r *= nclauses(pol, b);
    if (r >= many)
      return many;
  }
  return r;
}

si nclauses(bool pol, term a) {
  switch (tag(a)) {
  case term::All:
  case term::Exists:
  case term::Not:
    return nclauses(pol, at(a, 0));
  case term::And:
    return pol ? nclauses_and(pol, a) : nclauses_or(pol, a);
  case term::Eqv: {
    auto x = at(a, 0);
    auto x0 = nclauses(0, x);
    if (x0 >= many - 1)
      return many;
    auto x1 = nclauses(1, x);
    if (x1 >= many - 1)
      return many;
    auto y = at(a, 1);
    auto y0 = nclauses(0, y);
    if (y0 >= many - 1)
      return many;
    auto y1 = nclauses(1, y);
    auto r = pol ? x0 * y1 + x1 * y0 : x0 * y0 + x1 * y1;
    if (r >= many)
      return many;
    return r;
  }
  case term::Or:
    return pol ? nclauses_or(pol, a) : nclauses_and(pol, a);
  }
  return 1;
}

// creating new functions is necessary both to skolemize existential variables
// and to rename subformulas to avoid exponential blowup
term skolem(type t) {
  auto n = sprintf(buf, "\1%zu", skolemi++);
  auto s = intern(buf, n);
  s->ft = t;
  return tag(s, term::Sym);
}

term skolemterms(type rt) {
  // atom
  if (!termv.n)
    return skolem(rt);

  // compound type
  vec<type> t(termv.n + 1);
  t[0] = rt;
  for (si i = 0; i != termv.n; ++i)
    t[i + 1] = vartype(termv[i]);

  // compound
  termv.insert(termv.p, skolem(mktype(t)));
  return intern(term::Call, termv);
}

// rename subformulas to avoid exponential blowup
term rename_pos(term a) {
  getfree(a);
  auto b = skolemterms(type::Bool);
  cnf(imp(b, a), 0);
  return b;
}

term cnf1(term a);

term rename_both(term a) {
  a = cnf1(a);
  getfree(a);
  auto b = skolemterms(type::Bool);
  cnf(intern(term::And, imp(b, a), imp(a, b)), 0);
  return b;
}

// negation normal form
// for-all vars map to fresh vars
// exists vars map to skolem functions
struct quant {
  bool exists;
  term var;
  term renamed;

  quant() {}

  quant(bool exists, term var, term renamed)
      : exists(exists), var(var), renamed(renamed) {}
};

struct nnf {
  vec<quant> boundvars;
  vec<pair<term, term>> freevars;
  unordered_map<type, si> newvars;
  term r;

  term args(bool pol, term a, term op) {
    auto n = size(a);
    vec<term> v(n);
    for (si i = 0; i != n; ++i)
      v[i] = convert(pol, at(a, i));
    return intern(op, v);
  }

  term all(bool pol, term a) {
    auto n = size(a);
    auto old = boundvars.n;
    boundvars.resize(old + n - 1);
    for (si i = 1; i != n; ++i) {
      auto x = at(a, i);
      auto t = vartype(x);
      boundvars[old + i - 1] = quant(0, x, var(t, newvars[t]++));
    }
    a = convert(pol, at(a, 0));
    boundvars.n = old;
    return a;
  }

  term exists(bool pol, term a) {
    auto n = size(a);
    auto old = boundvars.n;
    boundvars.resize(old + n - 1);
    for (si i = 1; i != n; ++i) {
      termv.n = 0;
      for (auto &j : boundvars)
        if (!j.exists)
          termv.push(j.renamed);
      auto x = at(a, i);
      auto y = skolemterms(vartype(x));
      boundvars[old + i - 1] = quant(1, x, y);
    }
    a = convert(pol, at(a, 0));
    boundvars.n = old;
    return a;
  }

  term convert(bool pol, term a) {
    switch (tag(a)) {
    case term::And:
      return args(pol, a, pol ? term::And : term::Or);
    case term::Eqv: {
      auto x = at(a, 0);
      if (nclauses(0, x) >= many || nclauses(1, x) >= many)
        x = rename_both(x);
      auto y = at(a, 1);
      if (nclauses(0, y) >= many || nclauses(1, y) >= many)
        y = rename_both(y);
      return intern(term::And, intern(term::Or, convert(0, x), convert(pol, y)),
                    intern(term::Or, convert(1, x), convert(pol ^ 1, y)));
    }
    case term::All:
      return pol ? all(pol, a) : exists(pol, a);
    case term::Exists:
      return pol ? exists(pol, a) : all(pol, a);
    case term::False:
      return term(pol ^ 1);
    case term::Not:
      return convert(pol ^ 1, at(a, 0));
    case term::Or:
      return args(pol, a, pol ? term::Or : term::And);
    case term::True:
      return term(pol);
    case term::Var:
      for (auto i = boundvars.rbegin(), e = boundvars.rend(); i != e; ++i)
        if (i->var == a)
          return i->renamed;
      for (auto &i : freevars)
        if (i.first == a)
          return i.second;
      auto t = vartype(a);
      auto b = var(t, newvars[t]++);
      freevars.push_back(make_pair(a, b));
      return b;
    }
    a = args(1, a, tag(a));
    return pol ? a : intern(term::Not, a);
  }

  nnf(term a) { r = convert(1, a); }
};

// distribute or into and
// return:
// at most one layer of and
// any number of layers of or
term distrib(term a) {
  switch (tag(a)) {
  case term::And: {
    vec<term> v;
    for (auto b : a) {
      b = distrib(b);
      if (tag(b) == term::And) {
        v.insert(v.end(), begin(b), end(b));
        continue;
      }
      v.push_back(b);
    }
    return intern(term::And, v);
  }
  case term::Or: {
    // take possibly nested ands and turn them into a layer no more than one
    // deep
    // also look for where there is going to be exponential blowup
    // and rename terms to avoid it
    si product = 1;
    vec<term> ands;
    for (auto b : a) {
      b = distrib(b);
      if (tag(b) == term::And) {
        auto m = size(b) - 1;
        if (product > 1 && m > 1 && product * m > 4) {
          ands.push_back(rename_pos(b));
          continue;
        }
        product *= m;
      }
      ands.push_back(b);
    }

    // a vector of indexes into and terms
    // that will provide a slice through the and arguments
    // to create a single or term
    auto n = ands.n;
    vec<si> j(n);
    memset(j.p, 0, n * sizeof *j.p);

    // the components of a single or term
    vec<term> or1(n);

    // all the or terms
    // that will become the arguments to an and
    vec<term> ors;

    // cartesian product of ands
    for (;;) {
      // make another or that takes a slice through the and args
      for (si i = 0; i != n; ++i) {
        auto b = ands[i];
        if (tag(b) == term::And)
          b = at(b, j[i]);
        else
          assert(!j[i]);
        or1[i] = b;
      }
      ors.push_back(intern(term::Or, or1));

      // take the next slice
      for (si i = n;;) {
        // if we have done all the slices, return and of ors
        if (!i)
          return intern(term::And, ors);

        // next element of the index vector
        // this is equivalent to increment with carry, of a multi-precision
        // integer except that the 'base', the maximum value of a 'digit', is
        // different for each place, being the number of arguments to the and at
        // that position
        auto b = ands[--i];
        if (tag(b) == term::And) {
          auto m = size(b) - 1;
          if (++j[i] == m) {
            j[i] = 0;
            // carry
            continue;
          }
          break;
        }
      }
    }
  }
  }
  return a;
}

// make clauses
void toliterals(term a) {
  switch (tag(a)) {
  case term::And:
    unreachable;
  case term::Not:
    if (neg.n >= 0xffff)
      throw 0;
    neg.push(at(a, 1));
    return;
  case term::Or:
    for (auto b : a)
      toliterals(b);
    return;
  }
  if (pos.n >= 0xffff)
    throw 0;
  pos.push(a);
}

void toclause(term a) {
  assert(tag(a) != term::And);
  assert(!neg.n);
  assert(!pos.n);
  toliterals(a);
  addclause(infer::cnf);
}

term cnf1(term a) {
  ck(a);

  // negation normal form
  nnf nnf1(a);
  a = nnf1.r;
  ck(a);

  // distribute or into and
  a = distrib(a);
  ck(a);
  return a;
}
} // namespace

void cnf(term a, clause *f) {
  try {
    if (tag(a) == term::And)
      for (auto b : a)
        toclause(b);
    else
      toclause(a);
  } catch (int e) {
    neg.n = pos.n = 0;
    complete = 0;
  }
}
