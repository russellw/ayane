#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// TODO: rename?
Expr* gensym(Type* ty) {
	auto a = (Expr*)malloc(offsetof(Expr, s) + sizeof(char*));
	a->tag = Tag::fn;
	a->ty = ty;
	a->s = 0;
	return a;
}

// Composite expressions
struct CompCmp {
	static bool eq(Tag tag, Expr** a, size_t n, Expr* b) {
		return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(Expr* a, Expr* b) {
		return eq(a->tag, a->v, a->n, b);
	}

	static size_t hash(Tag tag, Expr** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(Expr* a) {
		return hash(a->tag, a->v, a->n);
	}
};

static void clear(Expr** a) {
}

static Set<Tag, Expr**, Expr, CompCmp> comps;

Expr* expr(Tag tag, Expr* a, Expr* b) {
	static Expr* v[2];
	v[0] = a;
	v[1] = b;
	return comps.intern(tag, v, 2);
}

Expr* expr(Tag tag, const Vec<Expr*>& v) {
	assert(v.size());
	return comps.intern(tag, v.data, v.n);
}

Type* type(Expr* a) {
	switch (a->tag) {
	case Tag::add:
	case Tag::ceil:
	case Tag::div:
	case Tag::divEuclid:
	case Tag::divFloor:
	case Tag::divTrunc:
	case Tag::floor:
	case Tag::minus:
	case Tag::mul:
	case Tag::remEuclid:
	case Tag::remFloor:
	case Tag::remTrunc:
	case Tag::round:
	case Tag::sub:
	case Tag::trunc:
		return type(at(a, 0));
	case Tag::all:
	case Tag::and1:
	case Tag::eq:
	case Tag::eqv:
	case Tag::exists:
	case Tag::false1:
	case Tag::isInteger:
	case Tag::isRational:
	case Tag::le:
	case Tag::lt:
	case Tag::not1:
	case Tag::or1:
	case Tag::true1:
		return &tbool;
	case Tag::call:
		a = at(a, 0);
		assert(a->tag == Tag::fn);
		return at(a->ty, 0);
	case Tag::distinctObj:
		return &tindividual;
	case Tag::fn:
	case Tag::var:
		return a->ty;
	case Tag::integer:
	case Tag::toInteger:
		return &tinteger;
	case Tag::rational:
	case Tag::toRational:
		return &trational;
	case Tag::real:
	case Tag::toReal:
		return &treal;
	}
	unreachable;
}

Type* ftype(Type* rty, Expr** first, Expr** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return type(v);
}

int cmp(Expr* a, Expr* b) {
	assert(a->tag == b->tag);
	if (a == b) return 0;
	switch (a->tag) {
	case Tag::integer:
		return mpz_cmp(a->mpz, b->mpz);
	case Tag::rational:
	case Tag::real:
		return mpq_cmp(a->mpq, b->mpq);
	}
	unreachable;
}
