#include "main.h"

namespace {
enum {
  o_distinctobj = 1,
  o_dollarword,
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

char isword[0x100];
#ifdef DEBUG
w header;
#endif

struct Select : std::unordered_set<Sym *> {
  bool all;

  explicit Select(bool all) : all(all) {}

  w count(Sym *S) const {
    if (all)
      return 1;
    return std::unordered_set<Sym *>::count(S);
  }
};

void strmemcpy(char *dest, const char *src, const char *end) {
  auto n = end - src;
  memcpy(dest, src, n);
  dest[n] = 0;
}

struct TptpParser : Parser {
  Select select;
  bool cnfMode;
  vec<std::pair<Sym *, w>> vars;

  // Tokenizer
  void word() {
    auto s = text;
    while (isword[*s])
      ++s;
    tokSym = intern(text, s - text);
    text = s;
  }

  void quote() {
    auto s = text;
    auto q = *s++;
    w i = 0;
    while (*s != q) {
      if (*s == '\\')
        ++s;
      if (*s < ' ')
        err("Unclosed quote");
      if (i >= sizeof buf)
        err("Symbol too long");
      buf[i++] = *s++;
    }
    text = s + 1;
    tokSym = intern(buf, i);
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
    while (isdigit1(*s))
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
    sign();
    // GMP doesn't handle unary +, so need to omit it from token
    if (*tokStart == '+')
      ++tokStart;
    // Sign without digits should give a clear error message
    if (!isdigit1(*text))
      err("Expected digit", text);
    tok = o_int;
    digits();
    switch (*text) {
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
    case 'E':
    case 'e':
      tok = o_real;
      exp();
      break;
    }
    if (text - tokStart > sizeof buf - 1)
      err("Number too long");
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
    case '!':
      switch (s[1]) {
      case '=':
        text = s + 2;
        tok = o_ne;
        return;
      }
      break;
    case '"':
      tok = o_distinctobj;
      quote();
      return;
    case '$':
      text = s + 1;
      tok = o_dollarword;
      word();
      return;
    case '%': {
      text = strchr(s, '\n');
#ifdef DEBUG
      if (!status) {
        std::string s1(s, text);
        std::smatch m;
        if (std::regex_match(s1, m, std::regex(R"(% Status\s*:\s*(\w+)\s*)"))) {
          for (w i = 1; i != n_szs; ++i)
            if (m[1] == szs[i]) {
              status = i;
              break;
            }
          if (!status)
            err("Unknown status");
        }
      }
      if (header) {
        if (s[1] == '-' && s[2] == '-')
          --header;
        while (s != text)
          putchar(*s++);
        putchar('\n');
        if (s[1] == '\n')
          putchar('\n');
      }
#endif
      goto loop;
    }
    case '+':
    case '-':
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
      num();
      return;
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
    case '=':
      switch (s[1]) {
      case '>':
        text = s + 2;
        tok = o_imp;
        return;
      }
      break;
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
    case '\'':
      tok = o_word;
      quote();
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
    sprintf(buf, "Expected '%c'", o);
    err(buf);
  }

  void expect(char o, const char *s) {
    if (eat(o))
      return;
    sprintf(buf, "Expected %s", s);
    err(buf);
  }

  // Types
  w atomicType() {
    auto k = tok;
    auto S = tokSym;
    auto ts = tokStart;
    lex();
    switch (k) {
    case '!':
    case '[':
      throw Inappropriate();
    case '(': {
      auto t = atomicType();
      expect(')');
      return t;
    }
    case o_dollarword:
      switch (keyword(S)) {
      case k_i:
        return t_individual;
      case k_int:
        return t_int;
      case k_o:
        return t_bool;
      case k_rat:
        return t_rat;
      case k_real:
        return t_real;
      }
      throw Inappropriate();
    case o_word:
      return type(S);
    default:
      err("Expected type", ts);
    }
  }

