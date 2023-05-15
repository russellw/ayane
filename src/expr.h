enum class Tag {
#define _(a) a,
#include "tags.h"
};

// TODO: refactor source file organization
// Expressions are the most important data structures in the system. Logic formulas are expressions of Boolean type; terms are
// expressions of other types.
struct Expr {
	Tag tag;
	uint32_t n = 0;

	Expr(Tag tag): tag(tag) {
	}
};

// The Boolean constants are primitive expressions. False and true are represented as &bools[value], i.e. bools and bools+1
// respectively.
extern Expr bools[2];

Type* type(Expr* a);

// Composite expressions. Built-in operators like equality and addition are represented by tags. The most important variety of
// composite expression is the function call, represented by Tag::call, with the function as v[0], so in that case the arguments
// start at 1.
struct Comp: Expr {
	Expr* v[];

	Comp(Tag tag, size_t n): Expr(tag) {
		this->n = n;
	}
};

inline Expr** begin(Expr* a) {
	return ((Comp*)a)->v;
}
inline Expr** end(Expr* a) {
	return ((Comp*)a)->v + a->n;
}

inline Expr* at(Expr* a, size_t i) {
	assert(i < a->n);
	return ((Comp*)a)->v[i];
}

// TODO: test using a bump allocator
Expr* comp(Tag tag, Expr* a);
Expr* comp(Tag tag, Expr* a, Expr* b);
Expr* comp(Tag tag, Vec<Expr*>& v);
Expr* comp(Tag tag, vector<Expr*>& v);

Expr* compc(Tag tag, Vec<Expr*>& v);

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct Eqn: pair<Expr*, Expr*> {
	Eqn(Expr* a);
};

#ifdef DBG
void print(Tag tag);
void print(Expr* a);
#endif
