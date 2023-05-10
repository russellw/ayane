#include "main.h"

namespace {
enum {
	k_distinctObj = ntoks,
	k_dollarWord,
	k_eqv,
	k_imp,
	k_impr,
	k_nand,
	k_ne,
	k_nor,
	k_var,
	k_xor,
};

unordered_map<Str*, Expr*> distinctObjs;

Expr* distinctObj(Str* s) {
	auto& a = distinctObjs[s];
	if (a) return a;
	a = new Expr(Tag::distinctObj);
	return a;
}

// If a term does not already have a type, assign it a specified one
void defaultType(Expr* a, Type* ty) {
	// A statement about the return type of a function call, can directly imply the type of the function. This generally does not
	// apply to basic operators; in most cases, they already have a definite type. That is not entirely true of the arithmetic
	// operators, but we don't try to do type inference to figure those out.
	switch (a->tag) {
	case Tag::call:
	{
		auto f = (Fn*)at(a, 0);
		assert(f->tag == Tag::fn);
		// TODO: might be some blank types in the arguments; is that okay, or do we need to error?
		if (!f->ty) f->ty = compType(ty, begin(a) + 1, end(a));
		break;
	}
	case Tag::fn:
	{
		auto a1 = (Fn*)a;
		if (!a1->ty) a1->ty = ty;
		break;
	}
	}
}

// TODO: Which types should use const?
struct Select: unordered_set<const char*> {
	bool all;

	explicit Select(bool all): all(all) {
	}

	size_t count(const char* s) const {
		if (all) return 1;
		return unordered_set<const char*>::count(s);
	}
};

struct Parser1: Parser {
	// SORT
	bool cnfMode;
	const Select& select;
	Vec<pair<Str*, Expr*>> vars;
	///

