#include "main.h"

// Atoms
Atom atoms[ntags];

namespace {
struct init {
	init() {
		auto n = (int)end;
		for (int i = 0; i < n; ++i) {
			auto p = (Atom*)atoms->ptr(tatoms + i * offsetof(atom, s) / 8);
			p->t = (tag)i;
		}
	}
} _;

map<type, vec<ex>> boxedVars;
} // namespace

ex var(size_t i, type ty) {
	auto unboxed = 1 << ex::idxBits;
	if (i < unboxed) {
		ex a;
		a.raw = ex::t_var | i << typeBits | ty.offset;
		return a;
	}
	auto& v = boxedVars.gadd(ty);
	while (v.size() <= i - unboxed) {
		auto o = atoms->alloc(offsetof(atom, idx) + 4);
		auto p = (Atom*)atoms->ptr(o);
		p->t = Var;
		p->ty = ty;
		p->idx = unboxed + v.size();
		ex a;
		a.raw = ex::t_var | ex::t_boxed | o;
		v.add(a);
	}
	return v[i - unboxed];
}

ex distinctObj(string* s) {
	ex a;
	if (s->dobj) {
		a.raw = s->dobj;
		return a;
	}
	auto o = atoms->alloc(offsetof(atom, s) + sizeof s);
	auto p = (Atom*)atoms->ptr(o);
	p->t = DistinctObj;
	p->s = s->v;
	a.raw = s->dobj = o;
	return a;
}

Ex* ex(string* s, type ty) {
	if (s->sym) {
		auto p = (Atom*)atoms->ptr(s->sym);
		assert(p->t == Fn);
		assert(p->s == s->v);
		assign(p->ty, ty);
	} else {
		s->sym = atoms->alloc(offsetof(atom, s) + sizeof s);
		auto p = (Atom*)atoms->ptr(s->sym);
		p->t = Fn;
		p->s = s->v;
		p->ty = ty;
	}
	raw = s->sym;
}

ex gensym(type ty) {
	auto o = atoms->alloc(offsetof(atom, s) + sizeof(char*));
	auto p = (Atom*)atoms->ptr(o);
	p->t = Fn;
	p->s = 0;
	p->ty = ty;
	ex a;
	a.raw = o;
	return a;
}

// Compounds
Heap<>* compounds;

namespace comps {
bool eq(const Ex* s, size_t n, const compound* z) {
	if (n != z->n) return 0;
	return !memcmp(s, z->v, n * sizeof *s);
}

size_t slot(uint32_t* entries, size_t cap, const Ex* s, size_t n) {
	size_t mask = cap - 1;
	auto i = fnv(s, n * sizeof *s) & mask;
	while (entries[i] && !eq(s, n, (compound*)compounds->ptr(entries[i]))) i = (i + 1) & mask;
	return i;
}

size_t cap = 0x100;
size_t qty;
uint32_t* entries;

struct init {
	init() {
		compounds = Heap<>::make();
		assert(isPow2(cap));
		entries = (uint32_t*)compounds->ptr(compounds->calloc(cap * 4));
	}
} _;

void expand() {
	auto cap1 = cap * 2;
	auto entries1 = (uint32_t*)compounds->ptr(compounds->calloc(cap1 * 4));
	for (size_t i = 0; i < cap; ++i) {
		auto o = entries[i];
		if (!o) continue;
		auto s = (compound*)compounds->ptr(o);
		entries1[slot(entries1, cap1, s->v, s->n)] = o;
	}
	compounds->free(compounds->offset(entries), cap * 4);
	cap = cap1;
	entries = entries1;
}

size_t intern(const Ex* s, size_t n) {
	incStat("ex");
	auto i = slot(entries, cap, s, n);

	// If we have seen this before, return the existing object
	if (entries[i]) return entries[i];

	// Expand the hash table if necessary
	if (++qty > cap * 3 / 4) {
		expand();
		i = slot(entries, cap, s, n);
		assert(!entries[i]);
	}

	// Make a new object
	incStat("ex alloc");
	incStat("ex alloc bytes", offsetof(compound, v) + n * sizeof *s);
	auto o = compounds->alloc(offsetof(compound, v) + n * sizeof *s);
	if (o & ex::t_compound) err("compound ex: Out of memory");
	auto p = (compound*)compounds->ptr(o);
	p->n = n;
	memcpy(p->v, s, n * sizeof *s);

	// Add to hash table
	return entries[i] = o;
}
} // namespace comps

Ex* ex(ex a, ex b) {
	const int n = 2;
	ex v[n];
	v[0] = a;
	v[1] = b;
	raw = t_compound | comps::intern(v, n);
}

Ex* ex(ex a, ex b, ex c) {
	const int n = 3;
	ex v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	raw = t_compound | comps::intern(v, n);
}

Ex* ex(ex a, ex b, ex c, ex d) {
	const int n = 4;
	ex v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
	raw = t_compound | comps::intern(v, n);
}

Ex* ex(ex a, ex b, ex c, ex d, ex e) {
	const int n = 5;
	ex v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
	v[4] = e;
	raw = t_compound | comps::intern(v, n);
}

Ex* ex(const vec<ex>& v) {
	assert(v.size());
	if (v.size() == 1) *this = v[0];
	else
		raw = t_compound | comps::intern(v.data(), v.size());
}

