#include "main.h"

namespace {
enum {
  o_dollarword = 1,
  o_eqv,
  o_imp,
  o_impr,
  o_nand,
  o_ne,
  o_nor,
  o_var,
  o_word,
  o_xor,
};

char isWord[0x100];

struct selection : std::unordered_set<w> {
  bool all;

  selection(const selection &x) : std::unordered_set<w>(x), all(x.all) {}
  selection(bool all) : all(all) {}

  w count(w a) const {
    if (all)
      return 1;
    return std::unordered_set<w>::count(a);
  }
};

struct TptpParser : Parser {
  selection select;
  vec<std::pair<sym *, w>> bound, free;

  // Tokenizer
  void word() {
    auto s = text;
    while (isWord[*s])
      ++s;
    tokSym = intern(text, s - text);
    text = s;
  }

  void quote() {
    auto s = text;
    auto q = *s++;
    buf.n = 0;
    while (*s != q) {
      if (*s == '\\')
        ++s;
      if (*s < ' ')
        throw "Unclosed quote";
      buf.push(*s++);
    }
    text = s + 1;
    tokSym = intern(buf.p, buf.n);
  }

  void lex() {
  loop:
    auto s = tokStart = text;
    switch (*s) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      text = s + 1;
      goto loop;
    case '%': {
      text = strchr(s, '\n');
#ifdef DEBUG
      std::string line(s, text);
      std::smatch m;
      if (!status &&
          std::regex_match(line, m, std::regex(R"(% Status\s*:\s*(\w+))"))) {
        for (w i = 1; i != n_szs; ++i)
          if (m[1] == szs[i]) {
            status = i;
            break;
          }
        if (!status)
          throw m[1].c_str();
      }
#endif
      goto loop;
    }
    case '/':
      if (s[1] != '*') {
        text = s + 1;
        throw "Expected '*'";
      }
      for (s += 2; !(s[0] == '*' && s[1] == '/'); ++s)
        if (!*s)
          throw "Unclosed comment";
      text = s + 2;
      goto loop;
    case '$':
      text = s + 1;
      tok = o_dollarword;
      word();
      return;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
      tok = o_word;
      word();
      return;
    case '\'':
      tok = o_word;
      quote();
      return;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      tok = o_var;
      word();
      return;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      do
        ++s;
      while (isDigit(*s));
      tok = o_int;
      switch (*s) {
      case '.':
        do
          ++s;
        while (isDigit(*s));
        tok = o_real;
        break;
      case '/':
        do
          ++s;
        while (isDigit(*s));
        tok = o_rat;
        break;
      }
      text = s;
      return;
    case '+':
    case '-':
      if (isDigit(s[1])) {
        tok = o_term;
        parse_number();
        return;
      }
      break;
    case '"':
      tok = o_term;
      quote();
      tok_term = atom(t_distinct_object, sizeof(type *) + sizeof(symbol *));
      tok_term->name = tokSym;
      return;
    case '!':
      switch (s[1]) {
      case '=':
        text = s + 2;
        tok = o_ne;
        return;
      }
      break;
    case '=':
      switch (s[1]) {
      case '>':
        text = s + 2;
        tok = o_imp;
        return;
      }
      break;
    case '<':
      switch (s[1]) {
      case '=':
        if (s[2] == '>') {
          text = s + 3;
          tok = o_eqv;
          return;
        }
        text = s + 2;
        tok = o_impr;
        return;
      case '~':
        if (s[2] == '>') {
          text = s + 3;
          tok = o_xor;
          return;
        }
        tokStart = s + 2;
        throw "Expected '>'";
      }
      break;
    case '~':
      switch (s[1]) {
      case '&':
        text = s + 2;
        tok = o_nand;
        return;
      case '|':
        text = s + 2;
        tok = o_nor;
        return;
      }
      break;
    case 0:
      tok = 0;
      return;
    }
    text = s + 1;
    tok = *s;
  }

  // parser

  bool eat(char o) {
    if (tok == o) {
      lex();
      return true;
    }
    return false;
  }

  void expect(char o) {
    if (eat(o))
      return;
    sprintf(buf.v, "Expected '%c'", o);
    throw buf.v;
  }

  void expect(char o, const char *s) {
    if (eat(o))
      return;
    sprintf(buf.v, "Expected %s", s);
    throw buf.v;
  }

  // types

  ty read_type() {
    switch (tok) {
    case o_dollarword:
      switch (keyword(tokSym)) {
      case k_o:
        lex();
        return t_bool;
      case k_int:
        lex();
        return t_int;
      case k_rat:
        lex();
        return t_rat;
      case k_real:
        lex();
        return t_real;
      case k_i:
        lex();
        return t_individual;
      }
      throw "Unknown word";
    case o_word: {
      auto name = tokSym;
      if (name->val && name->is_term)
        throw "Expected type";
      lex();
      if (name->val)
        return (type *)name->val;
      auto ty = make_type(name);
      name->val = ty;
      name->is_term = false;
      return ty;
    }
    case '(': {
      lex();
      vec<type *> v;
      do
        v.push(read_type());
      while (eat('*'));
      expect(')');
      expect('>');
      return make_type(read_type(), v);
    }
    }
    throw "Expected type";
  }

