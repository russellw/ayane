void init_syms();
sym *intern(const char *s, si n);
inline sym *intern(const char *s) { return intern(s, strlen(s)); }
