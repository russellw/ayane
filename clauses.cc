#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *infernames[] = {
    0,
#define _(s) #s,
#include "infer.h"
#undef _
};

// SORT
ary<w> neg, pos;
clause *conjecture;
unordered_map<const clause *, const char *> clausefiles;
unordered_map<const clause *, const char *> clausenames;
///

namespace {
w cap = 0x1000;
w count;
clause **entries = (clause **)xcalloc(cap, sizeof(clause *));

bool eq(const clause *c, const w *p, w nn, w n) {
  if (c->nn != nn)
    return 0;
  if (c->n != n)
    return 0;
  return !memcmp(c->v, p, n * sizeof *p);
}

w slot(clause **entries, w cap, const w *p, w nn, w n) {
  auto mask = cap - 1;
  auto i = (fnv(p, n * sizeof *p) ^ nn) & mask;
  while (entries[i] && !eq(entries[i], p, nn, n))
    i = (i + 1) & mask;
  return i;
}

void expand() {
  assert(ispow2(cap));
  auto cap1 = cap * 2;
  auto entries1 = (clause **)xcalloc(cap1, sizeof *entries);
  for (w i = 0; i != cap; ++i) {
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

clause *formula(w infer, w a, clause *from) {
  auto r = (clause *)formulas.alloc(offsetof(clause, v) + sizeof(w));
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

clause *mkclause(w infer, clause *from, clause *from1) {
  // remove redundancy
  neg.erase(remove(neg.begin(), neg.end(), basic(b_true)), neg.end());
  pos.erase(remove(pos.begin(), pos.end(), basic(b_false)), pos.end());

  // check for tautology
  for (auto a : neg)
    if (a == basic(b_false))
      return 0;
  for (auto a : pos)
    if (a == basic(b_true))
      return 0;
  for (auto a : neg)
    for (auto b : pos)
      if (a == b)
        return 0;

  // sort the literals. it's not that the order is meaningful, but that sorting
  // them into canonical order (even if that order is different in each run due
  // to address space layout randomization) makes it possible to detect
  // duplicate clauses that vary only by permutation of literals
  sort(neg.begin(), neg.end());
  sort(pos.begin(), pos.end());

  // gather literals
  auto nn = neg.n;
  auto pn = pos.n;
  auto n = nn + pn;
  neg.n = pos.n = 0;
  if (n > min(sizeof neg.p / sizeof *neg.p, (size_t)0xffff)) {
    // if the number of literals would exceed 16-bit count, discard the clause.
    // in principle this breaks completeness, though in practice it is unlikely
    // such a clause would contribute anything useful to the proof search anyway
    complete = 0;
    return 0;
  }
  memcpy(neg.p + nn, pos.p, pn * sizeof *pos.p);

  // check for an identical existing clause. unlike some other datatypes, we
  // return null rather than just returning a pointer to the existing object,
  // because creating a new clause is a significant action; caller may need to
  // know whether to go ahead and add the new clause to various containers
  auto i = slot(entries, cap, neg.p, nn, n);
  if (entries[i])
    return 0;
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, neg.p, nn, n);
  }

  // make new clause
  auto c = entries[i] =
      (clause *)xmalloc(offsetof(clause, v) + n * sizeof *neg.p);
  memset(c, 0, offsetof(clause, v));
  c->infer = infer;
  c->nn = nn;
  c->n = n;
  c->from[0] = from;
  c->from[1] = from1;
  memcpy(c->v, neg.p, n * sizeof *neg.p);
  return c;
}
///
