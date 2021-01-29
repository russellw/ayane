#include "main.h"

namespace {
enum {
  o_dollarword = 1,
  o_eqv,
  o_imp,
  o_int,
  o_rat,
  o_real,
  o_distinctobj,
  o_impr,
  o_nand,
  o_ne,
  o_nor,
  o_var,
  o_word,
  o_xor,
};

char isWord[0x100];

struct Select : std::unordered_set<sym *> {
  bool all;

  Select(const Select &x) : std::unordered_set<sym *>(x), all(x.all) {}
  explicit Select(bool all) : all(all) {}

  w count(sym *name) const {
    if (all)
      return 1;
    return std::unordered_set<sym *>::count(name);
  }
};

struct TptpParser : Parser {
  Select select;
  bool cnfMode;
  vec<std::pair<sym *, w>> vars;

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

  void sign() {
    switch (*text) {
    case '+':
    case '-':
      ++text;
      break;
    }
  }

  void digits() {
    auto s = text;
    while (isDigit(*s))
      ++s;
    text = s;
  }

  void exp() {
    assert(*text == 'E' || *text == 'e');
    ++text;
    sign();
    digits();
  }

  void num() {
    if (*text == '+')
      tokStart = text;
    sign();
    if (!isDigit(*text)) {
      tokStart = text;
      throw "Expected digit";
    }
    digits();
    tok = o_int;
    switch (*text) {
    case 'E':
    case 'e':
      exp();
      tok = o_real;
      break;
    case '.':
      ++text;
      digits();
      switch (*text) {
      case 'E':
      case 'e':
        exp();
        break;
      }
      tok = o_real;
      break;
    case '/':
      ++text;
      digits();
      tok = o_rat;
      break;
    }
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
          throw "Unknown status";
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
    case '+':
    case '-':
      num();
      return;
    case '"':
      tok = o_distinctobj;
      quote();
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

  // Types

  ty atomicType() {
    auto k = tok;
    auto name = tokSym;
    auto ts = tokStart;
    lex();
    switch (k) {
    case '(': {
      auto t = atomicType();
      expect(')');
      return t;
    }
    case '!':
    case '[':
      throw Inappropriate();
    case o_dollarword:
      switch (keyword(name)) {
      case k_o:
        return t_bool;
      case k_int:
        return t_int;
      case k_rat:
        return t_rat;
      case k_real:
        return t_real;
      case k_i:
        return t_individual;
      }
      err("Unknown word", ts);
    case o_word:
      return type(name);
    default:
      throw new ParseException(file, reader.getLineNumber(), "type expected");
    }
  }

  ty type1() {
    if (eat('(')) {
      vec<ty> v(0);
      do
        v.push(atomicType());
      while (eat('*'));
      expect(')');
      expect('>');
      v[0] = atomicType();
      return type(v);
    }
    var t = atomicType();
    if (eat('>'))
      return type(atomicType(), t);
    return t;
  }

  // Terms

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
    vec<w> v(op);
    args(v, arity);
    return term(v);
  }

  w atomic_term() {
    auto name = tokSym;
    auto o = tok;
    auto ts = tokStart;
    lex();
    switch (tok) {
    case o_distinctobj:
      return tag(name, a_distinctobj);
    case o_word: {
      auto f = name->f;
      if (!f)
        name->f = f = fn(0, name);
      if (tok != '(')
        return f;
      vec<w> v(f);
      args(v);
      return term(v);
    }
    case o_var: {
      for (auto it = vars.rbegin(); it != vars.rend(); ++it)
        if (it->first == name)
          return it->second;
      if (!cnfMode)
        err("Unknown variable", ts);
      auto x = var(t_individual, vars.n);
      vars.push(std::make_pair(name, x));
      return x;
    }
    case o_dollarword: {
      vec<w> v;
      switch (keyword(name)) {
      case k_false:
        return basic(b_false);
      case k_true:
        return basic(b_true);
      case k_less:
        return defined_functor(basic(b_lt), 2);
      case k_lesseq:
        return defined_functor(basic(b_le), 2);
      case k_greater:
        args(v, 2);
        return term(basic(b_lt), v[1], v[0]);
      case k_greatereq:
        args(v, 2);
        return term(basic(b_le), v[1], v[0]);
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
        args(v);
        vec<w> clauses(basic(b_and));
        for (auto i = v.begin(); i != v.end(); ++i)
          for (auto j = v.begin(); j != i; ++j)
            clauses.push(term(basic(b_not), term(basic(b_eq), *i, *j)));
        return term(clauses);
      }
      }
      err("Unknown word", ts);
    }
    }
    err("Syntax error", ts);
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
    auto old = vars.n;
    vec<w> v(op, 0);
    do {
      if (tok != o_var)
        throw "Expected variable";
      auto name = tokSym;
      lex();
      ty t = t_individual;
      if (eat(':'))
        t = read_type();
      auto x = var(t, vars.n);
      vars.push(std::make_pair(name, x));
      v.push(x);
    } while (eat(','));
    expect(']');
    expect(':');
    v[1] = unitary_formula();
    vars.n = old;
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
      return quant(basic(b_all));
    case '?':
      return quant(basic(b_exists));
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

