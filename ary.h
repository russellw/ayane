template <class T, w cap = 1000> struct ary {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  uint32_t n;
  T p[cap];

  // iterators
  iterator begin() { return p; }
  const_iterator begin() const { return p; }

  iterator end() { return p + n; }
  const_iterator end() const { return p + n; }

  reverse_iterator rbegin() { return reverse_iterator(p + n); }
  const_reverse_iterator rbegin() const { return reverse_iterator(p + n); }

  reverse_iterator rend() { return reverse_iterator(p); }
  const_reverse_iterator rend() const { return const_reverse_iterator(p); }

  // element access
  T &operator[](w i) {
    assert(i < n);
    return p[i];
  }

  const T &operator[](w i) const {
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
    if (n == cap)
      throw "array overflow";
    p[n++] = a;
  }
};
