template <class T, w small = 4> struct vec {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  uint32_t cap = small;
  uint32_t n;
  T *p = v;
  T v[small];

  // construct/copy/destroy
  vec() : n(0) {}

  explicit vec(T a) : n(1) {
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

  vec(vec &x) : n(x.n) {
    if (x.p == x.v) {
      memcpy(v, x.v, n * sizeof(T));
      return;
    }
    cap = x.cap;
    p = x.p;
    x.p = x.v;
  }

  ~vec() {
    if (p != v)
      free(p);
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

  // capacity
  void reserve(w m) {
    if (m <= cap)
      return;
    cap = std::max(m, (w)cap * 2);
    if (p == v) {
      p = (T *)xmalloc(cap * sizeof(T));
      memcpy(p, v, n * sizeof(T));
      return;
    }
    p = (T *)xrealloc(p, cap * sizeof(T));
  }

  void resize(w m) {
    reserve(m);
    n = m;
  }

  void resize(w m, T a) {
    reserve(m);
    for (auto i = n; i < m; ++i)
      p[i] = a;
    n = m;
  }

  // element access
  T &operator[](w i) {
    assert(0 <= i);
    assert(i < n);
    return p[i];
  }

  const T &operator[](w i) const {
    assert(0 <= i);
    assert(i < n);
    return p[i];
  }

  T &back() {
    assert(n > 0);
    return p[n - 1];
  }

  const T &back() const {
    assert(n > 0);
    return p[n - 1];
  }

  // modifiers
  void push(T a) {
    reserve(n + 1);
    p[n++] = a;
  }

  void pop() {
    assert(n > 0);
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

  // compare
  bool operator==(const vec &x) {
    if (n != x.n)
      return false;
    return !memcmp(p, x.p, n * sizeof(T));
  }
};
