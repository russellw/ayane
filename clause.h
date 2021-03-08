enum class infer : char {
  none,
#define X(s) s,
#include "infer.h"
};

extern const char *infernames[];

struct clause {
  // is this a first-order formula rather than an actual clause?
  // a first-order formula is represented like a clause with just one positive
  // literal, but the literal can be any first-order predicate
  bool fof;

  // set if this clause has been subsumed and should be henceforth ignored (or
  // perhaps eventually garbage collected)
  bool subsumed;

  // which inference rule derived it?
  infer inf;

  // number of negative and total literals
  // the literals are laid out in an array, negative then positive
  uint16_t nn, n;
  si np() { return n - nn; }

  // the majority of ways for a clause to be made, result in it deriving from
  // zero or one other clauses, but the majority of clauses will be made by the
  // superposition rule, so derived from two other clauses
  clause *from[2];

  // literals
  w v[0];
};

// SORT
extern ary<w> neg, pos;
extern clause *conjecture;
extern unordered_map<const clause *, const char *> clausefiles;
extern unordered_map<const clause *, const char *> clausenames;
///

// SORT
const char *clausename(const clause *c);
clause *formula(infer inf, w a, clause *from = 0);
void init_clauses();
clause *mkclause(infer inf, clause *from = 0, clause *from1 = 0);
///
