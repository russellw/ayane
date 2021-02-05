#ifdef _MSC_VER
#define noret __declspec(noreturn) void
#else
#define noret void
#endif

typedef intptr_t w;

#ifdef DEBUG

void stacktrace();
bool assertfail(const char *file, w line, const char *s);
#define assert(a) (a) || assertfail(__FILE__, __LINE__, #a)
#define unreachable assert(0)
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
#define unreachable __assume(0)
#else
#define assert(a)
#define unreachable __builtin_unreachable()
#endif
#define debug(a)

#endif

extern char buf[20000];

// SORT
const char *basename(const char *file);
size_t fnv(const void *p, w n);
void quote(char q, const char *s);
///

// SORT
inline void fpr(FILE *f, char c) { fputc(c, f); }
inline void fpr(FILE *f, const char *s) { fputs(s, f); }
inline void fpr(FILE *f, double a) { fprintf(f, "%f", a); }
inline void fpr(FILE *f, int32_t a) { fprintf(f, "%" PRIi32, a); }
inline void fpr(FILE *f, int64_t a) { fprintf(f, "%" PRIi64, a); }
inline void fpr(FILE *f, uint32_t a) { fprintf(f, "%" PRIu32, a); }
inline void fpr(FILE *f, uint64_t a) { fprintf(f, "%" PRIu64, a); }
inline void fpr(FILE *f, void *p) { fprintf(f, "%p", p); }
inline bool isdigit1(char c) { return '0' <= c && c <= '9'; }
inline bool isspace1(char c) { return c <= ' ' && c; }
///
