extern ary<pair<term, term>> pairv;

// match on top of existing map
bool match(term a, term b);

// unify on top of existing map
bool unify1(term a, term b);

// unify after clearing existing map
bool unify(term a, term b);
