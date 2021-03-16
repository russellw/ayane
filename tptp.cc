#include "stdafx.h"
// stdafx.h must be first
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
#ifdef DEBUG
si header;
#endif

struct init {
  init() {
    memset(isWord + '0', 1, 10);
    memset(isWord + 'A', 1, 26);
    isWord['_'] = 1;
    memset(isWord + 'a', 1, 26);
  }
} init1;

struct selection : unordered_set<sym *> {
  bool all;

  explicit selection(bool all) : all(all) {}

  si count(sym *s) const {
    if (all)
      return 1;
    return unordered_set<sym *>::count(s);
  }
};

void strmemcpy(char *dest, const char *src, const char *e) {
  auto n = e - src;
  memcpy(dest, src, n);
  dest[n] = 0;
}

struct parser1 : parser {
  // SORT
  bool cnfMode;
  selection sel;
  vec<pair<sym *, term>> vars;
  ///

  // tokenizer
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
    si i = 0;
    while (*s != q) {
      if (*s == '\\')
        ++s;
      if (*s < ' ')
        err("unclosed quote");
      if (i >= sizeof buf)
        err("symbol too long");
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
    sign();
    // gmp doesn't handle unary +, so need to omit it from token
    if (*tokStart == '+')
      ++tokStart;
    // sign without digits should give a clear error message
    if (!isDigit(*text))
      err("expected digit", text);
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
      err("number too long");
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
      tok = o_distinctObj;
      quote();
      return;
    case '$':
      text = s + 1;
      tok = o_dollarWord;
      word();
      return;
    case '%': {
      text = strchr(s, '\n');
#ifdef DEBUG
      if (expected == szs::none) {
        string s1(s, text);
        smatch m;
        if (regex_match(s1, m, regex(R"(% Status\s*:\s*(\term+)\s*)"))) {
          for (si i = 1; i != (si)szs::max; ++i)
            if (m[1] == szsNames[i]) {
              expected = (szs)i;
              break;
            }
          if (expected == szs::none)
            err("unknown status");
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
        err("expected '*'");
      }
      for (s += 2; !(s[0] == '*' && s[1] == '/'); ++s)
        if (!*s)
          err("unclosed comment");
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
        err("expected '>'");
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
      return 1;
    }
    return 0;
  }

  void expect(char o) {
    if (eat(o))
      return;
    sprintf(buf, "expected '%c'", o);
    err(buf);
  }

  void expect(char o, const char *s) {
    if (eat(o))
      return;
    sprintf(buf, "expected %s", s);
    err(buf);
  }

  // types
  type atomicType() {
    auto k = tok;
    auto s = tokSym;
    auto ts = tokStart;
    lex();
    switch (k) {
    case '!':
    case '[':
      throw inappropriate();
    case '(': {
      auto t = atomicType();
      expect(')');
      return t;
    }
    case o_dollarWord:
      switch (keyword(s)) {
      case k_i:
        return type::Individual;
      case k_int:
        return type::Int;
      case k_o:
        return type::Bool;
      case k_rat:
        return type::Rat;
      case k_real:
        return type::Real;
      }
      throw inappropriate();
    case o_word:
      return mktype(s);
    default:
      err("expected type", ts);
    }
  }

  type topLevelType() {
    if (eat('(')) {
      vec<type> v(1);
      do
        v.push_back(atomicType());
      while (eat('*'));
      expect(')');
      expect('>');
      v[0] = atomicType();
      return mktype(v);
    }
    auto t = atomicType();
    if (eat('>'))
      return mktype(atomicType(), t);
    return t;
  }

  // terms
  term parseInt() {
    strmemcpy(buf, tokStart, text);
    Int x;
    if (mpz_init_set_str(x.val, buf, 10))
      err("invalid number");
    lex();
    return tag(term::Int, intern(x));
  }

  term parseRat() {
    strmemcpy(buf, tokStart, text);
    Rat x;
    mpq_init(x.val);
    if (mpq_set_str(x.val, buf, 10))
      err("invalid number");
    mpq_canonicalize(x.val);
    lex();
    return tag(term::Rat, intern(x));
  }

  term parseReal() {
    auto p = tokStart;

    // sign
    bool sign = 0;
    if (*p == '-') {
      ++p;
      sign = 1;
    }

    // integer part
    auto q = p;
    while (isDigit(*q))
      ++q;
    strmemcpy(buf, p, q);
    mpz_t integer;
    mpz_init_set_str(integer, buf, 10);
    p = q;

    // decimal part
    mpz_t mantissa;
    mpz_init(mantissa);
    si scale = 0;
    if (*p == '.') {
      ++p;
      q = p;
      while (isDigit(*q))
        ++q;
      strmemcpy(buf, p, q);
      mpz_set_str(mantissa, buf, 10);
      scale = q - p;
      p = q;
    }
    mpz_t powScale;
    mpz_init(powScale);
    mpz_ui_pow_ui(powScale, 10, scale);

    // mantissa += integer * 10^scale
    mpz_addmul(mantissa, integer, powScale);

    // sign
    if (sign)
      mpz_neg(mantissa, mantissa);

    // result = scaled mantissa
    Rat x;
    mpq_init(x.val);
    mpq_set_num(x.val, mantissa);
    mpq_set_den(x.val, powScale);

    // exponent
    bool exponentSign = 0;
    si exponent = 0;
    if (*p == 'e' || *p == 'e') {
      ++p;
      switch (*p) {
      case '-':
        exponentSign = 1;
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

    // cleanup
    mpz_clear(integer);
    mpz_clear(mantissa);
    mpz_clear(powScale);
    mpz_clear(powExponent);

    // result
    lex();
    mpq_canonicalize(x.val);
    return tag(term::Real, intern(x));
  }

  void args(vec<term> &v) {
    expect('(');
    do
      v.push_back(atomicTerm());
    while (eat(','));
    expect(')');
  }

  void args(vec<term> &v, si arity) {
    auto old = v.n;
    args(v);
    if (v.n - old == arity)
      return;
    sprintf(buf, "expected %zu arguments", arity);
    err(buf);
  }

  term definedFunctor(term op, si arity) {
    vec<term> v;
    args(v, arity);
    auto t = typeofNum(v[0]);
    for (auto i = v.p + 1, e = v.end(); i != e; ++i)
      requireType(t, *i);
    return intern(op, v);
  }

  term atomicTerm() {
    switch (tok) {
    case '!':
      throw inappropriate();
    case o_distinctObj: {
      auto a = tag(term::DistinctObj, tokSym);
      lex();
      return a;
    }
    case o_dollarWord: {
      auto s = tokSym;
      auto ts = tokStart;
      lex();
      vec<term> v;
      switch (keyword(s)) {
      case k_ceiling:
        return definedFunctor(term::Ceil, 1);
      case k_difference:
        return definedFunctor(term::Sub, 2);
      case k_distinct: {
        args(v);
        defaultType(type::Individual, v[0]);
        auto t = typeof(v[0]);
        for (auto i = v.p + 1, e = v.end(); i != e; ++i)
          requireType(t, *i);
        vec<term> clauses;
        for (auto i = v.p, e = v.end(); i != e; ++i)
          for (auto j = v.p; j != i; ++j)
            clauses.push_back(intern(term::Not, intern(term::Eq, *i, *j)));
        return intern(term::And, clauses);
      }
      case k_false:
        return term::False;
      case k_floor:
        return definedFunctor(term::Floor, 1);
      case k_greater: {
        args(v, 2);
        auto t = typeofNum(v[0]);
        requireType(t, v[1]);
        return intern(term::Lt, v[1], v[0]);
      }
      case k_greatereq: {
        args(v, 2);
        auto t = typeofNum(v[0]);
        requireType(t, v[1]);
        return intern(term::Le, v[1], v[0]);
      }
      case k_is_int:
        return definedFunctor(term::IsInt, 1);
      case k_is_rat:
        return definedFunctor(term::IsRat, 1);
      case k_ite:
        throw inappropriate();
      case k_less:
        return definedFunctor(term::Lt, 2);
      case k_lesseq:
        return definedFunctor(term::Le, 2);
      case k_product:
        return definedFunctor(term::Mul, 2);
      case k_quotient: {
        auto a = definedFunctor(term::Div, 2);
        if (typeof(at(a, 1)) == type::Int)
          err("expected fraction term");
        return a;
      }
      case k_quotient_e:
        return definedFunctor(term::DivE, 2);
      case k_quotient_f:
        return definedFunctor(term::DivF, 2);
      case k_quotient_t:
        return definedFunctor(term::DivT, 2);
      case k_remainder_e:
        return definedFunctor(term::RemE, 2);
      case k_remainder_f:
        return definedFunctor(term::RemF, 2);
      case k_remainder_t:
        return definedFunctor(term::RemT, 2);
      case k_round:
        return definedFunctor(term::Round, 1);
      case k_sum:
        return definedFunctor(term::Add, 2);
      case k_to_int:
        return definedFunctor(term::ToInt, 1);
      case k_to_rat:
        return definedFunctor(term::ToRat, 1);
      case k_to_real:
        return definedFunctor(term::ToReal, 1);
      case k_true:
        return term::True;
      case k_truncate:
        return definedFunctor(term::Trunc, 1);
      case k_uminus:
        return definedFunctor(term::Minus, 1);
      }
      err("unknown word", ts);
    }
    case o_int:
      return parseInt();
    case o_rat:
      return parseRat();
    case o_real:
      return parseReal();
    case o_var: {
      auto s = tokSym;
      auto ts = tokStart;
      lex();
      for (auto i = vars.rbegin(), e = vars.rend(); i != e; ++i)
        if (i->first == s)
          return i->second;
      if (!cnfMode)
        err("unknown variable", ts);
      auto x = var(type::Individual, vars.n);
      vars.push_back(make_pair(s, x));
      return x;
    }
    case o_word: {
      auto a = tag(term::Sym, tokSym);
      lex();
      if (tok != '(')
        return a;
      vec<term> v(1);
      v[0] = a;
      args(v);
      for (auto i = v.p + 1, e = v.end(); i != e; ++i)
        defaultType(type::Individual, *i);
      return intern(term::Call, v);
    }
    }
    err("syntax error");
  }

  term infixUnary() {
    auto a = atomicTerm();
    switch (tok) {
    case '=': {
      lex();
      auto b = atomicTerm();
      defaultType(type::Individual, a);
      requireType(typeof(a), b);
      return intern(term::Eq, a, b);
    }
    case o_ne: {
      lex();
      auto b = atomicTerm();
      defaultType(type::Individual, a);
      requireType(typeof(a), b);
      return intern(term::Not, intern(term::Eq, a, b));
    }
    }
    requireType(type::Bool, a);
    return a;
  }

  term quantifiedFormula(term op) {
    lex();
    expect('[');
    auto old = vars.n;
    vec<term> v(1);
    do {
      if (tok != o_var)
        err("expected variable");
      auto s = tokSym;
      lex();
      auto t = type::Individual;
      if (eat(':'))
        t = atomicType();
      auto x = var(t, vars.n);
      vars.push_back(make_pair(s, x));
      v.push_back(x);
    } while (eat(','));
    expect(']');
    expect(':');
    v[0] = unitaryFormula();
    vars.n = old;
    return intern(op, v);
  }

  term unitaryFormula() {
    switch (tok) {
    case '!':
      return quantifiedFormula(term::All);
    case '(': {
      lex();
      auto a = logicFormula();
      expect(')');
      return a;
    }
    case '?':
      return quantifiedFormula(term::Exists);
    case '~':
      lex();
      return intern(term::Not, unitaryFormula());
    }
    return infixUnary();
  }

  term associativeLogicFormula(term op, term a) {
    vec<term> v(1);
    v[0] = a;
    auto o = tok;
    while (eat(o))
      v.push_back(unitaryFormula());
    return intern(op, v);
  }

  term logicFormula() {
    auto a = unitaryFormula();
    switch (tok) {
    case '&':
      return associativeLogicFormula(term::And, a);
    case '|':
      return associativeLogicFormula(term::Or, a);
    case o_eqv:
      lex();
      return intern(term::Eqv, a, unitaryFormula());
    case o_imp:
      lex();
      return intern(term::Imp, a, unitaryFormula());
    case o_impr:
      lex();
      return intern(term::Imp, unitaryFormula(), a);
    case o_nand:
      lex();
      return intern(term::Not, intern(term::And, a, unitaryFormula()));
    case o_nor:
      lex();
      return intern(term::Not, intern(term::Or, a, unitaryFormula()));
    case o_xor:
      lex();
      return intern(term::Not, intern(term::Eqv, a, unitaryFormula()));
    }
    return a;
  }

  // top level
  sym *name() {
    switch (tok) {
    case o_int: {
      auto s = intern(tokStart, text - tokStart);
      lex();
      return s;
    }
    case o_word: {
      auto s = tokSym;
      lex();
      return s;
    }
    }
    err("expected name");
  }

  void ignore() {
    switch (tok) {
    case '(':
      lex();
      while (!eat(')'))
        ignore();
      return;
    case 0:
      err("unexpected end of file");
    }
    lex();
  }

  parser1(const char *file, const selection &sel) : parser(file), sel(sel) {
    try {
      lex();
      while (tok) {
        auto ts = tokStart;
        vars.n = 0;
        switch (keyword(name())) {
        case k_cnf: {
          expect('(');

          // name
          auto clauseName = name();
          expect(',');

          // role
          name();
          expect(',');

          // literals
          cnfMode = 1;
          neg.n = pos.n = 0;
          auto parens = eat('(');
          do {
            auto no = eat('~');
            auto a = infixUnary();
            ck(a);
            if (tag(a) == term::Not) {
              no = no ^ 1;
              a = at(a, 0);
            }
            (no ? neg : pos).push_back(a);
          } while (eat('|'));
          if (parens)
            expect(')');

          // select
          if (!sel.count(clauseName))
            break;

          // clause
          auto c = addClause(infer::none);
          clauseFiles[c] = file;
          clauseNames[c] = clauseName->v;
          break;
        }
        case k_fof:
        case k_tff: {
          expect('(');

          // name
          auto formulaName = name();
          expect(',');

          // role
          if (tok != o_word)
            err("expected role");
          auto role = keyword(tokSym);
          if (role == k_conjecture && conjecture)
            err("multiple conjectures not supported");
          lex();
          expect(',');

          // type
          if (role == k_type) {
            si parens = 0;
            while (eat('('))
              ++parens;
            auto s = name();
            expect(':');
            ts = tokStart;
            if (tok == o_dollarWord && tokSym == keywords + k_tType) {
              lex();
              if (tok == '>')
                throw inappropriate();
            } else {
              auto t = topLevelType();
              if (s->ft == type::none)
                s->ft = t;
              else if (t != s->ft)
                err("type mismatch");
            }
            while (parens--)
              expect(')');
            break;
          }

          // formula
          cnfMode = 0;
          auto a = logicFormula();
          assert(!vars.n);
          ck(a);

          // select
          if (!sel.count(formulaName))
            break;

          auto f = formula(infer::none, a);
          clauseFiles[f] = file;
          clauseNames[f] = formulaName->v;
          if (role == k_conjecture) {
            a = intern(term::Not, a);
            f = formula(infer::negate, a, f);
            conjecture = f;
          }

          // cnf
          cnf(a, f);
          break;
        }
        case k_include: {
          auto dir = getenv("TPTP");
          if (!dir)
            err("TPTP environment variable not set", ts);
          expect('(');

          // file
          snprintf(buf, sizeof buf, "%s/%s", dir, name()->v);
          auto file1 = intern(buf, strlen(buf))->v;

          // select and read
          if (eat(',')) {
            expect('[');
            selection sel1(0);
            do {
              auto selName = name();
              if (sel.count(selName))
                sel1.insert(selName);
            } while (eat(','));
            expect(']');
            parser1 p(file1, sel1);
          } else {
            parser1 p(file1, sel);
          }
          break;
        }
        default:
          err("unknown language", ts);
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

void tptp(const char *file) {
#ifdef DEBUG
  header = 2;
#endif
  parser1 p(file, selection(1));
}

namespace {
bool needParens(term a, term parent) {
  switch (tag(a)) {
  case term::And:
  case term::Eqv:
  case term::Imp:
  case term::Or:
    switch (tag(parent)) {
    case term::All:
    case term::And:
    case term::Eqv:
    case term::Exists:
    case term::Imp:
    case term::Not:
    case term::Or:
      return 1;
    }
    break;
  }
  return 0;
}

// SORT
void infix(const char *op, term a, term parent) {
  auto parens = needParens(a, parent);
  if (parens)
    putchar('(');
  for (si i = 0, n = size(a); i != n; ++i) {
    if (i)
      printf("%s", op);
    print(at(a, i), a);
  }
  if (parens)
    putchar(')');
}

void quant(char op, term a) {
  printf("%c[", op);
  for (si i = 1, n = size(a); i != n; ++i) {
    if (i > 1)
      putchar(',');
    auto x = at(a, i);
    print(x);
    auto t = varType(x);
    if (t != type::Individual) {
      putchar(':');
      printType(t);
    }
  }
  printf("]:");
  print(at(a, 0), a);
}

bool weird(const char *s) {
  if (!isLower(*s))
    return 1;
  do
    if (!isWord[*s])
      return 1;
  while (*++s);
  return 0;
}
///
} // namespace

void printType(type t) {
  switch (t) {
  case type::Bool:
    printf("$o");
    return;
  case type::Individual:
    printf("$i");
    return;
  case type::Int:
    printf("$int");
    return;
  case type::Rat:
    printf("$rat");
    return;
  case type::Real:
    printf("$real");
    return;
  }
  unreachable;
}

void print(term a, term parent) {
  switch (tag(a)) {
  case term::Add:
    printf("$sum");
    break;
  case term::All:
    quant('!', a);
    return;
  case term::And:
    infix(" & ", a, parent);
    return;
  case term::Call:
    print(at(a, 0), a);
    putchar('(');
    assert(size(a) > 1);
    for (si i = 1, n = size(a); i != n; ++i) {
      if (i > 1)
        putchar(',');
      print(at(a, i), a);
    }
    putchar(')');
    return;
  case term::Ceil:
    printf("$ceiling");
    break;
  case term::DistinctObj:
    quote('"', ((sym *)rest(a))->v);
    return;
  case term::Div:
    printf("$quotient");
    break;
  case term::DivE:
    printf("$quotient_e");
    break;
  case term::DivF:
    printf("$quotient_f");
    break;
  case term::DivT:
    printf("$quotient_t");
    break;
  case term::Eq:
    infix("=", a, parent);
    return;
  case term::Eqv:
    infix(" <=> ", a, parent);
    return;
  case term::Exists:
    quant('?', a);
    return;
  case term::False:
    printf("$false");
    break;
  case term::Floor:
    printf("$floor");
    break;
  case term::Imp:
    infix(" => ", a, parent);
    return;
  case term::Int:
    mpz_out_str(stdout, 10, ((Int *)rest(a))->val);
    return;
  case term::IsInt:
    printf("$is_int");
    break;
  case term::IsRat:
    printf("$is_rat");
    break;
  case term::Le:
    printf("$lesseq");
    break;
  case term::Lt:
    printf("$less");
    break;
  case term::Minus:
    printf("$uminus");
    break;
  case term::Mul:
    printf("$product");
    break;
  case term::Not:
    putchar('~');
    print(at(a, 0), a);
    return;
  case term::Or:
    infix(" | ", a, parent);
    return;
  case term::Rat:
    mpq_out_str(stdout, 10, ((Rat *)rest(a))->val);
    if (!mpz_cmp_ui(mpq_denref(((Rat *)rest(a))->val), 1))
      printf("/1");
    return;
  case term::Real:
    printf("%f", mpq_get_d(((Rat *)rest(a))->val));
    return;
  case term::RemE:
    printf("$remainder_e");
    break;
  case term::RemF:
    printf("$remainder_f");
    break;
  case term::RemT:
    printf("$remainder_t");
    break;
  case term::Round:
    printf("$round");
    break;
  case term::Sub:
    printf("$difference");
    break;
  case term::Sym: {
    auto s = ((sym *)rest(a))->v;
    if (weird(s)) {
      quote('\'', s);
      return;
    }
    printf("%s", s);
    return;
  }
  case term::ToInt:
    printf("$to_int");
    break;
  case term::ToRat:
    printf("$to_rat");
    break;
  case term::ToReal:
    printf("$to_real");
    break;
  case term::True:
    printf("$true");
    break;
  case term::Trunc:
    printf("$truncate");
    break;
  case term::Var: {
    auto i = vari(a);
    if (i < 26) {
      putchar('A' + i);
      return;
    }
    printf("Z%zu", i - 25);
    return;
  }
  default:
    unreachable;
  }
  putchar('(');
  for (si i = 0, n = size(a); i != n; ++i) {
    if (i)
      putchar(',');
    print(at(a, i), a);
  }
  putchar(')');
}

void printClause(clause *c) {
  printf(c->fof ? "fof" : "cnf");

  // name
  printf("(%s, ", clauseName(c));

  // role
  if (c == conjecture)
    printf("conjecture");
  else if (c->inf == infer::negate)
    printf("negated_conjecture");
  else
    printf("plain");
  printf(", ");

  // literals
  for (si i = 0, n = c->n; i != n; ++i) {
    if (i)
      printf(" | ");
    if (i < c->nn)
      putchar('~');
    print(c->v[i]);
  }
  printf(", ");

  // source
  auto file = clauseFiles[c];
  if (file) {
    printf("file(");
    quote('\'', basename(file));
    printf(",%s", clauseName(c));
  } else if (*c->from) {
    printf("inference(%s,[status(", inferNames[(si)c->inf]);
    if (*c->from == conjecture)
      printf("ceq");
    else
      printf("thm");
    printf(")],[%s", clauseName(*c->from));
    if (c->from[1])
      printf(",%s", clauseName(c->from[1]));
    putchar(']');
  } else
    printf("introduced(definition");
  puts(")).");
}
