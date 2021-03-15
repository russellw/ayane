#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

#ifdef DEBUG
#ifdef _WIN32
#include <windows.h>
// windows.h must be first
#include <dbghelp.h>
#endif

void stackTrace() {
#ifdef _WIN32
  auto process = GetCurrentProcess();
  SymInitialize(process, 0, 1);
  static void *stack[1000];
  auto nframes =
      CaptureStackBackTrace(1, sizeof stack / sizeof *stack, stack, 0);
  auto symbol = (SYMBOL_INFO *)buf;
  symbol->MaxNameLen = 1000;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  IMAGEHLP_LINE64 location;
  location.SizeOfStruct = sizeof location;
  for (si i = 0; i != nframes; ++i) {
    auto address = (DWORD64)(stack[i]);
    SymFromAddr(process, address, 0, symbol);
    DWORD displacement;
    if (SymGetLineFromAddr64(process, address, &displacement, &location))
      fprintf(stderr, "%s:%lu: ", location.FileName, location.LineNumber);
    fprintf(stderr, "%s\n", symbol->Name);
  }
#endif
}

bool assertFail(const char *file, si line, const char *s) {
  fprintf(stderr, "%s:%zu: assert failed: %s\n", file, line, s);
  stackTrace();
  exit(1);
  // keep the compiler happy about the use of || in the assert macro
  return 0;
}
#endif

// sized for largest tptp symbols
char buf[20000];

// SORT
const char *basename(const char *file) {
  auto i = strlen(file);
  while (i) {
    if (file[i - 1] == '/')
      return file + 1;
#ifdef _WIN32
    if (file[i - 1] == '\\')
      return file + 1;
#endif
    --i;
  }
  return file;
}

noret err(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  stackTrace();
  exit(1);
}

size_t fnv(const void *p, si n) {
  // fowler-noll-vo-1a is slower than more sophisticated hash algorithms for
  // large chunks of data, but faster for tiny ones, so it still sees use
  auto p1 = (const char *)p;
  size_t h = 2166136261u;
  while (n--) {
    h ^= *p1++;
    h *= 16777619;
  }
  return h;
}

void *mmalloc(si n) {
  // monotonic malloc, for memory that does not need to be freed until the
  // program exits
  n = n + 7 & ~(si)7;
  static char *p;
  static char *e;
  assert(!((si)p & 7));
  assert(!((si)e & 7));
  if (e - p < n) {
    auto chunk = max(n, (si)10000);
    p = (char *)xmalloc(chunk);
    e = p + chunk;
  }
  auto r = p;
#ifdef DEBUG
  memset(r, 0xcc, n);
#endif
  p += n;
  return r;
}

void quote(char q, const char *s) {
  putchar(q);
  while (*s) {
    auto c = *s++;
    if (c == q || c == '\\')
      putchar('\\');
    putchar(c);
  }
  putchar(q);
}

void *xcalloc(si n, si size) {
  auto r = calloc(n, size);
  if (!r) {
    perror("calloc");
    exit(1);
  }
  return r;
}

void *xmalloc(si n) {
  auto r = malloc(n);
  if (!r) {
    perror("malloc");
    exit(1);
  }
#ifdef DEBUG
  memset(r, 0xcc, n);
#endif
  return r;
}

void *xrealloc(void *p, si n) {
  auto r = realloc(p, n);
  if (!r) {
    perror("realloc");
    exit(1);
  }
  return r;
}
///
