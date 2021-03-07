extern sym keywords[];

// SORT
void init_syms();
sym *intern(const char *s, si n);
///

// SORT
inline sym *intern(const char *s) { return intern(s, strlen(s)); }

inline si keyword(const sym *s) {
  // turn a symbol into a keyword number by subtracting the base of the keyword
  // array and dividing by the declared size of a symbol structure (which is
  // efficient as long as that size is a power of 2)

  // it's okay if the symbol is not a keyword; that just means the resulting
  // number will not correspond to any keyword and will not match any case in a
  // switch statement
  size_t i = (const char *)s - (const char *)keywords;
  return i / sizeof(sym);
}
///
