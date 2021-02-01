#include "main.h"

ty typeof(w a) {
  switch (a & 7) {
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) == a_sym) {
      auto t = symp(op)->ft;
      assert(istcompound(t));
      auto t1 = tcompoundp(t);
      assert(size(a) == t1->n);
      return t1->v[0];
    }
    unreachable;
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
    return a >> 3 & (1 << 8 * sizeof(ty)) - 1;
  }
  unreachable;
  return 0;
}
