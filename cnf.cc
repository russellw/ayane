#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
const uint64_t many = 1000;
uint64_t nclauses(bool pol, w a);

uint64_t nclauses_and(bool pol, w a) {
  uint64_t r = 0;
  for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
    r += nclauses(pol, *i);
    if (r >= many)
      return many;
  }
  return r;
}

uint64_t nclauses_or(bool pol, w a) {
  uint64_t r = 1;
  for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
    r *= nclauses(pol, *i);
    if (r >= many)
      return many;
  }
  return r;
}

uint64_t nclauses(bool pol, w a) {
  if ((a & 7) != a_compound)
    return 1;
  auto op = at(a, 0);
  if ((op & 7) != a_basic)
    return 1;
  switch (op >> 3) {
  case b_all:
  case b_exists:
  case b_not:
    return nclauses(pol, at(a, 1));
  case b_and:
    return pol ? nclauses_and(pol, a) : nclauses_or(pol, a);
  case b_or:
    return pol ? nclauses_or(pol, a) : nclauses_and(pol, a);
  case b_eqv: {
    auto x = at(a, 1);
    auto x0 = nclauses(0, x);
    if (x0 >= many - 1)
      return many;
    auto x1 = nclauses(1, x);
    if (x1 >= many - 1)
      return many;
    auto y = at(a, 2);
    auto y0 = nclauses(0, y);
    if (y0 >= many - 1)
      return many;
    auto y1 = nclauses(1, y);
    auto r = pol ? x0 * y1 + x1 * y0 : x0 * x1 + y0 * y1;
    if (r >= many)
      return many;
    return r;
  }
  }
  return 1;
}

w skolem(w t) {
  auto n = sprintf(buf, "\1%zu", skolemi++);
  auto s = intern(buf, n);
  s->ft = t;
  return tag(s, a_sym);
}

w skolemize(w rt) {
  // atom
  if (!freevars.n)
    return skolem(rt);

  // compound type
  vec<uint16_t> t;
  t.resize(freevars.n + 1);
  t[0] = rt;
  for (w i = 0; i != freevars.n; ++i)
    t[i + 1] = vartype(freevars[i]);

  // compound
  freevars.insert(freevars.p, skolem(type(t)));
  return term(freevars);
}

w skolemize(w rt, const vec<pair<w, w>> &u) {
  freevars.resize(u.n);
  for (w i = 0; i != u.n; ++i)
    freevars[i] = u[i].second;
  return skolemize(rt);
}

// negation normal form
// for-all vars map to fresh vars
// exists vars map to skolem functions
struct nnf {
  vec<pair<w, w>> allvars;
  vec<pair<w, w>> existsvars;
  unordered_map<w, w> newvars;
  w r;

  w args(bool pol, w a, w op) {
    auto n = size(a);
    vec<w> v;
    v.resize(n);
    v[0] = op;
    for (w i = 1; i != n; ++i)
      v[i] = convert(pol, at(a, i));
    return term(v);
  }

  w all(bool pol, w a) {
    auto n = size(a);
    auto old = allvars.n;
    allvars.resize(old + n - 2);
    for (w i = 2; i != n; ++i) {
      auto t = vartype(at(a, i));
      auto &j = newvars[t];
      allvars[old + i - 2] = make_pair(at(a, i), var(t, j++));
    }
    a = convert(pol, at(a, 1));
    allvars.n = old;
    return a;
  }

  w exists(bool pol, w a) {
    auto n = size(a);
    auto old = existsvars.n;
    existsvars.resize(old + n - 2);
    for (w i = 2; i != n; ++i)
      existsvars[old + i - 2] =
          make_pair(at(a, i), skolemize(vartype(at(a, i)), allvars));
    a = convert(pol, at(a, 1));
    existsvars.n = old;
    return a;
  }

  w convert(bool pol, w a) {
    switch (a & 7) {
    case a_basic:
      switch (a >> 3) {
      case b_false:
        return basic(pol ^ 1);
      case b_true:
        return basic(pol);
      }
      unreachable;
    case a_compound: {
      auto op = at(a, 0);
      if ((op & 7) == a_basic)
        switch (op >> 3) {
        case b_and:
          return args(pol, a, pol ? basic(b_and) : basic(b_or));
        case b_eqv: {
          auto x = at(a, 1);
          auto y = at(a, 2);
          return term(basic(b_and),
                      term(basic(b_or), convert(0, x), convert(pol, y)),
                      term(basic(b_or), convert(1, x), convert(pol ^ 1, y)));
        }
        case b_all:
          return pol ? all(pol, a) : exists(pol, a);
        case b_exists:
          return pol ? exists(pol, a) : all(pol, a);
        case b_not:
          return convert(pol ^ 1, at(a, 1));
        case b_or:
          return args(pol, a, pol ? basic(b_or) : basic(b_and));
        }
      a = args(1, a, op);
      break;
    }
    case a_var:
      for (auto &p : allvars)
        if (p.first == a)
          return p.second;
      for (auto &p : existsvars)
        if (p.first == a)
          return p.second;
      unreachable;
    }
    return pol ? a : term(basic(b_not), a);
  }

