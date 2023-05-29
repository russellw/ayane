#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

Parser::Parser(const char* file): file(file) {
	// Read all the input in one go before beginning parsing, to make the parsers simpler and faster. Testing indicates
	// memory-mapped files are not really faster for this case, so the input data is read-write so parsers can scribble over it if
	// that makes their job easier.
	size_t n;
	if (strcmp(file, "stdin") == 0) {
#ifdef _WIN32
		_setmode(0, O_BINARY);
#endif
		// Reading from standard input, we don't know the total size up front, so read in blocks of fixed size until we have
		// everything, increasing the buffer capacity by a factor of 2 when necessary, to avoid spending too much time reallocating
		const size_t block = 1 << 20;
		n = 0;
		size_t cap = 0;
		src = 0;
		for (;;) {
			if (n + block + 2 > cap) {
				auto cap1 = max(n + block + 2, cap * 2);
				src = (char*)realloc(src, cap1);
				cap = cap1;
			}
			auto r = read(0, src + n, block);
			if (r < 0) {
				perror(file);
				exit(errno);
			}
			n += r;
			if (r != block) break;
		}
	} else {
		// Reading from a file, we can find out the total size up front
		auto f = open(file, O_BINARY | O_RDONLY);
		struct stat st;
		if (f < 0 || fstat(f, &st)) {
			perror(file);
			exit(errno);
		}
		n = st.st_size;
		src = (char*)malloc(n + 2);
		if (read(f, src, n) != n) {
			perror(file);
			exit(errno);
		}
		close(f);
	}
	src0 = src;

	// Make sure input is null terminated and ends in a newline, to simplify parser code
	src[n] = '\n';
	src[n + 1] = 0;
}

Parser::~Parser() {
	free(src0);
}

void Parser::err(const char* msg, int code) {
	size_t line = 1;
	for (auto s = src0; s < srck; ++s)
		if (*s == '\n') ++line;
	fprintf(stderr, "%s:%zu: %s\n", file, line, msg);
	exit(code);
}

void Parser::digits() {
	auto s = src;
	while (isdigit((unsigned char)*s)) ++s;
	src = s;
}

void Parser::lexInt(mpz_t z) {
	auto s = src;
	switch (*s) {
	case '+':
		// mpz_init_set_str doesn't like leading '+', so eat it before proceeding
		++s;
		[[fallthrough]];
	case '-':
		++src;
		break;
	}
	digits();

	// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
	auto c = *src;
	*src = 0;

	// At least one digit is required
	if (mpz_init_set_str(z, s, 10)) err("expected number");

	// The following byte might be important, so put it back
	*src = c;
}

Expr* Parser::fn(Type* ty, Str* s) {
	// For languages like TPTP where it's okay to repeat declarations, provided they agree with each other
	if (s->fn) {
		auto a = s->fn;

		// Check consistency of data structures
		assert(a->tag == Tag::fn);
		assert(a->s == s->v);

		// If no type is specified, done
		if (!ty) return a;

		// If it already has the specified type, done
		if (a->ty == ty) return a;

		// If it doesn't already have a type, now it does
		if (!a->ty) {
			a->ty = ty;
			return a;
		}

		// Specified type is not consistent with what we already have
		err("type mismatch");
	}

	// Making a new function is the simple case
	auto a = new (ialloc(sizeof(Fn))) Fn(ty, s->v);
	return s->fn = a;
}

void Parser::check(size_t n, Expr* a) {
	if (a->n == n) return;
	if (a->tag == Tag::call) --n;
	sprintf(buf, "expected %zu args", n);
	err(buf);
}

