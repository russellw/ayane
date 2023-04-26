#include "main.h"

namespace {
bool constant(term a) {
	switch (a->tag) {
	case DistinctObj:
	case Integer:
	case Rational:
	case True:
		return 1;
	case False:
		// In the superposition calculus, true only shows up as an argument of equality and false never shows up as an argument
		unreachable;
	}
	return 0;
}

bool realConstant(term a) {
	return a->tag == ToReal && tag(a[1]) == Rational;
}
} // namespace

term simplify(const map<term, term>& m, term a) {
	// TODO: other simplifications e.g. x+0, x*1
	auto t = a->tag;
	switch (t) {
	case Add:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x + y;
		if (realConstant(x) && realConstant(y)) return term(ToReal, x[1] + y[1]);
		return term(t, x, y);
	}
	case Ceil:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(ToReal, ceil(x[1]));
		if (constant(x)) return ceil(x);
		return term(t, x);
	}
	case DistinctObj:
	case False:
	case Integer:
	case Rational:
	case True:
		return a;
	case Div:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x / y;
		if (realConstant(x) && realConstant(y)) return term(ToReal, x[1] / y[1]);
		return term(t, x, y);
	}
	case DivE:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divE(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, divE(x[1], y[1]));
		return term(t, x, y);
	}
	case DivF:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divF(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, divF(x[1], y[1]));
		return term(t, x, y);
	}
	case DivT:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divT(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, divT(x[1], y[1]));
		return term(t, x, y);
	}
	case Eq:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (x == y) return True;
		// TODO: optimize?
		if (constant(x) && constant(y)) return False;
		if (realConstant(x) && realConstant(y)) return False;
		return term(t, x, y);
	}
	case Floor:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(ToReal, floor(x[1]));
		if (constant(x)) return floor(x);
		return term(t, x);
	}
	case Fn:
	{
		if (a.size() == 1) {
			// TODO: optimize
			if (m.count(a)) return m.at(a);
			return a;
		}
		vec<term> v(1, a[0]);
		for (size_t i = 1; i != a.size(); ++i) v.push_back(simplify(m, a[i]));
		return term(v);
	}
	case IsInteger:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return mkbool(isInteger(x[1]));
		if (constant(x)) return mkbool(isInteger(x));
		if (type(x) == kind::Integer) return True;
		return term(t, x);
	}
	case IsRational:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x) || constant(x)) return True;
		switch (kind(type(x))) {
		case kind::Integer:
		case kind::Rational:
			return True;
		}
		return term(t, x);
	}
	case Le:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return mkbool(x <= y);
		if (realConstant(x) && realConstant(y)) return mkbool(x[1] <= y[1]);
		return term(t, x, y);
	}
	case Lt:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return mkbool(x < y);
		if (realConstant(x) && realConstant(y)) return mkbool(x[1] < y[1]);
		return term(t, x, y);
	}
	case Mul:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x * y;
		if (realConstant(x) && realConstant(y)) return term(ToReal, x[1] * y[1]);
		return term(t, x, y);
	}
	case Neg:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(ToReal, -x[1]);
		if (constant(x)) return -x;
		return term(t, x);
	}
	case RemE:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remE(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, remE(x[1], y[1]));
		return term(t, x, y);
	}
	case RemF:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remF(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, remF(x[1], y[1]));
		return term(t, x, y);
	}
	case RemT:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remT(x, y);
		if (realConstant(x) && realConstant(y)) return term(ToReal, remT(x[1], y[1]));
		return term(t, x, y);
	}
	case Round:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(ToReal, round(x[1]));
		if (constant(x)) return round(x);
		return term(t, x);
	}
	case Sub:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x - y;
		if (realConstant(x) && realConstant(y)) return term(ToReal, x[1] - y[1]);
		return term(t, x, y);
	}
	case ToInteger:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return toInteger(x[1]);
		if (constant(x)) return toInteger(x);
		if (type(x) == kind::Integer) return x;
		return term(t, x);
	}
	case ToRational:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return x[1];
		if (constant(x)) return toRational(x);
		if (type(x) == kind::Rational) return x;
		return term(t, x);
	}
	case ToReal:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return x;
		if (constant(x)) return term(ToReal, toRational(x));
		if (type(x) == kind::Real) return x;
		return term(t, x);
	}
	case Trunc:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(ToReal, trunc(x[1]));
		if (constant(x)) return trunc(x);
		return term(t, x);
	}
	case Var:
		if (m.count(a)) return m.at(a);
		return a;
	}
	unreachable;
}

// TODO: normalize variables
clause simplify(const map<term, term>& m, const clause& c) {
	vec<term> neg;
	for (auto& a: c.first) {
		auto b = simplify(m, a);
		switch (b->tag) {
		case False:
			return truec;
		case True:
			continue;
		}
		neg.push_back(b);
	}

	vec<term> pos;
	for (auto& a: c.second) {
		auto b = simplify(m, a);
		switch (b->tag) {
		case False:
			continue;
		case True:
			return truec;
		}
		pos.push_back(b);
	}

	for (auto& a: neg)
		if (find(pos.begin(), pos.end(), a) != pos.end()) return truec;

	return make_pair(neg, pos);
}

set<clause> simplify(const map<term, term>& m, const set<clause>& cs) {
	set<clause> r;
	for (auto& c0: cs) {
		auto c = simplify(m, c0);
		if (c == truec) continue;
		r.add(c);
	}
	return r;
}