ex::operator type() const {
	switch (tag(*this)) {
	case Add:
	case Ceil:
	case Div:
	case DivE:
	case DivF:
	case DivT:
	case Floor:
	case Mul:
	case Neg:
	case RemE:
	case RemF:
	case RemT:
	case Round:
	case Sub:
	case Trunc:
		return type((*this)[1]);
	case All:
	case And:
	case Eq:
	case Eqv:
	case Exists:
	case False:
	case IsInteger:
	case IsRational:
	case Le:
	case Lt:
	case Not:
	case Or:
	case True:
		return kind::Bool;
	case DistinctObj:
		return kind::Individual;
	case Fn:
	{
		// TODO: optimize
		auto ty = getAtom()->ty;
		if (kind(ty) == kind::Fn) return ty[0];
		return ty;
	}
	case Integer:
	case ToInteger:
		return kind::Integer;
	case Rational:
	case ToRational:
		return kind::Rational;
	case ToReal:
		return kind::Real;
	case Var:
		if (raw & t_boxed) return getAtom()->ty;
		return type(raw & (1 << typeBits) - 1);
	}
	unreachable;
}

type ftype(type rty, const Ex* first, const Ex* last) {
	if (first == last) return rty;
	vec<type> v(1, rty);
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return type(kind::Fn, v);
}

type ftype(type rty, const vec<ex>& args) {
	if (args.size()) return rty;
	vec<type> v(1, rty);
	for (auto a: args) v.add(type(a));
	return type(kind::Fn, v);
}

// TODO: eliminate this?
int cmp(ex a, ex b) {
	// Fast test for equality
	if (a == b) return 0;

	// If the tags differ, just sort in tag order; not meaningful, but it doesn't have to be meaningful, just consistent
	if (a->tag != b->tag) return (int)a->tag - (int)b->tag;

	// Numbers sort in numerical order
	switch (a->tag) {
	case Integer:
		return mpz_cmp(a.mpz(), b.mpz());
	case Rational:
		return mpq_cmp(a.mpq(), b.mpq());
	}

	// Compound terms sort in lexicographic order
	auto an = a.size();
	auto bn = b.size();
	auto n = min(an, bn);
	for (size_t i = 0; i < n; ++i) {
		auto c = cmp(at(a, i), b[i]);
		if (c) return c;
	}
	if (an - bn) return an - bn;

	// They are different terms with the same tags and no different components, so they must be different atoms; just do a straight
	// binary comparison
	return memcmp(&a, &b, sizeof a);
}

void print(int tag) {
	// TODO: eliminate
	static const char* tagNames[] = {
#define _(x) #x,
#include "tags.h"
	};
	print(tagNames[(int)t]);
}

void print(ex a) {
	switch (a->tag) {
	case Fn:
	{
		auto p = a.getAtom();
		auto s = p->s;
		if (s) print(s);
		else
			printf("_%p", p);
		break;
	}
	case Integer:
		mpz_out_str(stdout, 10, a.mpz());
		return;
	case Rational:
		mpq_out_str(stdout, 10, a.mpq());
		return;
	case Var:
		printf("%%%zu", a.varIdx());
		return;
	default:
		print(a->tag);
		break;
	}
	if (a.size() == 1) return;
	putchar('(');
	joining;
	for (auto b: a) {
		join(", ");
		print(b);
	}
	putchar(')');
}

static void check(ex a, size_t arity) {
	if (a.size() - 1 == arity) return;
	sprintf(buf, "Expected %zu args, received %zu", arity, a.size() - 1);
	err(buf);
}

void check(ex a, type ty) {
	// In first-order logic, a function cannot return a function, nor can a variable store one. (That would be higher-order logic.)
	// The code should be written so that neither the top-level callers nor the recursive calls, can ever ask for a function to be
	// returned.
	assert(kind(ty) != kind::Fn);

	// All symbols used in a formula must have specified types by the time this check is run. Otherwise, there would be no way of
	// knowing whether the types they will be given in the future, would have passed the check.
	if (ty == kind::Unknown) err("Unspecified type");

	// Need to handle calls before checking the type of this term, because the type of a call is only well-defined if the type of
	// the function is well-defined
	if (a->tag == Fn && a.size() > 1) {
		auto fty = a.getAtom()->ty;
		if (kind(fty) != kind::Fn) err("Called a non-function");
		check(a, fty.size() - 1);
		if (ty != fty[0]) err("Type mismatch");
		for (size_t i = 0; i < a->n; ++i) {
			switch (kind(fty[i])) {
			case kind::Bool:
			case kind::Fn:
				err("Invalid type for function argument");
			}
			check(at(a, i), fty[i]);
		}
		return;
	}

	// The core of the check: Make sure the term is of the required type
	if (type(a) != ty) err("Type mismatch");

	// Further checks can be done depending on operator. For example, arithmetic operators should have matching numeric arguments.
	switch (a->tag) {
	case Add:
	case DivE:
	case DivF:
	case DivT:
	case Mul:
	case RemE:
	case RemF:
	case RemT:
	case Sub:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case All:
	case Exists:
		check(at(a, 0), kind::Bool);
		return;
	case And:
	case Eqv:
	case Not:
	case Or:
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), kind::Bool);
		return;
	case Ceil:
	case Floor:
	case IsInteger:
	case IsRational:
	case Neg:
	case Round:
	case ToInteger:
	case ToRational:
	case ToReal:
	case Trunc:
		check(a, 1);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case DistinctObj:
	case False:
	case Fn:
	case Integer:
	case Rational:
	case True:
		return;
	case Div:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for rational division");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case Eq:
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Bool:
		case kind::Fn:
			err("Invalid type for equality");
		}
		check(at(a, 0), ty);
		check(at(a, 2), ty);
		return;
	case Le:
	case Lt:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for comparison");
		}
		check(at(a, 0), ty);
		check(at(a, 2), ty);
		return;
	case Var:
		// A function would also be an invalid type for a variable, but we already checked for that
		if (kind(ty) == kind::Bool) err("Invalid type for variable");
		return;
	}
	unreachable;
}
