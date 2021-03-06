#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
// estimate how many clauses a term will expand into, for the purpose of
// deciding when subformulas need to be renamed; the answer could exceed 2^31,
// but then we don't actually need the number, we only need to know whether it
// went over the threshold
const int many = 10;
int nclauses(bool pol, w a);

int nclauses_and(bool pol, w a) {
  int r = 0;
  for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
    r += nclauses(pol, *i);
    if (r >= many)
      return many;
  }
  return r;
}

int nclauses_or(bool pol, w a) {
  int r = 1;
  for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
    r *= nclauses(pol, *i);
    if (r >= many)
      return many;
  }
  return r;
}

int nclauses(bool pol, w a) {
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
    auto r = pol ? x0 * y1 + x1 * y0 : x0 * y0 + x1 * y1;
    if (r >= many)
      return many;
    return r;
  }
  case b_or:
    return pol ? nclauses_or(pol, a) : nclauses_and(pol, a);
  }
  return 1;
}

// creating new functions is necessary both to skolemize existential variables
// and to rename subformulas to avoid exponential blowup
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
  for (int i = 0; i != freevars.n; ++i)
    t[i + 1] = vartype(freevars[i]);

  // compound
  freevars.insert(freevars.p, skolem(type(t)));
  return term(freevars);
}

w skolemize(w rt, const vec<pair<w, w>> &u) {
  freevars.resize(u.n);
  for (int i = 0; i != u.n; ++i)
    freevars[i] = u[i].second;
  return skolemize(rt);
}

// rename subformulas to avoid exponential blowup
w rename_pos(w a) {
  getfree(a);
  auto b = skolemize(t_bool);
  cnf(imp(b, a), 0);
  return b;
}

w cnf1(w a);

w rename_both(w a) {
  a = cnf1(a);
  getfree(a);
  auto b = skolemize(t_bool);
  cnf(term(basic(b_and), imp(b, a), imp(a, b)), 0);
  return b;
}

// negation normal form
// for-all vars map to fresh vars
// exists vars map to skolem functions
struct nnf {
  vec<pair<w, w>> allvars;
  vec<pair<w, w>> existsvars;
  vec<pair<w, w>> freevars;
  unordered_map<w, w> newvars;
  w r;

  w args(bool pol, w a, w op) {
    auto n = size(a);
    vec<w> v;
    v.resize(n);
    v[0] = op;
    for (int i = 1; i != n; ++i)
      v[i] = convert(pol, at(a, i));
    return term(v);
  }

  w all(bool pol, w a) {
    auto n = size(a);
    auto old = allvars.n;
    allvars.resize(old + n - 2);
    for (int i = 2; i != n; ++i) {
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
    for (int i = 2; i != n; ++i)
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
          if (nclauses(0, x) >= many || nclauses(1, x) >= many)
            x = rename_both(x);
          auto y = at(a, 2);
          if (nclauses(0, y) >= many || nclauses(1, y) >= many)
            y = rename_both(y);
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
      for (auto i = allvars.rbegin(); i != allvars.rend(); ++i)
        if (i->first == a)
          return i->second;
      for (auto i = existsvars.rbegin(); i != existsvars.rend(); ++i)
        if (i->first == a)
          return i->second;
      for (auto &p : freevars)
        if (p.first == a)
          return p.second;
      auto t = vartype(a);
      auto &j = newvars[t];
      auto b = var(t, j++);
      freevars.push_back(make_pair(a, b));
      return b;
    }
    return pol ? a : term(basic(b_not), a);
  }

  nnf(w a) { r = convert(1, a); }
};

// distribute or into and
// return:
// at most one layer of and
// any number of layers of or
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
      v.push_back(b);
    }
    return term(v);
  }
  if (op == basic(b_or)) {
    // take possibly nested ands and turn them into a layer no more than one
    // deep
    // also look for where there is going to be exponential blowup
    // and rename terms to avoid it
    int product = 1;
    vec<w> ands;
    for (auto i = beginp(a) + 1, e = endp(a); i != e; ++i) {
      auto b = distrib(*i);
      if ((b & 7) == a_compound && at(b, 0) == basic(b_and)) {
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
    vec<int> j;
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
      for (int i = 0; i != n; ++i) {
        auto b = ands[i];
        if ((b & 7) == a_compound && at(b, 0) == basic(b_and))
          b = at(b, j[i] + 1);
        else
          assert(!j[i]);
        or1[i + 1] = b;
      }
      ors.push_back(term(or1));

      // take the next slice
      for (int i = n;;) {
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
  addclause(infer::cnf);
}

w cnf1(w a) {
  ckterm(a);

  // negation normal form
  nnf nnf1(a);
  a = nnf1.r;
  ckterm(a);

  // distribute or into and
  a = distrib(a);
  ckterm(a);
  return a;
}
} // namespace

void cnf(w a, clause *f) {
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
