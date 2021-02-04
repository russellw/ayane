#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

parser::parser(const char *file) : file(file) {
  char *s = 0;
  w n = 0;
  if (strcmp(file, "stdin")) {
    // Read from file
    auto f = open(file, O_BINARY | O_RDONLY);
    struct stat st;
    if (f < 0 || fstat(f, &st)) {
      perror(file);
      exit(1);
    }

    // Check size of file
    n = st.st_size;

    // Allow space for extra newline if required, and null terminator
    s = (char *)xmalloc(n + 2);

    // Read all the data
    if (read(f, s, n) != n) {
      perror("read");
      exit(1);
    }

    close(f);
  } else {
    // Read from stdin
#ifdef _WIN32
    _setmode(0, O_BINARY);
#endif
    w chunk = 1 << 20;
    w cap = 0;

    // stdin doesn't tell us in advance how much data there will be, so keep
    // reading chunks until done
    for (;;) {
      // Expand buffer as necessary, allowing space for extra newline if
      // required, and null terminator
      if (n + chunk + 2 > cap) {
        cap = std::max(n + chunk + 2, cap * 2);
        s = (char *)xrealloc(s, cap);
      }

      // Read another chunk
      auto r = read(0, s + n, chunk);
      if (r < 0) {
        perror("read");
        exit(1);
      }
      n += r;

      // No more data to read
      if (r != chunk)
        break;
    }
  }

  // Newline and null terminator
  s[n] = 0;
  if (n && s[n - 1] != '\n') {
    s[n] = '\n';
    s[n + 1] = 0;
  }

  // Start at the beginning
  textstart = s;
  text = s;
  tokstart = s;
}

parser::~parser() { free((void *)textstart); }

noret parser::err(const char *msg, const char *ts) {
  // Line number
  w line = 1;
  for (auto s = textstart; s != ts; ++s)
    if (*s == '\n')
      ++line;

  // Start of line
  auto lineStart = ts;
  while (!(lineStart == textstart || lineStart[-1] == '\n'))
    --lineStart;

  // Print context
  for (auto s = lineStart; *s >= ' '; ++s)
    fputc(*s, stderr);
  fputc('\n', stderr);

  // Print caret
  for (auto s = lineStart; s != ts; ++s)
    fputc(*s == '\t' ? '\t' : ' ', stderr);
  fprintf(stderr, "^\n");

  // Print message and exit
  fprintf(stderr, "%s:%zu: %s\n", file, line, msg);
  exit(1);
}

noret parser::err(const char *msg) { err(msg, tokstart); }