  void typing() {
    if (eat('(')) {
      typing();
      expect(')');
      return;
    }

    // name
    if (tok != o_word)
      throw "Expected role";
    auto name = tokSym;
    auto s = src1;
    lex();
    expect(':');

    // type: $tType
    if (tok == o_dollarword && tokSym == keywords + k_tType) {
      if (name->val) {
        if (name->is_term) {
          src1 = s;
          throw "already a term";
        }
      } else {
        name->val = make_type(name);
        name->is_term = false;
      }
      lex();
      return;
    }

    // constant: type
    auto ty = read_type();
    if (!name->val) {
      name->val = constant(ty, name);
      name->is_term = true;
    }
    auto a = (term *)name->val;
    if (a->ty_ != ty) {
      src1 = s;
      throw "already has another type";
    }
  }

  // terms

  void args(vec<w> &v) {
    expect('(');
    do
      v.push(atomic_term());
    while (eat(','));
    expect(')');
  }

  void args(vec<w> &v, w arity) {
    auto old = v.n;
    args(v);
    if (v.n - old == arity)
      return;
    sprintf(buf.v, "Expected %zu arguments", arity);
    throw buf.v;
  }

  w defined_functor(w op, w arity) {
    lex();
    vec<w> v(op);
    args(v, arity);
    return term(v);
  }

  w atomic_term() {
    switch (tok) {
    case o_word: {
      auto name = tokSym;
      if (name->val && !name->is_term)
        throw "Expected term";
      lex();

      if (!name->val) {
        name->val = constant(&Ind, name);
        name->is_term = true;
      }

      if (tok != '(')
        return (term *)name->val;

      vec<w> v(...);
      args(v);
      return term(t_call, (term *)name->val, v);
    }
    case o_var: {
      auto name = tokSym;
      if (name->val && !name->is_term)
        throw "Expected term";
      lex();

      if (!name->val) {
        name->val = var(&Ind);
        name->is_term = true;
      }

      return (w)name->val;
    }
    case o_term: {
      auto a = tok_term;
      lex();
      return a;
    }
    case o_dollarword: {
      vec<w> v;
      switch (keyword(tokSym)) {
      case k_false:
        lex();
        return basic(b_false);
      case k_true:
        lex();
        return basic(b_true);
      case k_less:
        return defined_functor(basic(b_lt), 2);
      case k_lesseq:
        lex();
        args(v, 2);
        return term(basic(b_or), term(basic(b_lt), v[0], v[1]),
                    term(basic(b_eq), v[0], v[1]));
      case k_greater:
        lex();
        args(v, 2);
        return term(basic(b_lt), v[1], v[0]);
      case k_greatereq:
        lex();
        args(v, 2);
        return term(basic(b_or), term(basic(b_lt), v[1], v[0]),
                    term(basic(b_eq), v[1], v[0]));
      case k_uminus:
        return defined_functor(basic(b_minus), 1);
      case k_sum:
        return defined_functor(basic(b_add), 2);
      case k_difference:
        return defined_functor(basic(b_sub), 2);
      case k_product:
        return defined_functor(basic(b_mul), 2);
      case k_quotient:
        return defined_functor(basic(b_div), 2);
      case k_quotient_e:
        return defined_functor(basic(b_dive), 2);
      case k_quotient_t:
        return defined_functor(basic(b_divt), 2);
      case k_quotient_f:
        return defined_functor(basic(b_divf), 2);
      case k_remainder_e:
        return defined_functor(basic(b_reme), 2);
      case k_remainder_t:
        return defined_functor(basic(b_remt), 2);
      case k_remainder_f:
        return defined_functor(basic(b_remf), 2);
      case k_floor:
        return defined_functor(basic(b_floor), 1);
      case k_ceiling:
        return defined_functor(basic(b_ceil), 1);
      case k_truncate:
        return defined_functor(basic(b_trunc), 1);
      case k_round:
        return defined_functor(basic(b_round), 1);
      case k_is_int:
        return defined_functor(basic(b_isint), 1);
      case k_is_rat:
        return defined_functor(basic(b_israt), 1);
      case k_to_int:
        return defined_functor(basic(b_toint), 1);
      case k_to_rat:
        return defined_functor(basic(b_torat), 1);
      case k_to_real:
        return defined_functor(basic(b_toreal), 1);
      case k_distinct: {
        lex();
        args(v);
        vec<w> clauses(basic(b_and));
        for (auto i = v.begin(); i != v.end(); ++i)
          for (auto j = v.begin(); j != i; ++j)
            clauses.push(term(basic(b_not), term(basic(b_eq), *i, *j)));
        return term(clauses);
      }
      }
      throw "Unknown word";
    }
    }
    throw "Syntax error";
  }

  w infix_unary() {
    auto a = atomic_term();
    switch (tok) {
    case '=':
      lex();
      return term(basic(b_eq), a, atomic_term());
    case o_ne:
      lex();
      return term(basic(b_not), term(basic(b_eq), a, atomic_term()));
    }
    return a;
  }

