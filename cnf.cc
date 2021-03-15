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
  return tag(term::Sym, s);
}

term skolemterms(type rt) {
  // atom
  auto n = termv.n;
  if (!n)
    return skolem(rt);

  // compound type
  vec<type> v(n + 1);
  v[0] = rt;
  for (si i = 0; i != n; ++i)
    v[i + 1] = vartype(termv[i]);

  // compound
  auto r = comp(n + 1);
  *r->v = skolem(mktype(v));
  memcpy(r->v + 1, termv.p, n * sizeof(term));
  return tag(term::Call, r);
}

// rename subformulas to avoid exponential blowup
term rename_pos(term a) {
  getfree(a);
  auto b = skolemterms(type::Bool);
  cnf(comp(term::Imp, b, a), 0);
  return b;
}

term cnf1(term a);

term rename_both(term a) {
  a = cnf1(a);
  getfree(a);
  auto b = skolemterms(type::Bool);
  cnf(comp(term::And, comp(term::Imp, b, a), comp(term::Imp, a, b)), 0);
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

struct converting {
  vec<quant> boundvars;
  vec<pair<term, term>> freevars;
  unordered_map<type, si> newvars;
  term r;

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

  void andPush(term a, vec<term> &r) {
    if (tag(a) == term::And) {
      r.insert(r.end(), begin(a), end(a));
      return;
    }
    r.push_back(a);
  }

  term and2(term a, term b) {
    vec<term> v;
    andPush(a, v);
    andPush(b, v);
    return comp(term::And, v);
  }

  void andConvertPush(bool pol, term a, vec<term> &r) {
    a = convert(pol, a);
    andPush(a, r);
  }

  term andConvertArgs(bool pol, term a) {
    vec<term> v;
    for (auto b : a)
      andConvertPush(pol, b, v);
    return comp(term::And, v);
  }

  term andConvert2(bool pol_a, term a, bool pol_b, term b) {
    vec<term> v;
    andConvertPush(pol_a, a, v);
    andConvertPush(pol_b, b, v);
    return comp(term::And, v);
  }

  void orConvertPush(bool pol, term a, vec<term> &r, si &nc) {
    a = convert(pol, a);
    if (tag(a) == term::And) {
      auto old = nc;
      nc *= size(a);
      if (nc >= many) {
        nc = many;
        if (old > 1) {
          r.push_back(rename_pos(a));
          return;
        }
      }
    }
    r.push_back(a);
  }

  term distrib(const vec<term> &ands) {
    // a vector of indexes into And terms
    // that will provide a slice through the And arguments
    // to create a single Or term
    auto n = ands.n;
    vec<si> j(n);
    memset(j.p, 0, n * sizeof *j.p);

    // the components of a single Or term
    vec<term> literals(n);

    // all the Or terms
    // that will become the arguments to an And
    vec<term> ors;

    // cartesian product of Ands
    for (;;) {
      // make another Or that takes a slice through the And args
      for (si i = 0; i != n; ++i) {
        auto b = ands[i];
        if (tag(b) == term::And)
          b = at(b, j[i]);
        else
          assert(!j[i]);
        literals[i] = b;
      }
      ors.push_back(comp(term::Or, literals));

      // take the next slice
      for (si i = n;;) {
        // if we have done all the slices, return And of Ors
        if (!i)
          return comp(term::And, ors);

        // next element of the index vector
        // this is equivalent to increment with carry, of a multi-precision
        // integer, except that the 'base', the maximum value of a 'digit', is
        // different for each place, being the number of arguments to the And at
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

  term orConvertArgs(bool pol, term a) {
    vec<term> v;
    si nc = 1;
    for (auto b : a)
      orConvertPush(pol, b, v, nc);
    return distrib(v);
  }

  term orConvert2(bool pol_a, term a, bool pol_b, term b) {
    vec<term> v;
    si nc = 1;
    orConvertPush(pol_a, a, v, nc);
    orConvertPush(pol_b, b, v, nc);
    return distrib(v);
  }

  term convert(bool pol, term a) {
    switch (tag(a)) {
    case term::All:
      return pol ? all(pol, a) : exists(pol, a);
    case term::And:
      return pol ? andConvertArgs(pol, a) : orConvertArgs(pol, a);
    case term::Eqv: {
      auto x = at(a, 0);
      if (nclauses(0, x) >= many || nclauses(1, x) >= many)
        x = rename_both(x);
      auto y = at(a, 1);
      if (nclauses(0, y) >= many || nclauses(1, y) >= many)
        y = rename_both(y);
      return and2(orConvert2(0, x, pol, y), orConvert2(1, x, pol ^ 1, y));
    }
    case term::Imp: {
      auto x = at(a, 0);
      auto y = at(a, 1);
      return pol ? orConvert2(0, x, 1, y) : andConvert2(1, x, 0, y);
    }
    case term::Exists:
      return pol ? exists(pol, a) : all(pol, a);
    case term::False:
      return term(pol ^ 1);
    case term::Not:
      return convert(pol ^ 1, at(a, 0));
    case term::Or:
      return pol ? orConvertArgs(pol, a) : andConvertArgs(pol, a);
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
    auto n = size(a);
    auto r = comp(n);
    for (si i = 0; i != n; ++i)
      r->v[i] = convert(1, at(a, i));
    a = tag(tag(a), r);
    return pol ? a : comp(term::Not, a);
  }

  converting(term a) { r = convert(1, a); }
};

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
  converting co(a);
  a = co.r;
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
