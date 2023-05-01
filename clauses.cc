#include "main.h"

const char* ruleNames[] = {
#define _(x) #x,
#include "rules.h"
};

Formula* conjecture;

static size_t cost(Ex* a) {
	size_t n = 1;
	for (size_t i = 1; i < a->n; ++i) n += cost(at(a, i));
	return n;
}

size_t cost(Clause* c) {
	size_t n = 0;
	for (size_t i = 0; i < c->n; ++i) n += cost(c->atoms[i]);
	return n;
}

std::priority_queue<Clause*, vec<Clause*>, CompareClauses> passive;
