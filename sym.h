struct sym {
  uint16_t n;
  char s[0x20 - sizeof(uint16_t)];
};

extern sym keywords[];

inline size_t keyword(sym *S) {
  size_t i = (char *)S - (char *)keywords;
  return i / sizeof(sym);
}

sym *intern(const char *s, size_t n);

inline sym *intern(const char *s) { return intern(s, strlen(s)); }

inline void fpr(FILE *F, sym *S) { fwrite(S->s, 1, S->n, F); }
