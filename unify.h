extern vec<std::pair<w, w>> unified;

bool match(w a, w b);
bool unify(w a, w b);

inline bool unify0(w a, w b) {
  unified.n = 0;
  return unify(a, b);
}

w replace(w a);
