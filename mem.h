// allocation with error checking
void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t size);
void *xrealloc(void *p, size_t n);

// monotonic allocation
void *mmalloc(int n);
