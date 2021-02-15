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
  fprintf(stderr, "%s:%d: %s: %zx\n", __FILE__, __LINE__, #a, (w)a)

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
inline bool isdigit1(char c) { return '0' <= c && c <= '9'; }

inline bool islower1(char c) { return 'a' <= c && c <= 'z'; }

inline bool ispow2(w n) {
  // doesn't work for 0
  assert(n);
  return !(n & n - 1);
}

inline bool isspace1(char c) { return c <= ' ' && c; }
///

// SORT
const char *basename(const char *file);
noret err(const char *msg);
size_t fnv(const void *p, w n);
void *mmalloc(w n);
void quote(char q, const char *s);
void *xcalloc(w n, w size);
void *xmalloc(w n);
void *xrealloc(void *p, w n);
///
