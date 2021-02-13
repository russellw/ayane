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
clause *mkclause(w infer, clause *from, clause *from1) {
  auto nn = neg.n;
  auto pn = pos.n;
  auto n = nn + pn;
  neg.n = pos.n = 0;
  if (n > min(sizeof neg.p / sizeof *neg.p, (size_t)0xffff)) {
    complete = 0;
    return 0;
  }
  memcpy(neg.p + nn, pos.p, pn * sizeof *pos.p);

  auto i = slot(entries, cap, neg.p, nn, n);
  if (entries[i])
    return entries[i];
  if (++count > cap * 3 / 4) {
    expand();
    i = slot(entries, cap, neg.p, nn, n);
  }

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
  formulas.clear();
}
///
