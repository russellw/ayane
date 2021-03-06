template <class T, si cap = 100000> struct ary {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  si n;
  T p[cap];

  // construct/copy/destroy
  ary() {}

  explicit ary(T a) : n(1) {
    assert(n <= cap);
    p[0] = a;
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
  void reserve(si m) {
    if (m > cap)
      err("array overflow");
  }

  void resize(si m) {
    reserve(m);
    n = m;
  }

  // element access
  T &operator[](si i) {
    assert(i < n);
    return p[i];
  }

  const T &operator[](si i) const {
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

  // modifiers
  void push(T a) {
    reserve(n + 1);
    p[n++] = a;
  }

  void insert(const_iterator position, T a) {
    assert(p <= position);
    assert(position <= end());
    auto i = position - p;

    reserve(n + 1);
    memmove(p + i + 1, p + i, (n - i) * sizeof(T));
    p[i] = a;
    ++n;
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
};