  w quant(w op) {
    lex();
    expect('[');
    auto old = bound.n;
    vec<w> v(op, 0);
    do {
      if (tok != o_var)
        throw "Expected variable";
      auto name = tokSym;
      lex();
      auto t = t_individual;
      if (eat(':'))
        t = read_type();
      auto x = var(t, bound.n);
      bound.push(std::make_pair(name, x));
      v.push(x);
    } while (eat(','));
    expect(']');
    expect(':');
    v[1] = unitary_formula();
    bound.n = old;
    return term(v);
  }

  w unitary_formula() {
    switch (tok) {
    case '~':
      lex();
      return term(basic(b_not), unitary_formula());
    case '(': {
      lex();
      auto a = logic_formula();
      expect(')');
      return a;
    }
    case '!':
      return quantified(basic(b_all));
    case '?':
      return quantified(basic(b_exists));
    }
    return infix_unary();
  }

  w associativeLogicFormula(w op, w a) {
    vec<w> v(op, a);
    auto o = tok;
    while (eat(o))
      v.push(unitary_formula());
    return term(v);
  }

  w logic_formula() {
    auto a = unitary_formula();
    switch (tok) {
    case '&':
      return associativeLogicFormula(basic(b_and), a);
    case '|':
      return associativeLogicFormula(basic(b_or), a);
    case o_eqv:
      lex();
      return term(basic(b_eqv), a, unitary_formula());
    case o_imp:
      lex();
      return imp(a, unitary_formula());
    case o_impr:
      lex();
      return imp(unitary_formula(), a);
    case o_nand:
      lex();
      return term(basic(b_not), term(basic(b_and), a, unitary_formula()));
    case o_nor:
      lex();
      return term(basic(b_not), term(basic(b_or), a, unitary_formula()));
    case o_xor:
      lex();
      return term(basic(b_not), term(basic(b_eqv), a, unitary_formula()));
    }
    return a;
  }

  void ignore() {
    switch (tok) {
    case 0:
      throw "Unexpected end of file";
    case '(':
      lex();
      while (!eat(')'))
        ignore();
      return;
    }
    lex();
  }

  w formula_name() {
    switch (tok) {
    case o_word:
      return atomic_term();
    case o_term:
      auto a = tok_term;
      lex();
      return a;
    }
    throw "Expected formula name";
  }

  // top level

  void read_tptp1(const char *file);

  void annotated_formula() {
    lex();
    expect('(');

    // name
    auto name = formula_name();
    expect(',');

    // role
    if (tok != o_word)
      throw "role expected";
    auto role = tokSym;
    if (role == keywords + k_conjecture && conjecture)
      throw "multiple conjectures not supported";
    lex();
    expect(',');

    // formula
    if (role == keywords + k_type)
      typing();
    else {
      auto a = logic_formula();
      if (role == keywords + k_conjecture) {
        get_free_vars(a);
        a = term(basic(b_not), term(basic(b_all), a, free_vars));
        conjecture = true;
      }
      cnf(a);
    }

    // annotations
    if (eat(','))
      while (tok != ')')
        ignore();

    // end
    expect(')');
    expect('.');
  }

  void include() {
    auto tptp = getenv("TPTP");
    if (!tptp)
      throw "TPTP environment variable not set";
    lex();
    expect('(');

    // file
    if (tok != o_word)
      throw "file expected";
    auto n = strlen(tptp);
    vec<char> filename1(n + tokSym->n + 2);
    memcpy(filename1.p, tptp, n);
    filename1[n] = '/';
    memcpy(filename1.p + n + 1, tokSym->s, tokSym->n);
    filename1[n + 1 + tokSym->n] = 0;
    lex();

    // select
    auto old_select = select;
    if (eat(',')) {
      expect('[');
      select = selection(false);
      do {
        auto name = formula_name();
        if (old_select.count(name))
          select.insert(name);
      } while (eat(','));
      expect(']');
    }

    // save current token
    auto old_src1 = src1;
    auto old_tok = tok;
    auto old_tokString = tokSym;
    auto old_tok_term = tok_term;

    // read
    read_tptp1(filename1.p);

    // restore current token
    src1 = old_src1;
    tok = old_tok;
    tokSym = old_tokString;
    tok_term = old_tok_term;

    // restore select
    select = old_select;

    // end
    expect(')');
    expect('.');
  }

  TptpParser(const char *file) : Parser(file) {
    try {
      lex();
      while (tok) {
        if (tok != o_word)
          throw "Expected formula";
        switch (keyword(tokSym)) {
        case k_cnf:
        case k_fof:
        case k_tff:
          annotated_formula();
          break;
        case k_include:
          include();
          break;
        default:
          throw "Unknown language";
        }
      }
    } catch (const char *e) {
      err(e);
    }
  }
};
} // namespace

void readTptp(const char *file) {
  isWord['$'] = 1;
  memset(isWord + '0', 1, 10);
  memset(isWord + 'A', 1, 26);
  isWord['_'] = 1;
  memset(isWord + 'a', 1, 26);
  TptpParser parser(file);
}
