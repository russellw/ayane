#ifdef _MSC_VER
#define noret __declspec(noreturn) void
#else
#define noret __attribute__((__noreturn__)) void
#endif

typedef intptr_t si;

#ifdef DEBUG

void stackTrace();
bool assertFail(const char *file, si line, const char *s);
#define assert(a) (a) || assertFail(__FILE__, __LINE__, #a)
#define unreachable assert(0)
#define debug(a)                                                               \
  fprintf(stderr, "%s:%d: %s: %zx\n", __FILE__, __LINE__, #a, (si)a)

#else

#define stackTrace()
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
inline bool isDigit(char c) { return '0' <= c && c <= '9'; }

inline bool isLower(char c) { return 'a' <= c && c <= 'z'; }

inline bool isPow2(si n) {
  // doesn't work for 0
  assert(n);
  return !(n & n - 1);
}

inline bool isSpace(char c) { return c <= ' ' && c; }

inline bool isUpper(char c) { return 'A' <= c && c <= 'Z'; }
///

inline bool isAlpha(char c) { return isLower(c) || isUpper(c); }

// SORT
const char *basename(const char *file);
noret err(const char *msg);
size_t fnv(const void *p, si n);
void *mmalloc(si n);
void quote(char q, const char *s);
void *xcalloc(si n, si size);
void *xmalloc(si n);
void *xrealloc(void *p, si n);
///