void Parser::check(Type* ty, Expr* a) {
	// In first-order logic, a function cannot return a function, nor can a variable store one. (That would be higher-order logic.)
	// The code should be written so that neither the top-level callers nor the recursive calls, can ever ask for a function to be
	// returned.
	assert(ty->kind != Kind::fn);

	// This switch should be exhaustive
	switch (a->tag) {
	case Tag::add:
	case Tag::divEuclid:
	case Tag::divFloor:
	case Tag::divTrunc:
	case Tag::mul:
	case Tag::remEuclid:
	case Tag::remFloor:
	case Tag::remTrunc:
	case Tag::sub:
		// Arithmetic of arity 2, type passes straight through
		// TODO: would it be better to specialize to addInt etc?
		check(2, a);
		if (!isNum(ty)) err("invalid type for arithmetic");
		check(ty, at(a, 0));
		check(ty, at(a, 1));
		break;
	case Tag::all:
	case Tag::exists:
		// Quantifier
		// TODO: does SMT-LIB need to check a Boolean was wanted here?
		check(&tbool, at(a, 0));
		break;
	case Tag::and1:
	case Tag::eqv:
	case Tag::or1:
		// Connective
		if (&tbool != ty) err("type mismatch");
		// TODO: foreach?
		for (size_t i = 0; i < a->n; ++i) check(&tbool, at(a, i));
		break;
	case Tag::call:
	{
		// Check consistency of data structures
		assert(a->n > 1);
		auto f = (Fn*)at(a, 0);
		assert(f->tag == Tag::fn);

		// Check for input like a(b) where a is just a constant
		auto fty = f->ty;
		if (fty->kind != Kind::fn) err("called a non-function");

		// Check for input like a(b) followed by a(b, c)
		check(fty->n, a);

		// Check for inappropriate parameter types
		for (size_t i = 1; i < fty->n; ++i) switch (at(fty, i)->kind) {
			case Kind::boolean:
				err("boolean parameters not supported", inappropriateError);
			case Kind::fn:
				err("higher-order functions not supported", inappropriateError);
			default:
				break;
			}

		// And of course, check return type
		if (at(fty, 0) != ty) err("type mismatch");

		// And recur, based on the parameter types
		for (size_t i = 1; i < fty->n; ++i) check(at(fty, i), at(a, i));
		break;
	}
	case Tag::ceil:
	case Tag::floor:
	case Tag::minus:
	case Tag::round:
	case Tag::trunc:
		// Arithmetic of arity 1, type passes straight through
		check(1, a);
		if (!isNum(ty)) err("invalid type for arithmetic");
		check(ty, at(a, 0));
		break;
	case Tag::distinctObj:
	case Tag::false1:
	case Tag::integer:
	case Tag::rat:
	case Tag::real:
	case Tag::true1:
		// Leaf
		assert(!a->n);
		if (type(a) != ty) err("type mismatch");
		break;
	case Tag::div:
		// Arithmetic of arity 2, type passes straight through, but fractions only
		check(2, a);
		switch (ty->kind) {
		case Kind::rat:
		case Kind::real:
			break;
		default:
			err("invalid type for division");
		}
		check(ty, at(a, 0));
		check(ty, at(a, 1));
		break;
	case Tag::eq:
		// Eq is always a special case
		check(2, a);
		if (&tbool != ty) err("type mismatch");
		ty = type(at(a, 0));
		switch (ty->kind) {
		case Kind::boolean:
		case Kind::fn:
			err("invalid type for equality");
		default:
			break;
		}
		check(ty, at(a, 0));
		check(ty, at(a, 1));
		break;
	case Tag::fn:
	{
		assert(!a->n);
		auto a1 = (Fn*)a;
		if (!a1->ty) {
			a1->ty = ty;
			break;
		}
		if (a1->ty != ty) err("type mismatch");
		break;
	}
	case Tag::isInt:
	case Tag::isRat:
	case Tag::toInt:
	case Tag::toRat:
	case Tag::toReal:
		// Type converter of arity 1
		check(1, a);
		if (type(a) != ty) err("type mismatch");

		// That means the argument may have a different type
		ty = type(at(a, 0));

		if (!isNum(ty)) err("invalid type for arithmetic");
		check(ty, at(a, 0));
		break;
	case Tag::lt:
		// Type converter of arity 2
		check(2, a);
		if (type(a) != ty) err("type mismatch");

		// That means the argument may have a different type
		ty = type(at(a, 0));

		if (!isNum(ty)) err("invalid type for comparison");
		check(ty, at(a, 0));
		check(ty, at(a, 1));
		break;
	case Tag::not1:
		// Connective of arity 1
		check(1, a);
		if (&tbool != ty) err("type mismatch");
		check(&tbool, at(a, 0));
		break;
	case Tag::var:
		assert(!a->n);

		// Parsers need to make sure variables have leaf types, to preserve validity of data structures, so we only need to check
		// here for Boolean variables
		if (ty == &tbool) err("boolean variables not supported", inappropriateError);

		if (((Var*)a)->ty != ty) err("type mismatch");
		break;
	}
}
