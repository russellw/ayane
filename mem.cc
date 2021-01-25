#include "main.h"

void *xmalloc(w n) {
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

void *xcalloc(w n, w size) {
  auto r = calloc(n, size);
  if (!r) {
    perror("calloc");
    exit(1);
  }
  return r;
}

void *xrealloc(void *p, w n) {
  auto r = realloc(p, n);
  if (!r) {
    perror("realloc");
    exit(1);
  }
  return r;
}
