template <class T, size_t small = 4> struct vec {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  uint32_t cap = small;
  uint32_t n;
  T *p = v;
  T v[small];

  vec() : n(0) {}

  vec(T a) : n(1) {
    assert(n <= small);
    v[0] = a;
  }

  vec(T a, T b) : n(2) {
    assert(n <= small);
    v[0] = a;
    v[1] = b;
  }

  vec(T a, T b, T c) : n(3) {
    assert(n <= small);
    v[0] = a;
    v[1] = b;
    v[2] = c;
  }

  vec(T a, T b, T c, T d) : n(4) {
    assert(n <= small);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
  }

  // move
  vec(vec &b) : n(b.n) {
    if (b.p == b.v) {
      memcpy(v, b.v, n * sizeof(T));
      return;
    }
    cap = b.cap;
    p = b.p;
    b.p = b.v;
  }

  ~vec() {
    if (p != v)
      free(p);
  }

  // capacity
  void reserve(size_t m) {
    if (m <= cap)
      return;
    cap = std::max(m, (size_t)cap * 2);
    if (p == v) {
      p = (T *)xmalloc(cap * sizeof(T));
      memcpy(p, v, n * sizeof(T));
      return;
    }
    p = (T *)xrealloc(p, cap * sizeof(T));
  }

  void resize(size_t m) {
    reserve(m);
    n = m;
  }

  void resize(size_t m, T a) {
    reserve(m);
    for (auto i = n; i < m; ++i)
      p[i] = a;
    n = m;
  }

  // element access
  T &operator[](size_t i) {
    assert(i < n);
    return p[i];
  }

  const T &operator[](size_t i) const {
    assert(i < n);
    return p[i];
  }

  T &back() {
    assert(n);
    return p[n - 1];
  }

  const T &back() const {
    assert(n);
    return p[n - 1];
  }

  // iterators
  iterator begin() { return p; }
  const_iterator begin() const { return p; }

  iterator end() { return p + n; }
  const_iterator end() const { return p + n; }

  reverse_iterator rbegin() { return reverse_iterator(p + n); }
  const_reverse_iterator rbegin() const { return reverse_iterator(p + n); }

  reverse_iterator rend() { return reverse_iterator(p); }
  const_reverse_iterator rend() const { return const_reverse_iterator(p); }

  // modifiers
  void push(T a) {
    reserve(n + 1);
    p[n++] = a;
  }

  void pop() {
    assert(n);
    --n;
  }

  void insert(const_iterator position, T *first, T *last) {
    assert(p <= position);
    assert(position <= end());
    auto i = position - p;

    assert(first <= last);
    auto m = last - first;

    reserve(n + m);
    memmove(p + i + m, p + i, (n - i) * sizeof(T));
    memcpy(p + i, first, m * sizeof(T));
    n += m;
  }

  void erase(iterator position) { erase(position, position + 1); }

  void erase(iterator first, iterator last) {
    assert(p <= first);
    assert(first <= end());

    assert(p <= last);
    assert(last <= end());

    assert(first <= last);

    memmove(first, last, (end() - last) * sizeof(T));
    n -= last - first;
  }

  bool operator==(const vec &b) {
    if (n != b.n)
      return false;
    return !memcmp(p, b.p, n * sizeof(T));
  }
};

template <class T, size_t limit> struct static_vec {
  typedef T *iterator;
  typedef const T *const_iterator;

  size_t cap = 1;
  size_t n = 1;
  T *p;

  // capacity
  void reserve(size_t m) {
    if (m <= cap)
      return;
    cap = std::max(m, (size_t)cap * 2);
    p = (T *)xrealloc(p, cap * sizeof(T));
  }

  void resize(size_t m) {
    reserve(m);
    n = m;
  }

  void resize(size_t m, T a) {
    reserve(m);
    for (auto i = n; i < m; ++i)
      p[i] = a;
    n = m;
  }

  // element access
  T &operator[](size_t i) {
    assert(i);
    assert(i < n);
    return p[i];
  }

  const T &operator[](size_t i) const {
    assert(i);
    assert(i < n);
    return p[i];
  }

  T &back() {
    assert(n);
    return p[n - 1];
  }

  const T &back() const {
    assert(n);
    return p[n - 1];
  }

  // iterators
  iterator begin() { return p; }
  const_iterator begin() const { return p; }

  iterator end() { return p + n; }
  const_iterator end() const { return p + n; }

  // modifiers
  void push(T a) {
    if (n >= limit)
      err("Too many objects");
    reserve(n + 1);
    p[n++] = a;
  }
};
