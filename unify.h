extern ary<pair<w, w>> varmap;

// match on top of existing map
bool match(w a, w b);

// unify on top of existing map
bool unify1(w a, w b);

// unify after clearing existing map
bool unify(w a, w b);
