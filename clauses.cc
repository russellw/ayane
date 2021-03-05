#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *infernames[] = {
    0,
#define X(s) #s,
#include "infer.h"
#undef X
};

// SORT
ary<w> neg, pos;
clause *conjecture;
unordered_map<const clause *, const char *> clausefiles;
unordered_map<const clause *, const char *> clausenames;
///

namespace {
size_t cap = 0x1000;
size_t count;
clause **entries = (clause **)xcalloc(cap, sizeof *entries);

bool eq(const clause *c, const w *p, int nn, int n) {
  if (c->nn != nn)
    return 0;
  if (c->n != n)
    return 0;
  return !memcmp(c->v, p, n * sizeof *p);
}

size_t slot(clause **entries, w cap, const w *p, int nn, int n) {
  auto mask = cap - 1;
  auto i = XXH64(p, n * sizeof *p, nn) & mask;
  while (entries[i] && !eq(entries[i], p, nn, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (clause **)xcalloc(cap1, sizeof *entries);
  for (int i = 0; i != cap; ++i) {
    auto c = entries[i];
    if (c)
      entries1[slot(entries1, cap1, c->v, c->nn, c->n)] = c;
  }
  free(entries);
  cap = cap1;
  entries = entries1;
}

pool<> formulas;
} // namespace

// SORT
const char *clausename(const clause *c) {
  auto name = clausenames[c];
  return name ? name : "?";
}

clause *formula(int infer, w a, clause *from) {
  auto r = (clause *)formulas.alloc(offsetof(clause, v) + sizeof a);
  memset(r, 0, offsetof(clause, v));
  r->fof = 1;
  r->infer = infer;
  r->n = 1;
  r->from[0] = from;
  r->v[0] = a;
  return r;
}

void init_clauses() {
  clausefiles.clear();
  clausenames.clear();
  conjecture = 0;
  formulas.init();

  count = 0;
  memset(entries, 0, cap * sizeof *entries);
}

clause *mkclause(int infer, clause *from, clause *from1) {
  // remove redundancy
  neg.erase(remove(neg.p, neg.end(), basic(b_true)), neg.end());
  pos.erase(remove(pos.p, pos.end(), basic(b_false)), pos.end());

  // sort the literals. it's not that the order is meaningful, but that sorting
  // them into canonical order (even if that order is different in each run due
  // to address space layout randomization) makes it possible to detect
  // duplicate clauses that vary only by permutation of literals
  sort(neg.p, neg.end());
  sort(pos.p, pos.end());

  // remove duplicate literals (must be done after sorting, because in order to
  // run in linear time, std::unique assumes duplicates will be consecutive)
  neg.erase(unique(neg.p, neg.end()), neg.end());
  pos.erase(unique(pos.p, pos.end()), pos.end());

  // gather literals
  auto p = neg.p;
  auto nn = neg.n;
  auto np = pos.n;
  auto n = nn + np;
  // postcondition: the negative and positive input arrays are cleared. this
  // means callers can assume they are clear to start with, and avoids
  // duplicating this line of code at many call sites
  neg.n = pos.n = 0;
  if (n > min(sizeof neg.p / sizeof *p, (size_t)0xffff)) {
    // if the number of literals would exceed 16-bit count, discard the clause.
    // in principle this breaks completeness, though in practice it is unlikely
    // such a clause would contribute anything useful to the proof search anyway
    complete = 0;
    return 0;
  }
  memcpy(p + nn, pos.p, np * sizeof *p);

  // check for tautology (could be done earlier, but as the only step that takes
  // quadratic time, it is here being done after the efforts to reduce the
  // number of literals)
  for (auto i = p, e = p + nn; i != e; ++i)
    if (*i == basic(b_false))
      return 0;
  for (auto i = p + nn, e = p + n; i != e; ++i)
    if (*i == basic(b_true))
      return 0;
  for (auto i = p, e = p + nn; i != e; ++i) {
    auto a = *i;
    for (auto j = p + nn, e = p + n; j != e; ++j)
      if (a == *j)
        return 0;
  }

  // check for an identical existing clause. unlike some other datatypes, we
  // return null rather than just returning a pointer to the existing object,
  // because creating a new clause is a significant action; caller may need to
  // know whether to go ahead and add the new clause to various containers
  auto i = slot(entries, cap, p, nn, n);
  if (entries[i])
    return 0;
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, p, nn, n);
  }

  // make new clause
  auto c = entries[i] = (clause *)xmalloc(offsetof(clause, v) + n * sizeof *p);
  memset(c, 0, offsetof(clause, v));
  c->infer = infer;
  c->nn = nn;
  c->n = n;
  c->from[0] = from;
  c->from[1] = from1;
  memcpy(c->v, p, n * sizeof *p);
  return c;
}
///
