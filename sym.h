struct sym {
  // Type named by this symbol
  ty t;

  // Type of function named by this symbol
  ty ft;

  // Number of characters (or UTF-8 bytes, if it's a Unicode string) in this
  // symbol

  // Symbols don't use null terminators, though keywords (which are statically
  // declared symbols) will have their strings padded out with nulls to the
  // declared length of the character array
  uint16_t n;

  // For the keyword system to work, the size of the declared character array
  // must be large enough to hold the longest keyword

  // For the system to work efficiently, the size of the whole structure must be
  // a power of 2

  // When symbols are allocated on the heap, the code doing the allocation is
  // responsible for allocating enough space to hold the corresponding strings
  char v[0x20 - 2 * sizeof(ty) - sizeof(uint16_t)];

  bool eq(const char *s, w m) const {
    if (this->n != m)
      return false;
    return !memcmp(v, s, m);
  }

  static sym *store(const char *s, w n) {
    auto r = (sym *)xmalloc(offsetof(sym, v) + n);
    memset(r, 0, offsetof(sym, v));
    r->n = n;
    memcpy(r->v, s, n);
    return r;
  }

  static sym *process(sym *S) { return S; }
};

extern sym keywords[];

inline size_t keyword(sym *S) {
  // Turn a symbol into a keyword number by subtracting the base of the keyword
  // array and dividing by the declared size of a symbol structure (which is
  // efficient as long as that size is a power of 2)

  // It's okay if the symbol is not a keyword; that just means the resulting
  // number will not correspond to any keyword and will not match any case in a
  // switch statement
  size_t i = (char *)S - (char *)keywords;
  return i / sizeof(sym);
}

sym *intern(const char *s, w n);

inline sym *intern(const char *s) { return intern(s, strlen(s)); }

inline void fpr(FILE *F, sym *S) { fwrite(S->v, 1, S->n, F); }
