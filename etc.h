#ifdef _MSC_VER
#define noret __declspec(noreturn) void
#else
#define noret void
#endif

typedef intptr_t w;

// SORT
inline void fpr(FILE *F, char c) { fputc(c, F); }
inline void fpr(FILE *F, const char *s) { fputs(s, F); }
inline void fpr(FILE *F, double a) { fprintf(F, "%f", a); }
inline void fpr(FILE *F, int32_t a) { fprintf(F, "%" PRIi32, a); }
inline void fpr(FILE *F, int64_t a) { fprintf(F, "%" PRIi64, a); }
inline void fpr(FILE *F, uint32_t a) { fprintf(F, "%" PRIu32, a); }
inline void fpr(FILE *F, uint64_t a) { fprintf(F, "%" PRIu64, a); }
inline void fpr(FILE *F, void *p) { fprintf(F, "%p", p); }
// END

#ifdef DEBUG

void stacktrace();
bool assertfail(const char *file, w line, const char *s);
#define assert(a) (a) || assertfail(__FILE__, __LINE__, #a)
#define unreachable assert(false)
#define debug(a)                                                               \
  do {                                                                         \
    fprintf(stderr, "%s:%d: %s: ", __FILE__, __LINE__, #a);                    \
    fpr(stderr, a);                                                            \
    fputc('\n', stderr);                                                       \
  } while (0)

#else

#define stacktrace()
#ifdef _MSC_VER
#define assert(a) __assume(a)
#define unreachable __assume(false)
#else
#define assert(a)
#define unreachable __builtin_unreachable()
#endif
#define debug(a)

#endif

extern char buf[0x10000];

size_t fnv(const void *p, w n);
