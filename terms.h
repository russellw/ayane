enum {
#define _(x) x,
#include "tags.h"
	ntags
};

// TODO: refactor source file organization?
struct Ex {
	int tag;
	uint32_t n;
	union {
		Ex* v[9];
		struct {
			Ex* ty;
			union {
				char s[9];
				size_t idx;
				mpz_t mpz;
				mpq_t mpq;
			};
		};
	};
};

// TODO: rename to 'atom'?
Ex* ex(mpz_t val);
Ex* ex(mpq_t val);

// Wrapping a symbol in a term is a common operation. Specifying the type at the same time is less so, but still common enough for
// this function to be useful.
Ex* ex(string* s, Ex* ty);

inline Ex* tbool(bool b) {
	return atoms + (b ? True : False);
}

Ex* var(size_t i, type ty);

Ex* gensym(type ty);
Ex* distinctObj(string* s);

// TODO: test using a bump allocator
Ex* ex(int tag);
Ex* ex(int tag, Ex* a);
Ex* ex(int tag, Ex* a, Ex* b);
Ex* ex(Ex* a, Ex* b);
Ex* ex(Ex* a, Ex* b, Ex* c);
Ex* ex(Ex* a, Ex* b, Ex* c, Ex* d);
Ex* ex(Ex* a, Ex* b, Ex* c, Ex* d, Ex* e);
Ex* ex(int tag, const vec<Ex*>& v);
Ex* ex(int tag, const std::vector<Ex*>& v);
type getType(Ex* a);

inline Ex* at(Ex* a, size_t i) {
	assert(i < a->n);
	return a->v[i];
}

type ftype(type rty, const Ex* first, const Ex* last);
type ftype(type rty, const vec<Ex*>& args);
void check(Ex* a, type ty);

void print(Ex* a);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using termx = pair<Ex*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
// TODO: this is not just a pair of arbitrary terms; it has special rules. Should it be a separate type?
using Eqn = pair<Ex*, Ex*>;
