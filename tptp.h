// input
struct inappropriate {};
void tptp(const char *file);

// output
void prtype(type t);
void prterm(term a, term parent = term::False);
void prclause(clause *c);
