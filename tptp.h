// input
struct inappropriate {};
void tptp(const char *file);

// output
void printType(type t);
void print(term a, term parent = term::False);
void printClause(clause *c);