  w topLevelType() {
    if (eat('(')) {
      vec<uint16_t> v(0);
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
  w parseInt() {
    strmemcpy(buf, tokStart, text);
    Int x;
    if (mpz_init_set_str(x.val, buf, 10))
      err("Invalid number");
    lex();
    return int1(&x);
  }

  w parseRat() {
    strmemcpy(buf, tokStart, text);
    Rat x;
    mpq_init(x.val);
    if (mpq_set_str(x.val, buf, 10))
      err("Invalid number");
    mpq_canonicalize(x.val);
    lex();
    return rat(&x);
  }

  w parseReal() {
    auto p = tokStart;

    // Sign
    auto sign = false;
    if (*p == '-') {
      ++p;
      sign = true;
    }

    // Integer part
    auto q = p;
    while (isdigit1(*q))
      ++q;
    strmemcpy(buf, p, q);
    mpz_t integer;
    mpz_init_set_str(integer, buf, 10);
    p = q;

    // Decimal part
    mpz_t mantissa;
    mpz_init(mantissa);
    w scale = 0;
    if (*p == '.') {
      ++p;
      q = p;
      while (isdigit1(*q))
        ++q;
      strmemcpy(buf, p, q);
      mpz_set_str(mantissa, buf, 10);
      scale = q - p;
      p = q;
    }
    mpz_t powScale;
    mpz_init(powScale);
    mpz_ui_pow_ui(powScale, 10, scale);

    // Mantissa += integer * 10^scale
    mpz_addmul(mantissa, integer, powScale);

    // Sign
    if (sign)
      mpz_neg(mantissa, mantissa);

    // Result = scaled mantissa
    Rat x;
    mpq_init(x.val);
    mpq_set_num(x.val, mantissa);
    mpq_set_den(x.val, powScale);

    // Exponent
    auto exponentSign = false;
    w exponent = 0;
    if (*p == 'e' || *p == 'E') {
      ++p;
      switch (*p) {
      case '-':
        exponentSign = true;
      case '+':
        ++p;
        break;
      }
      errno = 0;
      exponent = strtoul(p, 0, 10);
      if (errno)
        err(strerror(errno));
    }
    mpz_t powExponent;
    mpz_init(powExponent);
    mpz_ui_pow_ui(powExponent, 10, exponent);
    if (exponentSign)
      mpz_mul(mpq_denref(x.val), mpq_denref(x.val), powExponent);
    else
      mpz_mul(mpq_numref(x.val), mpq_numref(x.val), powExponent);

    // Cleanup
    mpz_clear(integer);
    mpz_clear(mantissa);
    mpz_clear(powScale);
    mpz_clear(powExponent);

    // Result
    lex();
    mpq_canonicalize(x.val);
    return real(&x);
  }

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
    sprintf(buf, "Expected %zu arguments", arity);
    err(buf);
  }

  w definedFunctor(w op, w arity) {
    vec<w> v(op);
    args(v, arity);
    auto t = numType(v[1]);
    for (auto i = v.begin() + 2; i != v.end(); ++i)
      requireType(t, *i);
    return term(v);
  }

  w atomicTerm() {
    switch (tok) {
    case o_distinctobj: {
      auto a = tag(tokSym, a_distinctobj);
      lex();
      return a;
    }
    case o_dollarword: {
      auto S = tokSym;
      auto ts = tokStart;
      lex();
      vec<w> v;
      switch (keyword(S)) {
      case k_ceiling:
        return definedFunctor(basic(b_ceil), 1);
      case k_difference:
        return definedFunctor(basic(b_sub), 2);
      case k_distinct: {
        args(v);
        defaultType(t_individual, v[0]);
        auto t = typeof(v[0]);
        for (auto i = v.begin() + 1; i != v.end(); ++i)
          requireType(t, *i);
        vec<w> clauses(basic(b_and));
        for (auto i = v.begin(); i != v.end(); ++i)
          for (auto j = v.begin(); j != i; ++j)
            clauses.push(term(basic(b_not), term(basic(b_eq), *i, *j)));
        return term(clauses);
      }
      case k_false:
        return basic(b_false);
      case k_floor:
        return definedFunctor(basic(b_floor), 1);
      case k_greater: {
        args(v, 2);
        auto t = numType(v[0]);
        requireType(t, v[1]);
        return term(basic(b_lt), v[1], v[0]);
      }
      case k_greatereq: {
        args(v, 2);
        auto t = numType(v[0]);
        requireType(t, v[1]);
        return term(basic(b_le), v[1], v[0]);
      }
      case k_is_int:
        return definedFunctor(basic(b_isint), 1);
      case k_is_rat:
        return definedFunctor(basic(b_israt), 1);
      case k_less:
        return definedFunctor(basic(b_lt), 2);
      case k_lesseq:
        return definedFunctor(basic(b_le), 2);
      case k_product:
        return definedFunctor(basic(b_mul), 2);
      case k_quotient: {
        auto a = definedFunctor(basic(b_div), 2);
        if (typeof(at(a, 1)) == t_int)
          err("Expected fraction term");
        return a;
      }
      case k_quotient_e:
        return definedFunctor(basic(b_dive), 2);
      case k_quotient_f:
        return definedFunctor(basic(b_divf), 2);
      case k_quotient_t:
        return definedFunctor(basic(b_divt), 2);
      case k_remainder_e:
        return definedFunctor(basic(b_reme), 2);
      case k_remainder_f:
        return definedFunctor(basic(b_remf), 2);
      case k_remainder_t:
        return definedFunctor(basic(b_remt), 2);
      case k_round:
        return definedFunctor(basic(b_round), 1);
      case k_sum:
        return definedFunctor(basic(b_add), 2);
      case k_to_int:
        return definedFunctor(basic(b_toint), 1);
      case k_to_rat:
        return definedFunctor(basic(b_torat), 1);
      case k_to_real:
        return definedFunctor(basic(b_toreal), 1);
      case k_true:
        return basic(b_true);
      case k_truncate:
        return definedFunctor(basic(b_trunc), 1);
      case k_uminus:
        return definedFunctor(basic(b_minus), 1);
      }
      err("Unknown word", ts);
    }
    case o_int:
      return parseInt();
    case o_rat:
      return parseRat();
    case o_real:
      return parseReal();
    case o_var: {
      auto S = tokSym;
      auto ts = tokStart;
      lex();
      for (auto i = vars.rbegin(); i != vars.rend(); ++i)
        if (i->first == S)
          return i->second;
      if (!cnfMode)
        err("Unknown variable", ts);
      auto x = var(t_individual, vars.n);
      vars.push(std::make_pair(S, x));
      return x;
    }
    case o_word: {
      auto a = tag(tokSym, a_sym);
      lex();
      if (tok != '(')
        return a;
      vec<w> v(a);
      args(v);
      for (auto i = v.begin() + 1; i != v.end(); ++i)
        defaultType(t_individual, *i);
      return term(v);
    }
    }
    err("Syntax error");
  }

  w infixUnary() {
    auto a = atomicTerm();
    switch (tok) {
    case '=': {
      lex();
      auto b = atomicTerm();
      defaultType(t_individual, a);
      requireType(typeof(a), b);
      return term(basic(b_eq), a, b);
    }
    case o_ne: {
      lex();
      auto b = atomicTerm();
      defaultType(t_individual, a);
      requireType(typeof(a), b);
      return term(basic(b_not), term(basic(b_eq), a, b));
    }
    }
    requireType(t_bool, a);
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
      w t = t_individual;
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
    case '!':
      return quantifiedFormula(basic(b_all));
    case '(': {
      lex();
      auto a = logicFormula();
      expect(')');
      return a;
    }
    case '?':
      return quantifiedFormula(basic(b_exists));
    case '~':
      lex();
      return term(basic(b_not), unitaryFormula());
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
  Sym *name() {
    switch (tok) {
    case o_int: {
      auto S = intern(tokStart, text - tokStart);
      lex();
      return S;
    }
    case o_word: {
      auto S = tokSym;
      lex();
      return S;
    }
    }
    err("Expected name");
  }

  void ignore() {
    switch (tok) {
    case '(':
      lex();
      while (!eat(')'))
        ignore();
      return;
    case 0:
      err("Unexpected end of file");
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
          auto formulaName = name();
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

          // Select
          if (!select.count(formulaName))
            break;

          // Clause
          clause();
          break;
        }
        case k_fof:
        case k_tff: {
          expect('(');

          // Name
          auto formulaName = name();
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
            w parens = 0;
            while (eat('('))
              ++parens;
            auto S = name();
            expect(':');
            ts = tokStart;
            if (tok == o_dollarword && tokSym == keywords + k_tType) {
              lex();
              if (tok == '>')
                throw Inappropriate();
            } else {
              auto t = topLevelType();
              if (S->ft) {
                if (t != typeof(S->ft))
                  err("Type mismatch", ts);
              } else
                S->ft = t;
            }
            while (parens--)
              expect(')');
            break;
          }

          // Formula
          cnfMode = false;
          auto a = logicFormula();
          assert(!vars.n);

          // Select
          if (!select.count(formulaName))
            break;

          // CNF
          if (role == k_conjecture) {
            a = term(basic(b_not), a);
            conjecture = true;
          }
          break;
        }
        case k_include: {
          auto tptp = getenv("TPTP");
          if (!tptp)
            err("TPTP environment variable not set", ts);
          expect('(');

          // File
          snprintf(buf, sizeof buf, "%s/%s", tptp, name()->v);
          auto file1 = intern(buf, strlen(buf))->v;

          // Select and read
          if (eat(',')) {
            expect('[');
            Select select1(false);
            do {
              auto formulaName = name();
              if (select.count(formulaName))
                select1.insert(formulaName);
            } while (eat(','));
            expect(']');
            TptpParser parser(file1, select1);
          } else {
            TptpParser parser(file1, select);
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
  memset(isword + '0', 1, 10);
  memset(isword + 'A', 1, 26);
  isword['_'] = 1;
  memset(isword + 'a', 1, 26);
#ifdef DEBUG
  header = 2;
#endif
  TptpParser parser(file, Select(true));
}
