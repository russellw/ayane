#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

#ifdef DEBUG
#ifdef _WIN32
#include <windows.h>
// windows.h must be first
#include <dbghelp.h>
#endif

void stacktrace() {
#ifdef _WIN32
  auto process = GetCurrentProcess();
  SymInitialize(process, 0, true);
  static void *stack[1000];
  auto nframes =
      CaptureStackBackTrace(1, sizeof stack / sizeof *stack, stack, 0);
  auto symbol = (SYMBOL_INFO *)buf;
  symbol->MaxNameLen = 1000;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  IMAGEHLP_LINE64 location;
  location.SizeOfStruct = sizeof location;
  for (w i = 0; i != nframes; ++i) {
    auto address = (DWORD64)(stack[i]);
    SymFromAddr(process, address, 0, symbol);
    DWORD displacement;
    if (SymGetLineFromAddr64(process, address, &displacement, &location))
      fprintf(stderr, "%s:%lu: ", location.FileName, location.LineNumber);
    fprintf(stderr, "%s\n", symbol->Name);
  }
#endif
}

bool assertfail(const char *file, w line, const char *s) {
  fprintf(stderr, "%s:%zu: assert failed: %s\n", file, line, s);
  stacktrace();
  exit(1);
  // Keep the compiler happy about the use of || in the assert macro
  return false;
}
#endif

char buf[0x10000];

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

size_t fnv(const void *p, w n) {
  // Fowler-Noll-Vo-1a
  auto p1 = (const char *)p;
  size_t h = 2166136261u;
  while (n--) {
    h ^= *p1++;
    h *= 16777619;
  }
  return h;
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
///