  nnf(w a) { r = convert(1, a); }
};

// distribute or into and
// return:
// at most one layer of and
// any number of layers of or
w rename(w a) {
  getfree(a);
  auto b = skolemize(t_bool);
  a = imp(b, a);
  if (freevars.n) {
    freevars[0] = basic(b_all);
    freevars.insert(freevars.p + 1, a);
    a = term(freevars);
  }
  cnf(formula(0, a));
  return b;
}

w distrib(w a) {
  if ((a & 7) != a_compound)
    return a;
  auto op = at(a, 0);
  if (op == basic(b_and)) {
    vec<w> v(basic(b_and));
    for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
      auto b = distrib(*i);
      if ((b & 7) == a_compound && at(b, 0) == basic(b_and)) {
        v.insert(v.end(), beginp(b) + 1, endp(b));
        continue;
      }
      v.push(b);
    }
    return term(v);
  }
  if (op == basic(b_or)) {
    // take possibly nested ands and turn them into a layer no more than one
    // deep
    // also look for where there is going to be exponential blowup
    // and rename terms to avoid it
    uint64_t product = 1;
    vec<w> ands;
    for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
      auto b = distrib(*i);
      if ((b & 7) == a_compound && at(b, 0) == basic(b_and)) {
        auto m = size(b) - 1;
        if (product > 1 && m > 1 && product * m > 4) {
          ands.push(rename(b));
          continue;
        }
        product *= m;
      }
      ands.push(b);
    }

    // a vector of indexes into and terms
    // that will provide a slice through the and arguments
    // to create a single or term
    auto n = ands.n;
    vec<w> j;
    j.resize(n);
    memset(j.p, 0, n * sizeof *j.p);

    // the components of a single or term
    vec<w> or1;
    or1.resize(n + 1);
    or1[0] = basic(b_or);

    // all the or terms
    // that will become the arguments to an and
    vec<w> ors(basic(b_and));

    // cartesian product of ands
    for (;;) {
      // make another or that takes a slice through the and args
      for (w i = 0; i != n; ++i) {
        auto b = ands[i];
        if ((b & 7) == a_compound && at(b, 0) == basic(b_and))
          b = at(b, j[i] + 1);
        else
          assert(!j[i]);
        or1[i + 1] = b;
      }
      ors.push(term(or1));

      // take the next slice
      for (w i = n;;) {
        // if we have done all the slices, return and of ors
        if (!i)
          return term(ors);

        // next element of the index vector
        // this is equivalent to increment with carry, of a multi-precision
        // integer except that the 'base', the maximum value of a 'digit', is
        // different for each place, being the number of arguments to the and at
        // that position
        auto b = ands[--i];
        if ((b & 7) == a_compound && at(b, 0) == basic(b_and)) {
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
  return a;
}

// make clauses
void toliterals(w a) {
  if ((a & 7) == a_compound) {
    auto op = at(a, 0);
    assert(op != basic(b_and));
    if (op == basic(b_not)) {
      if (neg.n == 0xffff)
        throw 0;
      neg.push(at(a, 1));
      return;
    }
    if (op == basic(b_or)) {
      for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i)
        toliterals(*i);
      return;
    }
  }
  if (pos.n == 0xffff)
    throw 0;
  pos.push(a);
}

void toclause(w a) {
  assert(!((a & 7) == a_compound && at(a, 0) == basic(b_and)));
  assert(!neg.n);
  assert(!pos.n);
  toliterals(a);
  addclause(i_cnf);
}
} // namespace

void cnf(clause *f) {
  assert(f->fof);
  auto a = *f->v;
  ckterm(a);

  // variables must be bound only for the first step
#ifdef DEBUG
  getfree(a);
  assert(!freevars.n);
#endif

  // negation normal form
  nnf nnf1(a);
  a = nnf1.r;
  ckterm(a);

  // distribute or into and
  a = distrib(a);
  ckterm(a);

  // make clauses
  try {
    if ((a & 7) == a_compound && at(a, 0) == basic(b_and))
      for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i)
        toclause(*i);
    else
      toclause(a);
  } catch (int e) {
    neg.n = pos.n = 0;
    complete = 0;
  }
}