	// Tokenizer
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			src = s + 1;
			goto loop;
		case '!':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_ne;
				return;
			}
			break;
		case '"':
			tok = k_distinctObj;
			quote();
			return;
		case '$':
			src = s + 1;
			word();
			tok = k_dollarWord;
			return;
		case '%':
			src = strchr(s, '\n');
			goto loop;
		case '+':
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			number();
			return;
		case '/':
			switch (s[1]) {
			case '*':
				for (s += 2; !(*s == '*' && s[1] == '/'); ++s)
					if (!*s) err("Unclosed comment");
				src = s + 2;
				goto loop;
			}
			break;
		case '<':
			switch (s[1]) {
			case '=':
				if (s[2] == '>') {
					src = s + 3;
					tok = k_eqv;
					return;
				}
				src = s + 2;
				tok = k_impr;
				return;
			case '~':
				if (s[2] == '>') {
					src = s + 3;
					tok = k_xor;
					return;
				}
				err("Expected '>'");
			}
			break;
		case '=':
			switch (s[1]) {
			case '>':
				src = s + 2;
				tok = k_imp;
				return;
			}
			break;
		case '\'':
			tok = k_id;
			quote();
			return;
		case '_':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			word();
			tok = k_var;
			return;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			word();
			return;
		case '~':
			switch (s[1]) {
			case '&':
				src = s + 2;
				tok = k_nand;
				return;
			case '|':
				src = s + 2;
				tok = k_nor;
				return;
			}
			break;
		case 0:
			tok = 0;
			return;
		}
		src = s + 1;
		tok = *s;
	}

	bool eat(int k) {
		if (tok == k) {
			lex();
			return 1;
		}
		return 0;
	}

	void expect(char k) {
		if (eat(k)) return;
		sprintf(buf, "Expected '%c'", k);
		err(buf);
	}

	// Types
	Type* atomicType() {
		auto k = tok;
		auto s = str;
		lex();
		switch (k) {
		case '(':
		{
			auto ty = atomicType();
			expect(')');
			return ty;
		}
		case k_dollarWord:
			switch (keyword(s)) {
			case s_i:
				return &tindividual;
			case s_int:
				return &tinteger;
			case s_o:
				return &tbool;
			case s_rat:
				return &trational;
			case s_real:
				return &treal;
			}
			break;
		case k_id:
			return typeName(s);
		}
		err("Expected type");
	}

	Type* topLevelType() {
		if (eat('(')) {
			// TODO: does vec(n) mean n or cap?
			Vec<Type*> v(1);
			do v.add(atomicType());
			while (eat('*'));
			expect(')');
			expect('>');
			v[0] = atomicType();
			return compType(v);
		}
		auto ty = atomicType();
		if (eat('>')) return compType(atomicType(), ty);
		return ty;
	}

	// Expressions
	void args(Vec<Expr*>& v) {
		expect('(');
		do v.add(atomicTerm());
		while (eat(','));
		expect(')');
	}

	Expr* definedFunctor(Tag tag) {
		Vec<Expr*> v;
		args(v);
		return comp(tag, v);
	}

	Expr* atomicTerm() {
		auto k = tok;
		auto s = str;
		auto num1 = num;
		lex();
		switch (k) {
		case k_distinctObj:
			return distinctObj(s);
		case k_dollarWord:
		{
			switch (keyword(s)) {
			case s_ceiling:
				return definedFunctor(Tag::ceil);
			case s_difference:
				return definedFunctor(Tag::sub);
			case s_distinct:
			{
				Vec<Expr*> v;
				args(v);
				for (auto& a: v) defaultType(a, &tindividual);
				Vec<Expr*> inequalities;
				for (auto i = v.begin(), e = v.end(); i < e; ++i)
					for (auto j = v.begin(); j != i; ++j) inequalities.add(comp(Tag::not1, comp(Tag::eq, *i, *j)));
				return comp(Tag::and1, inequalities);
			}
			case s_false:
				return bools;
			case s_floor:
				return definedFunctor(Tag::floor);
			case s_greater:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::lt, b, a);
			}
			case s_greatereq:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, b, a), comp(Tag::lt, b, a));
			}
			case s_is_int:
				return definedFunctor(Tag::isInteger);
			case s_is_rat:
				return definedFunctor(Tag::isRational);
			case s_less:
				return definedFunctor(Tag::lt);
			case s_lesseq:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, a, b), comp(Tag::lt, a, b));
			}
			case s_product:
				return definedFunctor(Tag::mul);
			case s_quotient:
				return definedFunctor(Tag::div);
			case s_quotient_e:
				return definedFunctor(Tag::divEuclid);
			case s_quotient_f:
				return definedFunctor(Tag::divFloor);
			case s_quotient_t:
				return definedFunctor(Tag::divTrunc);
			case s_remainder_e:
				return definedFunctor(Tag::remEuclid);
			case s_remainder_f:
				return definedFunctor(Tag::remFloor);
			case s_remainder_t:
				return definedFunctor(Tag::remTrunc);
			case s_round:
				return definedFunctor(Tag::round);
			case s_sum:
				return definedFunctor(Tag::add);
			case s_to_int:
				return definedFunctor(Tag::toInteger);
			case s_to_rat:
				return definedFunctor(Tag::toRational);
			case s_to_real:
				return definedFunctor(Tag::toReal);
			case s_true:
				return bools + 1;
			case s_truncate:
				return definedFunctor(Tag::trunc);
			case s_uminus:
				return definedFunctor(Tag::minus);
			}
			break;
		}
		case k_id:
		{
			auto a = fn(s, 0);

			// Not a function call
			if (tok != '(') return a;

			// Function is being called, so gather the function and arguments
			Vec<Expr*> v(1, a);
			args(v);

			// By the TPTP specification, symbols can be assumed Boolean or individual, if not previously specified otherwise.
			// First-order logic does not allow functions to take Boolean arguments, so the arguments can default to individual. But
			// we cannot yet make any assumption about the function return type. For all we know here, it could still be Boolean.
			// Leave it to the caller, which will know from context whether that is the case.
			for (size_t i = 0; i < v.size(); ++i) defaultType(v[i], &tindividual);

			return comp(Tag::call, v);
		}
		case k_num:
			return num1;
		case k_var:
		{
			for (auto i = vars.rbegin(), e = vars.rend(); i != e; ++i)
				if (i->first == s) return i->second;
			if (!cnfMode) err("Unknown variable");
			auto x = var(vars.size(), &tindividual);
			vars.add(make_pair(s, x));
			return x;
		}
		}
		err("Expected expression");
	}

	Expr* infixUnary() {
		auto a = atomicTerm();
		switch (tok) {
		case '=':
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, &tindividual);
			defaultType(b, &tindividual);
			return comp(Tag::eq, a, b);
		}
		case k_ne:
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, &tindividual);
			defaultType(b, &tindividual);
			return comp(Tag::not1, comp(Tag::eq, a, b));
		}
		}
		defaultType(a, &tbool);
		return a;
	}

	Expr* quant(Tag tag) {
		lex();
		expect('[');
		auto old = vars.size();
		// TODO: check generated code
		Vec<Expr*> v(1, 0);
		do {
			if (tok != k_var) err("Expected variable");
			auto s = str;
			lex();
			Type* ty = &tindividual;
			if (eat(':')) ty = atomicType();
			auto x = var(vars.size(), ty);
			vars.add(make_pair(s, x));
			v.add(x);
		} while (eat(','));
		expect(']');
		expect(':');
		v[0] = unary();
		vars.resize(old);
		return comp(tag, v);
	}

	Expr* unary() {
		switch (tok) {
		case '!':
			return quant(Tag::all);
		case '(':
		{
			lex();
			auto a = logicFormula();
			expect(')');
			return a;
		}
		case '?':
			return quant(Tag::exists);
		case '~':
			lex();
			return comp(Tag::not1, unary());
		}
		return infixUnary();
	}

	Expr* associativeLogicFormula(Tag tag, Expr* a) {
		Vec<Expr*> v(1, a);
		auto k = tok;
		while (eat(k)) v.add(unary());
		return comp(tag, v);
	}

	Expr* logicFormula() {
		auto a = unary();
		switch (tok) {
		case '&':
			return associativeLogicFormula(Tag::and1, a);
		case '|':
			return associativeLogicFormula(Tag::or1, a);
		case k_eqv:
			lex();
			return comp(Tag::eqv, a, unary());
		case k_imp:
			lex();
			return imp(a, unary());
		case k_impr:
			lex();
			return imp(unary(), a);
		case k_nand:
			lex();
			return comp(Tag::not1, comp(Tag::and1, a, unary()));
		case k_nor:
			lex();
			return comp(Tag::not1, comp(Tag::or1, a, unary()));
		case k_xor:
			lex();
			return comp(Tag::not1, comp(Tag::eqv, a, unary()));
		}
		return a;
	}

	// Top level
	Str* wordOrDigits() {
		switch (tok) {
		case k_id:
		{
			auto r = str;
			lex();
			return r;
		}
		case k_num:
		{
			auto r = intern(srck, src - srck);
			lex();
			return r;
		}
		}
		err("Expected name");
	}

	void ignore() {
		switch (tok) {
		case '(':
			lex();
			while (!eat(')')) ignore();
			return;
		case 0:
			err("Too many '('s");
		}
		lex();
	}

	Parser1(const char* file, const Select& select): Parser(file), select(select) {
		lex();
		while (tok) {
			// TODO: assert(!vars.n)
			vars.clear();
			auto kw = keyword(wordOrDigits());
			expect('(');
			auto name = wordOrDigits()->v;
			switch (kw) {
			case s_cnf:
			{
				expect(',');

				// Role
				wordOrDigits();
				expect(',');

				// Literals
				cnfMode = 1;
				auto a = quantify(logicFormula());

				// Select
				if (select.count(name)) cnf(a);
				break;
			}
			case s_fof:
			case s_tff:
			{
				expect(',');

				// Role
				auto role = keyword(wordOrDigits());
				expect(',');

				// Type
				if (role == s_type) {
					size_t parens = 0;
					while (eat('(')) ++parens;

					auto s = wordOrDigits();
					expect(':');
					if (tok == k_dollarWord && str == keywords + s_tType) {
						// The symbol will be used as the name of a type. No particular action is required at this point, so accept
						// this and move on.
						lex();
					} else {
						// The symbol is the name of a function with the specified type. Make sure it has this type.
						fn(s, topLevelType());
					}

					while (parens--) expect(')');
					break;
				}

				// Formula
				cnfMode = 0;
				auto a = logicFormula();
				assert(vars.empty());
				check(a, &tbool);

				// Select
				if (!select.count(name)) break;

				// Conjecture
				if (role == s_conjecture) {
					// If multiple conjectures occur in a problem, there are two possible interpretations (conjunction or
					// disjunction), and no consensus on which is correct, so rather than risk silently giving a wrong answer,
					// reject the problem as ambiguous and require it to be restated with the conjectures folded into one, using
					// explicit conjunction or disjunction
					static bool conjecture;
					if (conjecture) err("Multiple conjectures not supported");
					a = comp(Tag::not1, a);
					conjecture = 1;
				}

				// Convert to clauses
				cnf(a);
				break;
			}
			case s_include:
			{
				auto dir = getenv("TPTP");
				if (!dir) err("TPTP environment variable not set");

				// File
				snprintf(buf, sizeof buf, "%s/%s", dir, name);
				auto file1 = strdup(buf);

				// Select and read
				if (eat(',')) {
					expect('[');

					Select sel(0);
					do {
						auto selName = wordOrDigits();
						if (select.count(selName->v)) sel.insert(selName->v);
					} while (eat(','));

					expect(']');
					Parser1 parser(file1, sel);
				} else {
					Parser1 parser(file1, select);
				}
				break;
			}
			default:
				err("Unknown language");
			}
			if (tok == ',') do
					ignore();
				while (tok != ')');
			expect(')');
			expect('.');
		}
	}
};
} // namespace

void tptp(const char* file) {
	Parser1 parser(file, Select(1));
}
