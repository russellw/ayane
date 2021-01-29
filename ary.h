template <class T, w cap> struct ary {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  uint32_t n;
  T v[cap];

  // iterators
  iterator begin() { return v; }
  const_iterator begin() const { return v; }

  iterator end() { return v + n; }
  const_iterator end() const { return v + n; }

  reverse_iterator rbegin() { return reverse_iterator(v + n); }
  const_reverse_iterator rbegin() const { return reverse_iterator(v + n); }

  reverse_iterator rend() { return reverse_iterator(v); }
  const_reverse_iterator rend() const { return const_reverse_iterator(v); }

  // element access
  T &operator[](w i) {
    assert(i < n);
    return v[i];
  }

  const T &operator[](w i) const {
    assert(i < n);
    return v[i];
  }

  T &back() {
    assert(n);
    return v[n - 1];
  }

  const T &back() const {
    assert(n);
    return v[n - 1];
  }

  // modifiers
  void push(T a) {
    if (n == cap)
      throw "Array overflow";
    v[n++] = a;
  }

  void pop() {
    assert(n);
    --n;
  }
};
