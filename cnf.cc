#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
w skolem(w t) {
  auto n = sprintf(buf, "\1%zu", skolemi++);
  auto s = intern(buf, n);
  s->ft = t;
  return tag(s, a_sym);
}

w skolem(w rt, vec<w> &v) {
  // atom
  if (!v.n)
    return skolem(rt);

  // compound type
  vec<uint16_t> t;
  t.resize(v.n + 1);
  t[0] = rt;
  for (w i = 0; i != v.n; ++i)
    t[i + 1] = vartype(v[i]);

  // compound
  v.insert(0, skolem(type(t)));
  return term(v);
}

w skolem(w rt, const vec<pair<w, w>> &u) {
  vec<w> v;
  v.resize(u.n);
  for (w i = 0; i != u.n; ++i)
    v[i] = u[i].second;
  return skolem(rt, v);
}

// rename a term to avoid exponential blowup
bool iscomplex(w a) {
  while ((a & 7) == a_compound && at(a, 0) == basic(b_not))
    a = at(a, 1);
  if ((a & 7) != a_compound)
    return 0;
  auto op = at(a, 0);
  if ((op & 7) != a_basic)
    return 0;
  switch (op >> 3) {
  case b_all:
  case b_eqv:
  case b_exists:
    unreachable;
  case b_and:
  case b_or:
    return 1;
  }
  return 0;
}

w ren_pos(w a) {
  getfree(a);
  auto b = skolem(0, freevars);
  cnf(implies(b, a));
  return b;
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
          make_pair(at(a, i), skolem(vartype(at(a, i)), allvars));
    a = convert(pol, at(a, 1));
    existsvars.n = old;
    return a;
  }

  w convert(bool pol, w a) {
    switch (a & 7) {
    case a_basic:
      switch (a >> 3) {
      case b_false:
        return basic(!pol);
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
                      term(basic(b_or), convert(1, x), convert(!pol, y)));
        }
        case b_all:
          return pol ? all(pol, a) : exists(pol, a);
        case b_exists:
          return pol ? exists(pol, a) : all(pol, a);
        case b_not:
          return convert(!pol, at(a, 0));
        case b_or:
          return args(pol, a, pol ? basic(b_or) : basic(b_and));
        }
      return args(pol, a, op);
    }
    case a_var:
      for (auto p : allvars)
        if (p.first == a)
          return p.second;
      for (auto p : existsvars)
        if (p.first == a)
          return p.second;
      unreachable;
    }
    return pol ? a : term(basic(b_not), a);
  }

  nnf(w a) { r = convert(true, a); }
};

// distribute or into and
// return:
// at most one layer of and
// any number of layers of or

int and_size(w a) {
  if (a->tag == t_and)
    return a->n;
  return 1;
}

w and_at(w a, int i) {
  if (a->tag == t_and)
    return at(a, i);
  assert(!i);
  return a;
}

w distribute(w a) {
  switch (a->tag) {
  case t_and: {
    vec<w> v;
    for (auto i : a) {
      auto b = distribute(at(a, i));
      if (b->tag == t_and) {
        v.insert(v.end(), b->args, b->args + b->n);
        continue;
      }
      v.push_back(b);
    }
    return make(t_and, v);
  }
  case t_or: {
    // flat layer of ands
    int64_t n = 1;
    vec<w> ands(a->n);
    for (auto i : a) {
      auto b = distribute(at(a, i));
      if (n > 1 && and_size(b) > 1 && n * and_size(b) > 4)
        b = ren_pos(b);
      n *= and_size(b);
      ands[i] = b;
    }

    // cartesian product of ands
    vec<int> j(ands.size());
    memset(j.data(), 0, j.n * sizeof(int));
    vec<w> or_args(ands.n);
    vec<w> ors;
    for (;;) {
      for (int i = 0; i != ands.n; ++i)
        or_args[i] = and_at(ands[i], j[i]);
      ors.push_back(make(t_or, or_args));
      for (int i = ands.n;;) {
        if (!i)
          return make(t_and, ors);
        --i;
        if (++j[i] < and_size(ands[i]))
          break;
        j[i] = 0;
      }
    }
  }
  default:
    return a;
  }
}

// make clauses

void clausify(w a) {
  switch (a->tag) {
  case t_and:
    assert(false);
  case t_not:
    neg.push_back(at(a, 0));
    break;
  case t_or:
    for (auto i : a)
      clausify(at(a, i));
    break;
  default:
    pos.push_back(a);
    break;
  }
}

void clausify_ors(w a) {
  assert(a->tag != t_and);
  assert(neg.empty());
  assert(pos.empty());
  clausify(a);
  make_clause();
}
} // namespace

void cnf(clause *f) {
  assert(f->fof);
  auto a = f->v[0];

  // variables must be bound only for the first step
#ifdef DEBUG
  getfree(a);
  assert(!freevars.n);
#endif

  // negation normal form
  nnf nnf1(a);
  a = nnf1.r;

  // distribute or into and
  a = distribute(a);

  // make clauses
  if (a->tag == t_and)
    for (auto i : a)
      clausify_ors(at(a, i));
  else
    clausify_ors(a);
}
