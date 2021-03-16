#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

namespace {
enum {
  o_num = 1,
  o_zero,
};

struct parser1 : parser {
  // tokenizer
  void lex() {
  loop:
    auto s = tokStart = text;
    switch (*text) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      text = s + 1;
      goto loop;
    case '0':
      if (!isDigit(s[1])) {
        text = s + 1;
        tok = o_zero;
        return;
      }
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
      text = s;
      tokSym = intern(tokStart, s - tokStart);
      tok = o_num;
      return;
    case 'c': {
      text = strchr(s, '\n');
#ifdef DEBUG
      string line(s, text);
      smatch m;
      if (expected == szs::none &&
          regex_match(line, m, regex(R"(c .* (SAT|UNSAT) .*)")))
        expected = m[1] == "SAT" ? szs::Satisfiable : szs::Unsatisfiable;
#endif
      goto loop;
    }
    case 0:
      tok = 0;
      return;
    }
    text = s + 1;
    tok = *s;
  }

  // terms
  term fn() {
    auto s = tokSym;
    lex();
    s->ft = type::Bool;
    return tag(term::Sym, s);
  }

  // top level
  parser1(const char *file) : parser(file) {
    try {
      lex();
      if (tok == 'p') {
        while (isSpace(*text))
          ++text;

        if (!(text[0] == 'c' && text[1] == 'n' && text[2] == 'f'))
          err("expected 'cnf'");
        text += 3;
        lex();

        if (tok != o_num)
          err("expected count");
        lex();

        if (tok != o_num)
          err("expected count");
        lex();
      }
      for (;;)
        switch (tok) {
        case '-':
          neg.push_back(fn());
          break;
        case 0:
          if (neg.n | pos.n)
            addClause(infer::none);
          return;
        case o_num:
          pos.push_back(fn());
          break;
        case o_zero:
          lex();
          addClause(infer::none);
          break;
        default:
          err("syntax error");
        }
    } catch (const char *e) {
      err(e);
    }
  }
};
} // namespace

void dimacs(const char *file) { parser1 p(file); }
