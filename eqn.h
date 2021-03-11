struct eqn {
  term left, right;

  explicit eqn(term a) {
    assert(typeof(a) == type::Bool);
    if (tag(a) == term::Eq) {
      left = at(a, 0);
      right = at(a, 1);
    } else {
      left = a;
      right = term::True;
    }
    assert(typeof(left) == typeof(right));
  }
};
