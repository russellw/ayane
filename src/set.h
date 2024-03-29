// When a template depends on ambient functions, GCC requires the functions to be declared before the template, so this header
// should be included in each source file that uses it, after the relevant function declarations
template <class K, class V, class T, int initCap = 4> struct Set {
	size_t cap = initCap;
	size_t qty;
	T** entries = (T**)calloc(cap, sizeof(void*));

	static size_t slot(T** entries, size_t cap, K tag, V v, size_t n) {
		size_t mask = cap - 1;
		auto i = hash(tag, v, n) & mask;
		while (entries[i] && !eq(tag, v, n, entries[i])) i = (i + 1) & mask;
		return i;
	}
	static size_t slot(T** entries, size_t cap, T* a) {
		size_t mask = cap - 1;
		auto i = hash(a) & mask;
		while (entries[i] && !eq(a, entries[i])) i = (i + 1) & mask;
		return i;
	}

	void expand() {
		auto cap1 = cap * 2;
		auto entries1 = (T**)calloc(cap1, sizeof(void*));
		for (auto i = entries, e = entries + cap; i < e; ++i) {
			auto a = *i;
			if (a) entries1[slot(entries1, cap1, a)] = a;
		}
		free(entries);
		cap = cap1;
		entries = entries1;
	}

	T* intern(K tag, V v, size_t n) {
		auto i = slot(entries, cap, tag, v, n);

		// If we have seen this before, return the existing object
		auto a = entries[i];
		if (a) {
			clear(v);
			return a;
		}

		// Expand the hash table if necessary
		if (++qty > cap * 3 / 4) {
			expand();
			i = slot(entries, cap, tag, v, n);
			assert(!entries[i]);
		}

		// Make a new object and add to hash table
		return entries[i] = make(tag, v, n);
	}
};
