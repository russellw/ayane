#include "stdafx.h"
// stdafx.h must be first
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
si header;
#endif

struct init {
  init() {
    memset(isword + '0', 1, 10);
    memset(isword + 'A', 1, 26);
    isword['_'] = 1;
    memset(isword + 'a', 1, 26);
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
  bool cnfmode;
  selection sel;
  vec<pair<sym *, w>> vars;
  ///

  // tokenizer
  void word() {
    auto s = text;
    while (isword[*s])
      ++s;
    toksym = intern(text, s - text);
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
    toksym = intern(buf, i);
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
    // gmp doesn't handle unary +, so need to omit it from token
    if (*tokstart == '+')
      ++tokstart;
    // sign without digits should give a clear error message
    if (!isdigit1(*text))
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
    if (text - tokstart > sizeof buf - 1)
      err("number too long");
  }

  void lex() {
  loop:
    auto s = tokstart = text;
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
      if (expected == szs::none) {
        string s1(s, text);
        smatch m;
        if (regex_match(s1, m, regex(R"(% Status\s*:\s*(\w+)\s*)"))) {
          for (si i = 1; i != (si)szs::max; ++i)
            if (m[1] == szsnames[i]) {
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
        tokstart = s + 2;
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
  w atomic_type() {
    auto k = tok;
    auto s = toksym;
    auto ts = tokstart;
    lex();
    switch (k) {
    case '!':
    case '[':
      throw inappropriate();
    case '(': {
      auto t = atomic_type();
      expect(')');
      return t;
    }
    case o_dollarword:
      switch (keyword(s)) {
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
      throw inappropriate();
    case o_word:
      return mktype(s);
    default:
      err("expected type", ts);
    }
  }

  w top_level_type() {
    if (eat('(')) {
      vec<uint16_t> v(0);
      do
        v.push_back(atomic_type());
      while (eat('*'));
      expect(')');
      expect('>');
      v[0] = atomic_type();
      return mktype(v);
    }
    auto t = atomic_type();
    if (eat('>'))
      return mktype(atomic_type(), t);
    return t;
  }

  // terms
  w parse_int() {
    strmemcpy(buf, tokstart, text);
    Int x;
    if (mpz_init_set_str(x.val, buf, 10))
      err("invalid number");
    lex();
    return int1(x);
  }

  w parse_rat() {
    strmemcpy(buf, tokstart, text);
    Rat x;
    mpq_init(x.val);
    if (mpq_set_str(x.val, buf, 10))
      err("invalid number");
    mpq_canonicalize(x.val);
    lex();
    return rat(x);
  }

  w parse_real() {
    auto p = tokstart;

    // sign
    bool sign = 0;
    if (*p == '-') {
      ++p;
      sign = 1;
    }

    // integer part
    auto q = p;
    while (isdigit1(*q))
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
      while (isdigit1(*q))
        ++q;
      strmemcpy(buf, p, q);
      mpz_set_str(mantissa, buf, 10);
      scale = q - p;
      p = q;
    }
    mpz_t powscale;
    mpz_init(powscale);
    mpz_ui_pow_ui(powscale, 10, scale);

    // mantissa += integer * 10^scale
    mpz_addmul(mantissa, integer, powscale);

    // sign
    if (sign)
      mpz_neg(mantissa, mantissa);

    // result = scaled mantissa
    Rat x;
    mpq_init(x.val);
    mpq_set_num(x.val, mantissa);
    mpq_set_den(x.val, powscale);

    // exponent
    bool exponentsign = 0;
    si exponent = 0;
    if (*p == 'e' || *p == 'e') {
      ++p;
      switch (*p) {
      case '-':
        exponentsign = 1;
      case '+':
        ++p;
        break;
      }
      errno = 0;
      exponent = strtoul(p, 0, 10);
      if (errno)
        err(strerror(errno));
    }
    mpz_t powexponent;
    mpz_init(powexponent);
    mpz_ui_pow_ui(powexponent, 10, exponent);
    if (exponentsign)
      mpz_mul(mpq_denref(x.val), mpq_denref(x.val), powexponent);
    else
      mpz_mul(mpq_numref(x.val), mpq_numref(x.val), powexponent);

    // cleanup
    mpz_clear(integer);
    mpz_clear(mantissa);
    mpz_clear(powscale);
    mpz_clear(powexponent);

    // result
    lex();
    mpq_canonicalize(x.val);
    return real(x);
  }

  void args(vec<w> &v) {
    expect('(');
    do
      v.push_back(atomic_term());
    while (eat(','));
    expect(')');
  }

  void args(vec<w> &v, si arity) {
    auto old = v.n;
    args(v);
    if (v.n - old == arity)
      return;
    sprintf(buf, "expected %zu arguments", arity);
    err(buf);
  }

  w defined_functor(w op, si arity) {
    vec<w> v(op);
    args(v, arity);
    auto t = numtype(v[1]);
    for (auto i = v.p + 2, e = v.end(); i != e; ++i)
      requiretype(t, *i);
    return mk(v);
  }

  w atomic_term() {
    switch (tok) {
    case '!':
      throw inappropriate();
    case o_distinctobj: {
      auto a = tag(toksym, a_distinctobj);
      lex();
      return a;
    }
    case o_dollarword: {
      auto s = toksym;
      auto ts = tokstart;
      lex();
      vec<w> v;
      switch (keyword(s)) {
      case k_ceiling:
        return defined_functor(basic(b_ceil), 1);
      case k_difference:
        return defined_functor(basic(b_sub), 2);
      case k_distinct: {
        args(v);
        defaulttype(t_individual, v[0]);
        auto t = typeof(v[0]);
        for (auto i = v.p + 1, e = v.end(); i != e; ++i)
          requiretype(t, *i);
        vec<w> clauses(basic(b_and));
        for (auto i = v.p, e = v.end(); i != e; ++i)
          for (auto j = v.p; j != i; ++j)
            clauses.push_back(mk(basic(b_not), mk(basic(b_eq), *i, *j)));
        return mk(clauses);
      }
      case k_false:
        return basic(b_false);
      case k_floor:
        return defined_functor(basic(b_floor), 1);
      case k_greater: {
        args(v, 2);
        auto t = numtype(v[0]);
        requiretype(t, v[1]);
        return mk(basic(b_lt), v[1], v[0]);
      }
      case k_greatereq: {
        args(v, 2);
        auto t = numtype(v[0]);
        requiretype(t, v[1]);
        return mk(basic(b_le), v[1], v[0]);
      }
      case k_is_int:
        return defined_functor(basic(b_isint), 1);
      case k_is_rat:
        return defined_functor(basic(b_israt), 1);
      case k_ite:
        throw inappropriate();
      case k_less:
        return defined_functor(basic(b_lt), 2);
      case k_lesseq:
        return defined_functor(basic(b_le), 2);
      case k_product:
        return defined_functor(basic(b_mul), 2);
      case k_quotient: {
        auto a = defined_functor(basic(b_div), 2);
        if (typeof(at(a, 1)) == t_int)
          err("expected fraction term");
        return a;
      }
      case k_quotient_e:
        return defined_functor(basic(b_dive), 2);
      case k_quotient_f:
        return defined_functor(basic(b_divf), 2);
      case k_quotient_t:
        return defined_functor(basic(b_divt), 2);
      case k_remainder_e:
        return defined_functor(basic(b_reme), 2);
      case k_remainder_f:
        return defined_functor(basic(b_remf), 2);
      case k_remainder_t:
        return defined_functor(basic(b_remt), 2);
      case k_round:
        return defined_functor(basic(b_round), 1);
      case k_sum:
        return defined_functor(basic(b_add), 2);
      case k_to_int:
        return defined_functor(basic(b_toint), 1);
      case k_to_rat:
        return defined_functor(basic(b_torat), 1);
      case k_to_real:
        return defined_functor(basic(b_toreal), 1);
      case k_true:
        return basic(b_true);
      case k_truncate:
        return defined_functor(basic(b_trunc), 1);
      case k_uminus:
        return defined_functor(basic(b_minus), 1);
      }
      err("unknown word", ts);
    }
    case o_int:
      return parse_int();
    case o_rat:
      return parse_rat();
    case o_real:
      return parse_real();
    case o_var: {
      auto s = toksym;
      auto ts = tokstart;
      lex();
      for (auto i = vars.rbegin(); i != vars.rend(); ++i)
        if (i->first == s)
          return i->second;
      if (!cnfmode)
        err("unknown variable", ts);
      auto x = var(t_individual, vars.n);
      vars.push_back(make_pair(s, x));
      return x;
    }
    case o_word: {
      auto a = tag(toksym, a_sym);
      lex();
      if (tok != '(')
        return a;
      vec<w> v(a);
      args(v);
      for (auto i = v.p + 1, e = v.end(); i != e; ++i)
        defaulttype(t_individual, *i);
      return mk(v);
    }
    }
    err("syntax error");
  }

  w infix_unary() {
    auto a = atomic_term();
    switch (tok) {
    case '=': {
      lex();
      auto b = atomic_term();
      defaulttype(t_individual, a);
      requiretype(typeof(a), b);
      return mk(basic(b_eq), a, b);
    }
    case o_ne: {
      lex();
      auto b = atomic_term();
      defaulttype(t_individual, a);
      requiretype(typeof(a), b);
      return mk(basic(b_not), mk(basic(b_eq), a, b));
    }
    }
    requiretype(t_bool, a);
    return a;
  }

  w quantified_formula(w op) {
    lex();
    expect('[');
    auto old = vars.n;
    vec<w> v(op, 0);
    do {
      if (tok != o_var)
        err("expected variable");
      auto s = toksym;
      lex();
      w t = t_individual;
      if (eat(':'))
        t = atomic_type();
      auto x = var(t, vars.n);
      vars.push_back(make_pair(s, x));
      v.push_back(x);
    } while (eat(','));
    expect(']');
    expect(':');
    v[1] = unitary_formula();
    vars.n = old;
    return mk(v);
  }

  w unitary_formula() {
    switch (tok) {
    case '!':
      return quantified_formula(basic(b_all));
    case '(': {
      lex();
      auto a = logic_formula();
      expect(')');
      return a;
    }
    case '?':
      return quantified_formula(basic(b_exists));
    case '~':
      lex();
      return mk(basic(b_not), unitary_formula());
    }
    return infix_unary();
  }

  w associative_logic_formula(w op, w a) {
    vec<w> v(op, a);
    auto o = tok;
    while (eat(o))
      v.push_back(unitary_formula());
    return mk(v);
  }

  w logic_formula() {
    auto a = unitary_formula();
    switch (tok) {
    case '&':
      return associative_logic_formula(basic(b_and), a);
    case '|':
      return associative_logic_formula(basic(b_or), a);
    case o_eqv:
      lex();
      return mk(basic(b_eqv), a, unitary_formula());
    case o_imp:
      lex();
      return imp(a, unitary_formula());
    case o_impr:
      lex();
      return imp(unitary_formula(), a);
    case o_nand:
      lex();
      return mk(basic(b_not), mk(basic(b_and), a, unitary_formula()));
    case o_nor:
      lex();
      return mk(basic(b_not), mk(basic(b_or), a, unitary_formula()));
    case o_xor:
      lex();
      return mk(basic(b_not), mk(basic(b_eqv), a, unitary_formula()));
    }
    return a;
  }

  // top level
  sym *name() {
    switch (tok) {
    case o_int: {
      auto s = intern(tokstart, text - tokstart);
      lex();
      return s;
    }
    case o_word: {
      auto s = toksym;
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
        auto ts = tokstart;
        vars.n = 0;
        switch (keyword(name())) {
        case k_cnf: {
          expect('(');

          // name
          auto forname = name();
          expect(',');

          // role
          name();
          expect(',');

          // literals
          cnfmode = 1;
          neg.n = pos.n = 0;
          auto parens = eat('(');
          do {
            auto no = eat('~');
            auto a = infix_unary();
            ckterm(a);
            if ((a & 7) == a_compound && at(a, 0) == basic(b_not)) {
              no = !no;
              a = at(a, 1);
            }
            (no ? neg : pos).push(a);
          } while (eat('|'));
          if (parens)
            expect(')');

          // select
          if (!sel.count(forname))
            break;

          // clause
          auto c = addclause(infer::none);
          clausefiles[c] = file;
          clausenames[c] = forname->v;
          break;
        }
        case k_fof:
        case k_tff: {
          expect('(');

          // name
          auto forname = name();
          expect(',');

          // role
          if (tok != o_word)
            err("expected role");
          auto role = keyword(toksym);
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
            ts = tokstart;
            if (tok == o_dollarword && toksym == keywords + k_tType) {
              lex();
              if (tok == '>')
                throw inappropriate();
            } else {
              auto t = top_level_type();
              if (s->ft) {
                if (t != s->ft)
                  err("type mismatch");
              } else
                s->ft = t;
            }
            while (parens--)
              expect(')');
            break;
          }

          // formula
          cnfmode = 0;
          auto a = logic_formula();
          assert(!vars.n);
          ckterm(a);

          // select
          if (!sel.count(forname))
            break;

          auto f = formula(infer::none, a);
          clausefiles[f] = file;
          clausenames[f] = forname->v;
          if (role == k_conjecture) {
            a = mk(basic(b_not), a);
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
              auto forname = name();
              if (sel.count(forname))
                sel1.insert(forname);
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
bool needparens(w a, w parent) {
  if (!parent)
    return 0;
  auto op = at(a, 0);
  if ((op & 7) != a_basic)
    return 0;
  switch (op >> 3) {
  case b_and:
  case b_eqv:
  case b_or: {
    auto parentop = at(parent, 0);
    if ((parentop & 7) != a_basic)
      return 0;
    switch (parentop >> 3) {
    case b_all:
    case b_and:
    case b_eqv:
    case b_exists:
    case b_not:
    case b_or:
      return 1;
    }
    return 0;
  }
  }
  return 0;
}

// SORT
void infix(const char *op, w a, w parent) {
  auto parens = needparens(a, parent);
  if (parens)
    putchar('(');
  for (si i = 1, n = size(a); i != n; ++i) {
    if (i > 1)
      printf("%s", op);
    prterm(at(a, i), a);
  }
  if (parens)
    putchar(')');
}

void quant(char op, w a) {
  printf("%c[", op);
  for (si i = 2, n = size(a); i != n; ++i) {
    if (i > 2)
      putchar(',');
    auto x = at(a, i);
    prterm(x);
    auto t = vartype(x);
    if (t != t_individual) {
      putchar(':');
      prtype(t);
    }
  }
  printf("]:");
  prterm(at(a, 1), a);
}

bool weird(const char *s) {
  if (!islower1(*s))
    return 1;
  do
    if (!isword[*s])
      return 1;
  while (*++s);
  return 0;
}
///
} // namespace

void prtype(w t) {
  switch (t) {
  case t_bool:
    printf("$o");
    return;
  case t_individual:
    printf("$i");
    return;
  case t_int:
    printf("$int");
    return;
  case t_rat:
    printf("$rat");
    return;
  case t_real:
    printf("$real");
    return;
  }
  unreachable;
}

void prterm(w a, w parent) {
  switch (a & 7) {
  case a_basic:
    switch (a >> 3) {
    case b_add:
      printf("$sum");
      return;
    case b_ceil:
      printf("$ceiling");
      return;
    case b_div:
      printf("$quotient");
      return;
    case b_dive:
      printf("$quotient_e");
      return;
    case b_divf:
      printf("$quotient_f");
      return;
    case b_divt:
      printf("$quotient_t");
      return;
    case b_false:
      printf("$false");
      return;
    case b_floor:
      printf("$floor");
      return;
    case b_isint:
      printf("$is_int");
      return;
    case b_israt:
      printf("$is_rat");
      return;
    case b_le:
      printf("$lesseq");
      return;
    case b_lt:
      printf("$less");
      return;
    case b_minus:
      printf("$uminus");
      return;
    case b_mul:
      printf("$product");
      return;
    case b_reme:
      printf("$remainder_e");
      return;
    case b_remf:
      printf("$remainder_f");
      return;
    case b_remt:
      printf("$remainder_t");
      return;
    case b_round:
      printf("$round");
      return;
    case b_sub:
      printf("$difference");
      return;
    case b_toint:
      printf("$to_int");
      return;
    case b_torat:
      printf("$to_rat");
      return;
    case b_toreal:
      printf("$to_real");
      return;
    case b_true:
      printf("$true");
      return;
    case b_trunc:
      printf("$truncate");
      return;
    }
    break;
  case a_compound: {
    auto op = at(a, 0);
    if ((op & 7) == a_basic)
      switch (op >> 3) {
      case b_all:
        quant('!', a);
        return;
      case b_and:
        infix(" & ", a, parent);
        return;
      case b_eq:
        infix("=", a, parent);
        return;
      case b_eqv:
        infix(" <=> ", a, parent);
        return;
      case b_exists:
        quant('?', a);
        return;
      case b_not:
        putchar('~');
        prterm(at(a, 1), a);
        return;
      case b_or:
        infix(" | ", a, parent);
        return;
      }
    prterm(at(a, 0), a);
    putchar('(');
    for (si i = 1, n = size(a); i != n; ++i) {
      if (i > 1)
        putchar(',');
      prterm(at(a, i), a);
    }
    putchar(')');
    return;
  }
  case a_distinctobj:
    quote('"', distinctobjp(a)->v);
    return;
  case a_int:
    mpz_out_str(stdout, 10, intp(a)->val);
    return;
  case a_rat:
    mpq_out_str(stdout, 10, ratp(a)->val);
    if (!mpz_cmp_ui(mpq_denref(ratp(a)->val), 1))
      printf("/1");
    return;
  case a_real:
    printf("%f", mpq_get_d(ratp(a)->val));
    return;
  case a_sym: {
    auto s = symp(a)->v;
    if (weird(s)) {
      quote('\'', s);
      return;
    }
    printf("%s", s);
    return;
  }
  case a_var: {
    auto i = vari(a);
    if (i < 26) {
      putchar('A' + i);
      return;
    }
    printf("Z%zu", i - 25);
    return;
  }
  }
  unreachable;
}

void prclause(clause *c) {
  printf(c->fof ? "fof" : "cnf");

  // name
  printf("(%s, ", clausename(c));

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
    prterm(c->v[i]);
  }
  printf(", ");

  // source
  auto file = clausefiles[c];
  if (file) {
    printf("file(");
    quote('\'', basename(file));
    printf(",%s", clausename(c));
  } else if (*c->from) {
    printf("inference(%s,[status(", infernames[(si)c->inf]);
    if (*c->from == conjecture)
      printf("ceq");
    else
      printf("thm");
    printf(")],[%s", clausename(*c->from));
    if (c->from[1])
      printf(",%s", clausename(c->from[1]));
    putchar(']');
  } else
    printf("introduced(definition");
  puts(")).");
}
