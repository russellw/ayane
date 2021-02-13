enum {
  t_none,

  t_bool,
  t_int,
  t_rat,
  t_real,
  t_individual,

  basic_types
};

// SORT
void defaulttype(w t, w a);
w numtype(w a);
void requiretype(w t, w a);
w typeof(w a);
///
