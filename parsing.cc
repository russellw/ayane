#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

// current file
const char *file;
// beginning of source text
const char *src0;
// current position in source text
const char *src;

// current token
const char *tok0;
int tok;
vec<char> buf;
sym *toksym;

File::File(const char *file)
    : old_file(::file), old_src0(::src0), old_src(::src) {
  char *s = 0;
  w n = 0;
  if (strcmp(file, "stdin")) {
    auto f = open(file, O_BINARY | O_RDONLY);
    struct stat st;
    if (f < 0 || fstat(f, &st)) {
      perror(file);
      exit(1);
    }

    n = st.st_size;
    s = (char *)xmalloc(n + 2);
    if (read(f, s, n) != n) {
      perror("read");
      exit(1);
    }

    close(f);
  } else {
#ifdef _WIN32
    _setmode(0, O_BINARY);
#endif
    w chunk = 1 << 20;
    w cap = 0;
    for (;;) {
      if (n + chunk + 2 > cap) {
        cap = std::max(n + chunk + 2, cap * 2);
        s = (char *)xrealloc(s, cap);
      }

      auto r = read(0, s + n, chunk);
      if (r < 0) {
        perror("read");
        exit(1);
      }
      n += r;
      if (r != chunk)
        break;
    }
  }
  s[n] = 0;
  if (n && s[n - 1] != '\n') {
    s[n] = '\n';
    s[n + 1] = 0;
  }
  ::file = file;
  src0 = src = s;
  tok0 = 0;
}

File::~File() {
  free((void *)src0);

  ::file = old_file;
  ::src0 = old_src0;
  ::src = old_src;
}

const char *szs[] = {
#define _(s) #s,
#include "szs.h"
#undef _
};

bool conjecture;
#ifdef DEBUG
int status;
#endif

#ifdef _MSC_VER
__declspec(noreturn)
#endif
    void err(const char *msg) {
  if (file && src0 && tok0) {
    // line number
    w line = 1;
    for (auto s = src0; s != tok0; ++s)
      if (*s == '\n')
        ++line;

    // beginning of line
    auto s0 = tok0;
    while (!(s0 == src0 || s0[-1] == '\n'))
      --s0;

    // print context
    for (auto s1 = s0; *s1 >= ' '; ++s1)
      fputc(*s1, stderr);
    fputc('\n', stderr);
    for (auto s1 = s0; s1 != tok0; ++s1)
      fputc(*s1 == '\t' ? '\t' : ' ', stderr);
    fprintf(stderr, "^\n");
    fprintf(stderr, "%s:%zu: ", file, line);
  }
  fprintf(stderr, "%s\n", msg);
  exit(1);
}
