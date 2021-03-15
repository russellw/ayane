#include "stdafx.h"
// stdafx.h must be first
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
  si n = 0;
  if (strcmp(file, "stdin")) {
    // read from file
    auto f = open(file, O_BINARY | O_RDONLY);
    struct stat st;
    if (f < 0 || fstat(f, &st)) {
      perror(file);
      exit(1);
    }

    // check size of file
    n = st.st_size;

    // allow space for extra newline if required, and null terminator
    s = (char *)xmalloc(n + 2);

    // read all the data
    if (read(f, s, n) != n) {
      perror("read");
      exit(1);
    }

    close(f);
  } else {
    // read from stdin
#ifdef _WIN32
    _setmode(0, O_BINARY);
#endif
    const si chunk = 1 << 20;
    si cap = 0;

    // stdin doesn't tell us in advance how much data there will be, so keep
    // reading chunks until done
    for (;;) {
      // expand buffer as necessary, allowing space for extra newline if
      // required, and null terminator
      if (n + chunk + 2 > cap) {
        cap = max(n + chunk + 2, cap * 2);
        s = (char *)xrealloc(s, cap);
      }

      // read another chunk
      auto r = read(0, s + n, chunk);
      if (r < 0) {
        perror("read");
        exit(1);
      }
      n += r;

      // no more data to read
      if (r != chunk)
        break;
    }
  }

  // newline and null terminator
  s[n] = 0;
  if (n && s[n - 1] != '\n') {
    s[n] = '\n';
    s[n + 1] = 0;
  }

  // start at the beginning
  textStart = s;
  text = s;
  tokStart = s;
}

parser::~parser() { free((void *)textStart); }

noret parser::err(const char *msg, const char *ts) {
  // line number
  si line = 1;
  for (auto s = textStart; s != ts; ++s)
    if (*s == '\n')
      ++line;

  // start of line
  auto lineStart = ts;
  while (!(lineStart == textStart || lineStart[-1] == '\n'))
    --lineStart;

  // print context
  for (auto s = lineStart; *s >= ' '; ++s)
    fputc(*s, stderr);
  fputc('\n', stderr);

  // print caret
  for (auto s = lineStart; s != ts; ++s)
    fputc(*s == '\t' ? '\t' : ' ', stderr);
  fprintf(stderr, "^\n");

  // print message and exit
  fprintf(stderr, "%s:%zu: %s\n", file, line, msg);
  exit(1);
}

noret parser::err(const char *msg) { err(msg, tokStart); }
