#include "main.h"
#include <new>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
// The following must be after windows.h
#include <psapi.h>
static VOID CALLBACK timeout(PVOID a, BOOLEAN b) { ExitProcess(1); }
#else
#include <unistd.h>
#endif

#define version "3"

enum {
  l_unknown,

  l_dimacs,
  l_tptp,
};

namespace {
struct LineParser : parser {
  LineParser(const char *file, vec<const char *> &v) : parser(file) {
    auto s = text;
    while (*s) {
      auto t = strchr(s, '\n');
      auto s1 = t + 1;
      if (t != s && t[-1] == '\r')
        --t;
      v.push(intern(s, t - s)->v);
      s = s1;
    }
  }
};

// SORT
vec<const char *> files;
w lang;
///

void help() {
  printf("Usage: ayane [options] [files]\n"
         "\n"
         "General options:\n"
         "-help       Show help\n"
         "-version    Show version\n"
         "\n"
         "\n"
         "Input:\n"
         "-dimacs     DIMACS format\n"
         "-tptp       TPTP format\n"
         "-           Read stdin\n"
         "\n"
         "Resources:\n"
         "-t seconds  Time limit\n");
}

const char *ext(const char *file) {
  // Don't care about a.b/c
  auto s = strrchr(file, '.');
  return s ? s + 1 : "";
}

const char *opt(int argc, const char **argv, int &i) {
  auto s = argv[i];
  auto r = strpbrk(s, "=:");
  if (r)
    return r + 1;
  if (*s == '-' && isdigit1(s[2]))
    return s + 2;
  if (++i == argc) {
    fprintf(stderr, "%s: Expected arg\n", s);
    exit(1);
  }
  return argv[i];
}

double optdouble(int argc, const char **argv, int &i) {
  auto s = opt(argc, argv, i);
  char *t;
  errno = 0;
  auto a = strtod(s, &t);
  if (errno) {
    perror(s);
    exit(1);
  }
  if (t == s) {
    fprintf(stderr, "%s: Expected number\n", s);
    exit(1);
  }
  return a;
}

void parse(int argc, const char **argv) {
  for (int i = 0; i != argc; ++i) {
    auto s = argv[i];

    // File
    if (!strcmp(s, "-"))
      s = "stdin";
    if (*s != '-') {
      if (!strcmp(ext(s), "lst")) {
        vec<const char *> v;
        LineParser p(s, v);
        parse(v.n, v.p);
        continue;
      }
      files.push(s);
      continue;
    }

    // Option
    while (*s == '-')
      ++s;
    switch (keyword(intern(s))) {
    case k_V:
    case k_v:
    case k_version:
      printf("Ayane " version ", %zu-bit "
#ifdef DEBUG
             "debug"
#else
             "release"
#endif
             " build\n",
             sizeof(void *) * 8);
      exit(0);
    case k_dimacs:
      lang = l_dimacs;
      break;
    case k_h:
    case k_help:
      help();
      exit(0);
    case k_t: {
      auto seconds = optdouble(argc, argv, i);
#ifdef _WIN32
      HANDLE timer = 0;
      CreateTimerQueueTimer(&timer, 0, timeout, 0, (DWORD)(seconds * 1000), 0,
                            WT_EXECUTEINTIMERTHREAD);
#else
      alarm(seconds);
#endif
      break;
    }
    case k_tptp:
      lang = l_tptp;
      break;
    default:
      fprintf(stderr, "%s: Unknown option\n", argv[i]);
      exit(1);
    }
  }
}

w language(const char *file) {
  if (lang)
    return lang;
  switch (keyword(intern(ext(file)))) {
  case k_cnf:
    return l_dimacs;
  }
  return l_tptp;
}

#ifdef DEBUG
#ifdef _WIN32
void print(w n, const char *caption) {
  auto s = buf + sizeof buf - 1;
  *s = 0;
  // Use signed integer calculations for left justification to degrade
  // gracefully just in case a number does overflow the allocated width
  int i = 0;
  do {
    // Extract a digit
    *--s = '0' + n % 10;
    n /= 10;

    // Track how many digits we have extracted
    ++i;

    // So that we can punctuate them in groups of 3
    if (i % 3 == 0 && n)
      *--s = ',';
  } while (n);
  int used = buf + sizeof buf - s;
  int spaces = 15 - used;
  for (i = 0; i < spaces; ++i)
    putchar(' ');
  printf("%s  %s\n", s, caption);
}

#define printitem(x) print(pmc.x, #x)

void printmem() {
  PROCESS_MEMORY_COUNTERS_EX pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc,
                       sizeof pmc);
  printitem(PageFaultCount);
  printitem(PeakWorkingSetSize);
  printitem(WorkingSetSize);
  printitem(QuotaPeakPagedPoolUsage);
  printitem(QuotaPagedPoolUsage);
  printitem(QuotaPeakNonPagedPoolUsage);
  printitem(QuotaNonPagedPoolUsage);
  printitem(PagefileUsage);
  printitem(PeakPagefileUsage);
  printitem(PrivateUsage);
}
#endif
#endif
} // namespace

int main(int argc, const char **argv) {
  std::set_new_handler([]() {
    perror("new");
    exit(1);
  });
#ifdef DEBUG
  test();
#endif

  parse(argc - 1, argv + 1);
  if (!files.n) {
    if (isatty(0)) {
      help();
      return 0;
    }
    files.push("stdin");
  }

  for (w i = 0; i != files.n; ++i) {
    auto file = files[i];
#ifdef DEBUG
    auto start = time(0);
#endif
    auto bname = basename(file);
    clear();
    try {
      switch (language(file)) {
      case l_dimacs:
        dimacs(file);
        break;
      case l_tptp:
        tptp(file);
        break;
      default:
        unreachable;
      }
#ifdef DEBUG
#ifdef _WIN32
      putchar('\n');
      printmem();
      putchar('\n');
#endif
      printf("%zu seconds\n", (w)(time(0) - start));
#endif
    } catch (Inappropriate e) {
      printf("%% SZS status Inappropriate for %s\n", bname);
    }
    if (i + 1 < files.n)
      putchar('\n');
  }
  return 0;
}
