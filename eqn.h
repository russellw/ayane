struct eqn {
  w left, right;

  explicit eqn(w a) {
    assert(typeof(a) == type::Bool);
    if ((a & 7) == a_compound && at(a, 0) == basic(b_eq)) {
      left = at(a, 1);
      right = at(a, 2);
    } else {
      left = a;
      right = basic(b_true);
    }
    assert(typeof(left) == typeof(right));
  }
};
