#include "main.h"
#include <new>

#ifdef _WIN32
#include <io.h>
#include <windows.h>

namespace {
LONG WINAPI handler(_EXCEPTION_POINTERS *ExceptionInfo) {
  if (ExceptionInfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_STACK_OVERFLOW) {
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), "Stack overflow\n", 15, 0, 0);
    ExitProcess(1);
  }
  fprintf(stderr, "Exception code %lx\n",
          ExceptionInfo->ExceptionRecord->ExceptionCode);
  stacktrace();
  ExitProcess(1);
}

VOID CALLBACK timeout(PVOID a, BOOLEAN b) { ExitProcess(1); }
} // namespace
#else
#include <unistd.h>
#endif

#define version "3"

enum language {
  dimacs,
  tptp,
  unknown,
};

namespace {
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
  // don't care about a.b/c
  auto s = strrchr(file, '.');
  return s ? s + 1 : "";
}

const char *opt(int argc, const char **argv, int &i) {
  auto s = argv[i];
  auto r = strpbrk(s, "=:");
  if (r)
    return r + 1;
  if (*s == '-' && '0' <= s[2] && s[2] <= '9')
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

language lang = unknown;
vec<const char *> files;

void parse(int argc, const char **argv) {
  for (int i = 0; i != argc; ++i) {
    auto s = argv[i];

    // file
    if (!strcmp(s, "-"))
      s = "stdin";
    if (*s != '-') {
      files.push(s);
      continue;
    }

    // option
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
      lang = dimacs;
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
      lang = tptp;
      break;
    default:
      fprintf(stderr, "%s: Unknown option\n", argv[i]);
      exit(1);
    }
  }
}
} // namespace

int main(int argc, const char **argv) {
  std::set_new_handler([]() {
    perror("new");
    exit(1);
  });
#ifdef _WIN32
  AddVectoredExceptionHandler(0, handler);
#endif

  parse(argc - 1, argv + 1);
  if (!files.n) {
    if (isatty(0)) {
      help();
      return 0;
    }
    files.push("stdin");
  }

  for (auto file : files)
    puts(file);
  return 0;
}
