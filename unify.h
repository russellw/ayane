// match on top of existing map
bool match(term a, term b);

// unify on top of existing map
bool unifyMore(term a, term b);

// unify after clearing existing map
bool unify(term a, term b);
