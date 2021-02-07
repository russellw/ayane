#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
w skolem(w t) {
  auto n = sprintf(buf, "\1%zu", skolemi++);
  return fn(t, intern(buf, n));
}

w skolem(w t, const vec<w> &v) {
  auto a = constant(t, 0);
  if (!v.n)
    return a;
  // don't care about params because type check is already done
  return make(t_call, a, v);
}

w skolem(w t, const vec<pair<w, w>> &v) {
  vec<w> w(v.size());
  for (int i = 0; i != v.size(); ++i)
    w[i] = v[i].second;
  return skolem(t, w);
}

// rename a term to avoid exponential blowup

bool iscomplex(w a) {
  while ((a & 7) == a_compound && at(a, 0) == basic(b_not))
    a = at(a, 1);
  switch (a->tag) {
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
  vec<pair<w, w>> &allvars;
  vec<pair<w, w>> &existsvars;
  w r;

  w args(bool pol, w a, tag_t tag) {
    vec<w> v(a->n);
    for (auto i : a)
      v[i] = convert(pol, at(a, i));
    return make(tag, v);
  }

  w literal(bool pol, w a) {
    switch (a->tag) {
    case t_all:
    case t_and:
    case t_eqv:
    case t_exists:
    case t_or:
      assert(false);
    case t_false:
      return make(!pol);
    case t_not:
      return literal(!pol, at(a, 0));
    case t_true:
      return make(pol);
    }
    return pol ? a : make(t_not, a);
  }

  w all(bool pol, w a) {
    auto n = size(a);
    auto old = allvars.n;
    allvars.resize(old + n - 2);
    for (int i = 2; i != n; ++i)
      allvars[old + i - 2] = make_pair(at(a, i), var(vartype(at(a, i))));
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
    switch (a->tag) {
    case t_and:
      return args(pol, a, pol ? t_and : t_or);
    case t_eqv: {
      auto x = ren_eqv(convert(true, at(a, 0)));
      auto y = ren_eqv(convert(true, at(a, 1)));
      return make(t_and, make(t_or, literal(false, x), literal(pol, y)),
                  make(t_or, literal(true, x), literal(!pol, y)));
    }
    case t_all:
      if (pol)
        return all(pol, a);
      else
        return exists(pol, a);
    case t_exists:
      if (pol)
        return exists(pol, a);
      else
        return all(pol, a);
    case t_false:
      return make(!pol);
    case t_or:
      return args(pol, a, pol ? t_or : t_and);
    case t_true:
      return make(pol);
    case t_var:
      for (auto p : allvars)
        if (p.first == a)
          return p.second;
      for (auto p : existsvars)
        if (p.first == a)
          return p.second;
      assert(false);
    case t_not:
      return convert(!pol, at(a, 0));
    }
    if (a->n)
      a = args(true, a, a->tag);
    return pol ? a : make(t_not, a);
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
