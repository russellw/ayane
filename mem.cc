#include "main.h"

void *xmalloc(size_t n) {
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

void *xcalloc(size_t n, size_t size) {
  auto r = calloc(n, size);
  if (!r) {
    perror("calloc");
    exit(1);
  }
  return r;
}

void *xrealloc(void *p, size_t n) {
  auto r = realloc(p, n);
  if (!r) {
    perror("realloc");
    exit(1);
  }
  return r;
}