  sym *formula_name() {
    switch (tok) {
    case o_word: {
      auto name = tokSym;
      lex();
      return name;
    }
    case o_int: {
      auto name = intern(tokStart, text - tokStart);
      lex();
      return name;
    }
    }
    throw "Expected formula name";
  }

  // top level

  TptpParser(const char *file, const Select &select)
      : Parser(file), select(select) {
    try {
      lex();
      while (tok) {
        if (tok != o_word)
          throw "Expected formula";
        vars.n = 0;
        switch (keyword(tokSym)) {
        case k_cnf: {
          lex();
          expect('(');

          // Name
          auto name = formula_name();
          expect(',');

          // Role
          formula_name();
          expect(',');

          // Literals
          cnfMode = true;
          neg.n = pos.n = 0;
          auto parens = eat('(');
          do {
            auto not = eat('~');
            auto a = infix_unary();
            if ((a & 7) == a_compound && at(a, 0) == basic(b_not)) {
              not = !not;
              a = at(a, 1);
            }
            (not? neg : pos).push(a);
          } while (eat('|'));
          if (parens)
            expect(')');
          if (select.count(name))
            clause();
          break;
        }
        case k_fof:
        case k_tff: {
          lex();
          expect('(');

          // Name
          auto name = formula_name();
          expect(',');

          // Role
          if (tok != o_word)
            throw "Expected role";
          auto role = keyword(tokSym);
          if (role == k_conjecture && conjecture)
            throw "Multiple conjectures not supported";
          lex();
          expect(',');

          // Type
          if (role == k_type) {
            auto parens = 0;
            while (eat('('))
              ++parens;
            auto funcName = formula_name();
            expect(':');
            if (tok == o_word && tokSym == keywords + tType) {
              lex();
              if (tok == '>')
                throw Inappropriate();
            } else {
              auto t = type1();
              auto a = problem.funcs.get(funcName);
              if (a == null) {
                a = new Func(type, funcName);
                problem.funcs.put(funcName, a);
              } else if (!Types.typeof(a).equals(type))
                throw new ParseException(file, reader.getLineNumber(),
                                         "type mismatch");
            }
            while (parens-- > 0)
              expect(')');
            break;
          }

          // Formula
          cnfMode = false;
          auto a = logic_formula();
          assert(!vars.n);
          if (role == k_conjecture) {
            a = term(basic(b_not), a);
            conjecture = true;
          }
          break;
        }
        case k_include: {
          auto tptp = getenv("TPTP");
          if (!tptp)
            throw "TPTP environment variable not set";
          lex();
          expect('(');

          // File
          if (tok != o_word)
            throw "Expected name";
          auto n = strlen(tptp);
          vec<char> file1;
          file1.resize(n + tokSym->n + 2);
          memcpy(file1.p, tptp, n);
          file1[n] = '/';
          memcpy(file1.p + n + 1, tokSym->v, tokSym->n);
          file1[n + 1 + tokSym->n] = 0;
          lex();

          // Select and read
          if (eat(',')) {
            expect('[');
            Select select1(false);
            do {
              auto name = formula_name();
              if (select.count(name))
                select1.insert(name);
            } while (eat(','));
            expect(']');
            TptpParser parser(file1.p, select1);
          } else {
            TptpParser parser(file1.p, select);
          }
          break;
        }
        default:
          throw "Unknown language";
        }
        if (tok == ',')
          do
            ignore();
          while (tok != ')');
        expect(')');
        expect('.');
      }
    } catch (const char *e) {
      err(e);
    }
  }
};
} // namespace

void readTptp(const char *file) {
  memset(isWord + '0', 1, 10);
  memset(isWord + 'A', 1, 26);
  isWord['_'] = 1;
  memset(isWord + 'a', 1, 26);
  TptpParser parser(file, Select(true));
}
