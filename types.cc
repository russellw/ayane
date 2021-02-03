#include "main.h"

void defaultType(w t, w a) {
  assert(!istcompound(t));
  switch (a & 7) {
  case a_sym:
    if (symp(a)->ft)
      break;
    symp(a)->ft = t;
    return;
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) != a_sym)
      break;
    auto ft = symp(op)->ft;
    if (ft)
      break;
    auto n = size(a);
    vec<uint16_t> v;
    v.resize(n);
    v[0] = t;
    for (w i = 1; i != n; ++i)
      v[i] = typeof(at(a, i));
    symp(op)->ft = type(v);
    return;
  }
  }
}

void requireType(w t, w a) {
  assert(!istcompound(t));
  switch (a & 7) {
  case a_sym:
    if (symp(a)->ft)
      break;
    symp(a)->ft = t;
    return;
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) != a_sym)
      break;
    auto ft = symp(op)->ft;
    if (ft)
      break;
    auto n = size(a);
    vec<uint16_t> v;
    v.resize(n);
    v[0] = t;
    for (w i = 1; i != n; ++i)
      v[i] = typeof(at(a, i));
    symp(op)->ft = type(v);
    return;
  }
  }
  if (t != typeof(a))
    throw "Type mismatch";
}

w typeof(w a) {
  switch (a & 7) {
  case a_compound: {
    auto op = at(a, 0);
    switch (op & 7) {
    case a_sym: {
      auto ft = symp(op)->ft;
      assert(istcompound(ft));
      auto ftp = tcompoundp(ft);
      assert(size(a) == ftp->n);
      return ftp->v[0];
    }
    case a_basic:
      switch (op >> 3) {
      case b_eq:
      case b_and:
      case b_or:
      case b_eqv:
      case b_not:
      case b_all:
      case b_exists:
      case b_isint:
      case b_israt:
      case b_lt:
      case b_le:
        return t_bool;
      case b_toint:
        return t_int;
      case b_torat:
        return t_rat;
      case b_toreal:
        return t_real;
      }
      return typeof(at(a, 1));
    }
    break;
  }
  case a_int:
    return t_int;
  case a_rat:
    return t_rat;
  case a_real:
    return t_real;
  case a_sym:
    return symp(a)->ft;
  case a_var:
    return a >> 3 & 0xffff;
  case a_basic:
    assert(a >> 3 == b_false || a >> 3 == b_true);
    return t_bool;
  }
  unreachable;
  return 0;
}
