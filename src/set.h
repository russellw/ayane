// TODO: document rationale
template <class K, class L, class T, class Cmp> class Set {
	size_t cap = 4;
	size_t qty;
	T** entries = (T**)calloc(cap, sizeof *entries);

	// TODO: rename a to val
	static size_t slot(T** entries, size_t cap, K tag, L a, size_t n) {
		size_t mask = cap - 1;
		auto i = Cmp::hash(tag, a, n) & mask;
		while (entries[i] && !Cmp::eq(tag, a, n, entries[i])) i = (i + 1) & mask;
		return i;
	}
	static size_t slot(T** entries, size_t cap, T* a) {
		size_t mask = cap - 1;
		auto i = Cmp::hash(a) & mask;
		while (entries[i] && !Cmp::eq(a, entries[i])) i = (i + 1) & mask;
		return i;
	}

	void expand() {
		assert(isPow2(cap));
		auto cap1 = cap * 2;
		auto entries1 = (T**)calloc(cap1, sizeof *entries);
		// TODO: check generated code
		for (auto i = entries, e = entries + cap; i < e; ++i) {
			auto a = *i;
			if (a) entries1[slot(entries1, cap1, a)] = a;
		}
		free(entries);
		cap = cap1;
		entries = entries1;
	}

public:
	T* intern(K tag, L a, size_t n) {
		auto i = slot(entries, cap, tag, a, n);

		// If we have seen this before, return the existing object
		if (entries[i]) {
			// TODO: cache result in local?
			clear(a);
			return entries[i];
		}

		// Expand the hash table if necessary
		if (++qty > cap * 3 / 4) {
			expand();
			i = slot(entries, cap, tag, a, n);
			assert(!entries[i]);
		}

		// Make a new object and add to hash table
		return entries[i] = Cmp::make(tag, a, n);
	}
};