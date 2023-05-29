#include "main.h"

namespace {
const size_t many = 50;

// How many clauses a formula will expand into, for the purpose of deciding when subformulas need to be renamed. The answer could
// exceed the range of a fixed-size integer, but then we don't actually need the number, we only need to know whether it went over
// the threshold.
size_t nclauses(bool pol, Expr* a);

size_t nclausesMul(bool pol, Expr* a) {
	size_t n = 1;
	for (size_t i = 0; i < a->n; ++i) {
		n *= nclauses(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t nclausesAdd(bool pol, Expr* a) {
	size_t n = 0;
	for (size_t i = 0; i < a->n; ++i) {
		n += nclauses(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t nclauses(bool pol, Expr* a) {
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
		return nclauses(pol, at(a, 0));
	case Tag::and1:
		return pol ? nclausesAdd(pol, a) : nclausesMul(pol, a);
	case Tag::eqv:
	{
		auto x = at(a, 0);
		auto y = at(a, 1);

		// Recur twice into each argument. This would cause a problem of exponential blowup in the time taken to calculate the
		// number of clauses that would be generated by nested equivalences. We solve this problem by returning early if the number
		// is becoming large.
		auto n = nclauses(0, x) * nclauses(pol, y);
		if (n >= many) return many;
		n += nclauses(1, x) * nclauses(pol ^ 1, y);
		return min(n, many);
	}
	case Tag::not1:
		return nclauses(!pol, at(a, 0));
	case Tag::or1:
		return pol ? nclausesMul(pol, a) : nclausesAdd(pol, a);
	default:
		return 1;
	}
}

// The function to calculate the number of clauses generated by a formula in a positive or negative context, returns a
// mathematically defined result (up to a ceiling). However, when it comes to actually trying to rename formulas, we may be dealing
// with both positive and negative contexts. In particular, this may happen in nested equivalences, where the total number of
// formulas cannot be calculated without the full context, but would in any case be unreasonably large, so it is neither feasible
// nor necessary to calculate the number. What we actually need to do is make a heuristic decision. To that end, if we have a
// context that is both positive and negative, we add the two values for the number of clauses; this doesn't have a clear
// mathematical justification, but seems as reasonable as anything else, and simple enough that there are hopefully few ways it can
// go wrong.
size_t nclausesApprox(int pol, Expr* a) {
	size_t n = 0;
	if (pol >= 0) n += nclauses(1, a);
	if (pol <= 0) n += nclauses(0, a);
	return n;
}

// Skolem functions replace existentially quantified variables, also formulas that are renamed to avoid exponential expansion
Type* compType(Type* rty, Vec<Expr*>& args) {
	assert(args.n);
	Vec<Type*> v(1 + args.n, rty);
	for (size_t i = 0; i < args.n; ++i) v[i + 1] = type(args[i]);
	return compType(v);
}

Expr* skolem(Type* rty, Vec<Expr*>& args) {
	if (!args.n) return new (ialloc(sizeof(Fn))) Fn(rty, 0);
	Vec<Expr*> v(1 + args.n, new (ialloc(sizeof(Fn))) Fn(compType(rty, args), 0));
	memcpy(v.data + 1, args.data, args.n * sizeof(void*));
	return comp(Tag::call, v);
}

// SORT
Vec<Expr*> defs;
size_t vars = 0;
///

// Rename formulas to avoid exponential expansion. It's tricky to do this while in the middle of doing other things, easier to be
// sure of the logic if it's done as a separate pass first.
Expr* rename(int pol, Expr* a) {
	// TODO: should this be static?
	Vec<Expr*> vars;
	freeVars(a, vars);
	auto b = skolem(type(a), vars);
	if (pol < 0)
		// If this formula is only being used with negative polarity, the new name only needs to be implied by the original formula
		a = imp(a, b);
	else if (pol > 0)
		// If this formula is only being used with positive polarity, the new name only needs to imply the original formula
		a = imp(b, a);
	else
		// In the general case, full equivalence is needed; the new name implies and is implied by the original formula
		a = comp(Tag::and1, imp(b, a), imp(a, b));
	defs.add(quantify(a));
	return b;
}

// Maybe rename some of the arguments to an OR-over-AND (taking polarity into account), where the number of clauses generated would
// be the product of the arguments
void maybeRenameAnds(int pol, Vec<Expr*>& v) {
	// Sorting the arguments doesn't change the meaning of the formula, because AND and OR are commutative. The effect is that if
	// only some of them are to be renamed, we will leave the simple ones alone and end up renaming the complicated ones, which is
	// probably what we want.
	sort(v.begin(), v.end(), [=](Expr* a, Expr* b) { return nclausesApprox(pol, a) < nclausesApprox(pol, b); });
	size_t n = 1;
	for (size_t i = 0; i < v.n; ++i) {
		auto o = nclausesApprox(pol, v[i]);
		if (n * o < many) n *= o;
		else
			v[i] = rename(pol, v[i]);
	}
}

// Given a formula, and whether it is used for positive polarity, negative or both (i.e. under an equivalence), maybe rename some of
// its subformulas. If a subformula occurs many times (whether under the same formula, or different ones), it is considered in
// isolation each time, so that each occurrence could end up with a different name. In principle, it would be more efficient to
// rename on a global basis, but in practice, nontrivial subformulas are rarely duplicated (e.g. less than 1% of the nontrivial
// formulas in the TPTP), so this is probably not worth doing.
Expr* maybeRename(int pol, Expr* a) {
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
	{
		Vec<Expr*> v(a->n, maybeRename(pol, at(a, 0)));
		memcpy(v.data + 1, begin(a) + 1, (a->n - 1) * sizeof(void*));
		return comp(a->tag, v);
	}
	case Tag::and1:
	{
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = maybeRename(pol, at(a, i));

		// NOT-AND yields OR, so mirror the OR case
		if (pol <= 0) maybeRenameAnds(pol, v);
		return comp(a->tag, v);
	}
	case Tag::eqv:
	{
		auto x = maybeRename(0, at(a, 0));
		auto y = maybeRename(0, at(a, 1));
		if (nclausesApprox(0, x) >= many) x = rename(0, x);
		if (nclausesApprox(0, y) >= many) y = rename(0, y);
		return comp(Tag::eqv, x, y);
	}
	case Tag::not1:
		return comp(Tag::not1, maybeRename(-pol, at(a, 0)));
	case Tag::or1:
	{
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = maybeRename(pol, at(a, i));

		// If this formula will be used with positive polarity (including the case where it will be used both ways), we are looking
		// at OR over possible ANDs, which would produce exponential expansion at the distribution stage, so may need to rename some
		// of the arguments
		if (pol >= 0) maybeRenameAnds(pol, v);
		return comp(a->tag, v);
	}
	default:
		return a;
	}
}

Vec<pair<Var*, Expr*>> m;
Expr* nnf(bool pol, Expr* a);

// For-all doesn't need much work to convert. Clauses contain variables with implied for-all. The tricky part is that quantifier
// binds variables to local scope, so the same variable name used in two for-all's corresponds to two different logical variables.
// So we rename each quantified variable to a new variable of the same type.
Expr* all(int pol, Expr* a) {
	// TODO: does it actually need to be new variables, if the parser has in any case not been allowing variable shadowing, because of types?
	auto o = m.n;
	for (size_t i = 1; i < a->n; ++i) {
		auto x = (Var*)at(a, i);
		assert(x->tag == Tag::var);
		auto y = var(((Var*)x)->ty, vars++);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 0));
	m.n = o;
	return a;
}

// Each existentially quantified variable is replaced with a Skolem function whose parameters are all the surrounding universally
// quantified variables
Expr* exists(int pol, Expr* a) {
	// Get the surrounding universally quantified variables that will be arguments to the Skolem functions
	Vec<Expr*> args;
	for (auto& ab: m)
		if (ab.second->tag == Tag::var) args.add(ab.second);

	// Make a replacement for each existentially quantified variable
	auto o = m.n;
	for (size_t i = 1; i < a->n; ++i) {
		auto x = (Var*)at(a, i);
		assert(x->tag == Tag::var);
		auto y = skolem(((Var*)x)->ty, args);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 0));
	m.n = o;
	return a;
}

// Negation normal form consists of several transformations that are as easy to do at the same time: Move NOTs inward to the literal
// layer, flipping things around on the way, while simultaneously resolving quantifiers
Expr* nnf(bool pol, Expr* a) {
	auto tag = a->tag;
	switch (tag) {
	case Tag::all:
		return pol ? all(pol, a) : exists(pol, a);
	case Tag::and1:
	{
		if (!pol) tag = Tag::or1;
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = nnf(pol, at(a, i));
		return comp(tag, v);
	}
	case Tag::distinctObj:
	case Tag::fn:
	case Tag::integer:
	case Tag::rat:
	case Tag::real:
		assert(!a->n);
		break;
	case Tag::eqv:
	{
		// Equivalence is the most difficult operator to deal with
		auto x = at(a, 0);
		auto y = at(a, 1);
		auto x0 = nnf(0, x);
		auto x1 = nnf(1, x);
		auto y0 = nnf(0, y);
		auto y1 = nnf(1, y);
		return pol ? comp(Tag::and1, comp(Tag::or1, x0, y1), comp(Tag::or1, x1, y0))
				   : comp(Tag::and1, comp(Tag::or1, x0, y0), comp(Tag::or1, x1, y1));
	}
	case Tag::exists:
		return pol ? exists(pol, a) : all(pol, a);
	case Tag::false1:
		// Boolean constants and operators can be inverted by downward-sinking NOTs
		return bools + !pol;
	case Tag::not1:
		return nnf(!pol, at(a, 0));
	case Tag::or1:
	{
		if (!pol) tag = Tag::and1;
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = nnf(pol, at(a, i));
		return comp(tag, v);
	}
	case Tag::true1:
		return bools + pol;
	case Tag::var:
	{
		// Variables are mapped to new variables or Skolem functions
		Expr* b;
		auto found = get((Var*)a, b, m);
		assert(found);
		return b;
	}
	default:
	{
		assert(a->n);
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = nnf(1, at(a, i));
		a = comp(tag, v);
		break;
	}
	}
	return pol ? a : comp(Tag::not1, a);
}

// Distribute OR down into AND, completing the layering of the operators for CNF. This is the second place where exponential
// expansion would occur, had selected formulas not already been renamed.
Expr* distribute(Expr* a) {
	// TODO: .clang-format AfterCaseLabel
	// TODO: .clang-format AllowShortFunctionsOnASingleLine
	switch (a->tag) {
	case Tag::and1:
	{
		Vec<Expr*> v(a->n);
		for (size_t i = 0; i < a->n; ++i) v[i] = distribute(at(a, i));
		return comp(Tag::and1, v);
	}
	case Tag::or1:
	{
		// Arguments can be taken without loss of generality as ANDs
		vector<vector<Expr*>> ands;
		for (size_t i = 0; i < a->n; ++i) {
			// Recur
			auto b = distribute(at(a, i));

			// And make a flat layer of ANDs
			vector<Expr*> v;
			flatten(Tag::and1, b, v);
			ands.push_back(v);
		}

		// OR distributes over AND by Cartesian product
		auto p = cartProduct(ands);
		Vec<Expr*> v(p.size());
		for (size_t i = 0; i < v.n; ++i) v[i] = comp(Tag::or1, p[i]);
		return comp(Tag::and1, v);
	}
	default:
		return a;
	}
}

// Convert a suitably rearranged formula to actual clauses
void literals(Expr* a) {
	switch (a->tag) {
	case Tag::all:
	case Tag::and1:
	case Tag::eqv:
	case Tag::exists:
		unreachable;
	case Tag::not1:
		neg.add(at(a, 0));
		break;
	case Tag::or1:
		for (size_t i = 0; i < a->n; ++i) literals(at(a, i));
		break;
	default:
		pos.add(a);
		break;
	}
}

bool hasNum(Expr* a) {
	if (isNum(type(a))) return 1;
	for (size_t i = 0; i < a->n; ++i)
		if (hasNum(at(a, i))) return 1;
	return 0;
}

bool hasNum(Vec<Expr*>& v) {
	for (auto a: v)
		if (hasNum(a)) return 1;
	return 0;
}

void clauses(Expr* a) {
	if (a->tag == Tag::and1) {
		for (size_t i = 0; i < a->n; ++i) clauses(at(a, i));
		return;
	}

	neg.n = pos.n = 0;
	literals(a);

	// First-order logic is not complete on arithmetic. The conservative approach to this is that if any clause contains terms of
	// numeric type, we mark the proof search incomplete, so that failure to derive a contradiction, means the result is
	// inconclusive rather than satisfiable.
	// TODO: do something about the possibility of TPTP cnf bypassing this check
	if (result == 1 && (hasNum(neg) || hasNum(pos))) result = -1;

	clause();
}
} // namespace

void cnf(Expr* a) {
	dbgCheck(a);

	// First run the input formula through the full process: Rename subformulas where necessary to avoid exponential expansion, then
	// convert to negation normal form, distribute OR into AND, and convert to clauses
	defs.n = 0;
	a = maybeRename(1, a);

	assert(!m.n);
	vars = 0;
	a = nnf(1, a);
	a = distribute(a);
	clauses(a);

	// Then convert all the definitions created by the renaming process. That process works by bottom-up recursion, which means each
	// renamed subformula is simple, so there is no need to put the definitions through the renaming process again; they just need
	// to go through the rest of the conversion steps.
	for (auto a: defs) {
		dbgCheck(a);

		assert(!m.n);
		vars = 0;
		a = nnf(1, a);
		a = distribute(a);
		clauses(a);
	}
}
