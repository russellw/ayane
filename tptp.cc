#include "main.h"

namespace {
enum {
  o_distinctObj = 1,
  o_dollarWord,
  o_eqv,
  o_imp,
  o_impr,
  o_int,
  o_nand,
  o_ne,
  o_nor,
  o_rat,
  o_real,
  o_var,
  o_word,
  o_xor,
};

char isWord[0x100];

struct Select : std::unordered_set<sym *> {
  bool all;

  Select(const Select &x) : std::unordered_set<sym *>(x), all(x.all) {}
  explicit Select(bool all) : all(all) {}

  w count(sym *S) const {
    if (all)
      return 1;
    return std::unordered_set<sym *>::count(S);
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
        err("Unclosed quote");
      buf.push(*s++);
    }
    text = s + 1;
    tokSym = intern(buf.v, buf.n);
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
      err("Expected digit");
    }
    tok = o_int;
    digits();
    switch (*text) {
    case 'E':
    case 'e':
      tok = o_real;
      exp();
      break;
    case '.':
      tok = o_real;
      ++text;
      digits();
      switch (*text) {
      case 'E':
      case 'e':
        exp();
        break;
      }
      break;
    case '/':
      tok = o_rat;
      ++text;
      digits();
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
          err("Unknown status");
      }
#endif
      goto loop;
    }
    case '/':
      if (s[1] != '*') {
        text = s + 1;
        err("Expected '*'");
      }
      for (s += 2; !(s[0] == '*' && s[1] == '/'); ++s)
        if (!*s)
          err("Unclosed comment");
      text = s + 2;
      goto loop;
    case '$':
      text = s + 1;
      tok = o_dollarWord;
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
      tok = o_distinctObj;
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
        err("Expected '>'");
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
    err(buf.v);
  }

  void expect(char o, const char *s) {
    if (eat(o))
      return;
    sprintf(buf.v, "Expected %s", s);
    err(buf.v);
  }

  // Types

  ty atomicType() {
    auto k = tok;
    auto S = tokSym;
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
    case o_dollarWord:
      switch (keyword(S)) {
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
      return type(S);
    default:
      err("Expected type", ts);
    }
  }

  ty topLevelType() {
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
    auto t = atomicType();
    if (eat('>'))
      return type(atomicType(), t);
    return t;
  }

  // Terms

  void args(vec<w> &v) {
    expect('(');
    do
      v.push(atomicTerm());
    while (eat(','));
    expect(')');
  }

  void args(vec<w> &v, w arity) {
    auto old = v.n;
    args(v);
    if (v.n - old == arity)
      return;
    sprintf(buf.v, "Expected %zu arguments", arity);
    err(buf.v);
  }

  w definedFunctor(w op, w arity) {
    vec<w> v(op);
    args(v, arity);
    return term(v);
  }

  w atomicTerm() {
    auto S = tokSym;
    auto o = tok;
    auto ts = tokStart;
    lex();
    switch (o) {
    case o_distinctObj:
      return tag(S, a_distinctObj);
    case o_word: {
      auto f = S->f;
      if (!f)
        S->f = f = fn(0, S);
      if (tok != '(')
        return f;
      vec<w> v(f);
      args(v);
      return term(v);
    }
    case o_var: {
      for (auto i = vars.rbegin(); i != vars.rend(); ++i)
        if (i->first == S)
          return i->second;
      if (!cnfMode)
        err("Unknown variable", ts);
      auto x = var(t_individual, vars.n);
      vars.push(std::make_pair(S, x));
      return x;
    }
    case o_dollarWord: {
      vec<w> v;
      switch (keyword(S)) {
      case k_false:
        return basic(b_false);
      case k_true:
        return basic(b_true);
      case k_less:
        return definedFunctor(basic(b_lt), 2);
      case k_lesseq:
        return definedFunctor(basic(b_le), 2);
      case k_greater:
        args(v, 2);
        return term(basic(b_lt), v[1], v[0]);
      case k_greatereq:
        args(v, 2);
        return term(basic(b_le), v[1], v[0]);
      case k_uminus:
        return definedFunctor(basic(b_minus), 1);
      case k_sum:
        return definedFunctor(basic(b_add), 2);
      case k_difference:
        return definedFunctor(basic(b_sub), 2);
      case k_product:
        return definedFunctor(basic(b_mul), 2);
      case k_quotient:
        return definedFunctor(basic(b_div), 2);
      case k_quotient_e:
        return definedFunctor(basic(b_dive), 2);
      case k_quotient_t:
        return definedFunctor(basic(b_divt), 2);
      case k_quotient_f:
        return definedFunctor(basic(b_divf), 2);
      case k_remainder_e:
        return definedFunctor(basic(b_reme), 2);
      case k_remainder_t:
        return definedFunctor(basic(b_remt), 2);
      case k_remainder_f:
        return definedFunctor(basic(b_remf), 2);
      case k_floor:
        return definedFunctor(basic(b_floor), 1);
      case k_ceiling:
        return definedFunctor(basic(b_ceil), 1);
      case k_truncate:
        return definedFunctor(basic(b_trunc), 1);
      case k_round:
        return definedFunctor(basic(b_round), 1);
      case k_is_int:
        return definedFunctor(basic(b_isint), 1);
      case k_is_rat:
        return definedFunctor(basic(b_israt), 1);
      case k_to_int:
        return definedFunctor(basic(b_toint), 1);
      case k_to_rat:
        return definedFunctor(basic(b_torat), 1);
      case k_to_real:
        return definedFunctor(basic(b_toreal), 1);
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

  w infixUnary() {
    auto a = atomicTerm();
    switch (tok) {
    case '=':
      lex();
      return term(basic(b_eq), a, atomicTerm());
    case o_ne:
      lex();
      return term(basic(b_not), term(basic(b_eq), a, atomicTerm()));
    }
    return a;
  }

  w quantifiedFormula(w op) {
    lex();
    expect('[');
    auto old = vars.n;
    vec<w> v(op, 0);
    do {
      if (tok != o_var)
        err("Expected variable");
      auto S = tokSym;
      lex();
      ty t = t_individual;
      if (eat(':'))
        t = atomicType();
      auto x = var(t, vars.n);
      vars.push(std::make_pair(S, x));
      v.push(x);
    } while (eat(','));
    expect(']');
    expect(':');
    v[1] = unitaryFormula();
    vars.n = old;
    return term(v);
  }

  w unitaryFormula() {
    switch (tok) {
    case '~':
      lex();
      return term(basic(b_not), unitaryFormula());
    case '(': {
      lex();
      auto a = logicFormula();
      expect(')');
      return a;
    }
    case '!':
      return quantifiedFormula(basic(b_all));
    case '?':
      return quantifiedFormula(basic(b_exists));
    }
    return infixUnary();
  }

  w associativeLogicFormula(w op, w a) {
    vec<w> v(op, a);
    auto o = tok;
    while (eat(o))
      v.push(unitaryFormula());
    return term(v);
  }

  w logicFormula() {
    auto a = unitaryFormula();
    switch (tok) {
    case '&':
      return associativeLogicFormula(basic(b_and), a);
    case '|':
      return associativeLogicFormula(basic(b_or), a);
    case o_eqv:
      lex();
      return term(basic(b_eqv), a, unitaryFormula());
    case o_imp:
      lex();
      return imp(a, unitaryFormula());
    case o_impr:
      lex();
      return imp(unitaryFormula(), a);
    case o_nand:
      lex();
      return term(basic(b_not), term(basic(b_and), a, unitaryFormula()));
    case o_nor:
      lex();
      return term(basic(b_not), term(basic(b_or), a, unitaryFormula()));
    case o_xor:
      lex();
      return term(basic(b_not), term(basic(b_eqv), a, unitaryFormula()));
    }
    return a;
  }

  // Top level

  sym *name() {
    switch (tok) {
    case o_word: {
      auto S = tokSym;
      lex();
      return S;
    }
    case o_int: {
      auto S = intern(tokStart, text - tokStart);
      lex();
      return S;
    }
    }
    err("Expected name");
  }

  void ignore() {
    switch (tok) {
    case 0:
      err("Unexpected end of file");
    case '(':
      lex();
      while (!eat(')'))
        ignore();
      return;
    }
    lex();
  }

  TptpParser(const char *file, const Select &select)
      : Parser(file), select(select) {
    try {
      lex();
      while (tok) {
        auto ts = tokStart;
        vars.n = 0;
        switch (keyword(name())) {
        case k_cnf: {
          expect('(');

          // Name
          auto S = name();
          expect(',');

          // Role
          name();
          expect(',');

          // Literals
          cnfMode = true;
          neg.n = pos.n = 0;
          auto parens = eat('(');
          do {
            auto no = eat('~');
            auto a = infixUnary();
            if ((a & 7) == a_compound && at(a, 0) == basic(b_not)) {
              no = !no;
              a = at(a, 1);
            }
            (no ? neg : pos).push(a);
          } while (eat('|'));
          if (parens)
            expect(')');
          if (select.count(S))
            clause();
          break;
        }
        case k_fof:
        case k_tff: {
          expect('(');

          // Name
          auto S = name();
          expect(',');

          // Role
          if (tok != o_word)
            err("Expected role");
          auto role = keyword(tokSym);
          if (role == k_conjecture && conjecture)
            err("Multiple conjectures not supported");
          lex();
          expect(',');

          // Type
          if (role == k_type) {
            auto parens = 0;
            while (eat('('))
              ++parens;
            auto funcName = name();
            expect(':');
            ts = tokStart;
            if (tok == o_word && tokSym == keywords + k_tType) {
              lex();
              if (tok == '>')
                throw Inappropriate();
            } else {
              auto t = topLevelType();
              if (funcName->f) {
                if (t != typeof(funcName->f))
                  err("Type mismatch", ts);
              } else
                funcName->f = fn(t, funcName);
            }
            while (parens-- > 0)
              expect(')');
            break;
          }

          // Formula
          cnfMode = false;
          auto a = logicFormula();
          assert(!vars.n);
          if (select.count(S)) {
            if (role == k_conjecture) {
              a = term(basic(b_not), a);
              conjecture = true;
            }
          }
          break;
        }
        case k_include: {
          auto tptp = getenv("TPTP");
          if (!tptp)
            err("TPTP environment variable not set", ts);
          expect('(');

          // File
          auto fname = name();
          auto n = strlen(tptp);
          vec<char> file1;
          file1.resize(n + fname->n + 2);
          memcpy(file1.p, tptp, n);
          file1[n] = '/';
          memcpy(file1.p + n + 1, fname->v, fname->n);
          file1[n + 1 + fname->n] = 0;

          // Select and read
          if (eat(',')) {
            expect('[');
            Select select1(false);
            do {
              auto S = name();
              if (select.count(S))
                select1.insert(S);
            } while (eat(','));
            expect(']');
            TptpParser parser(file1.p, select1);
          } else {
            TptpParser parser(file1.p, select);
          }
          break;
        }
        default:
          err("Unknown language", ts);
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
