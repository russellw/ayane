#include "main.h"

namespace {
enum {
  o_num = 1,
  o_zero,
};

struct DimacsParser : Parser {
  // Tokenizer
  void lex() {
  loop:
    auto s = tokstart = text;
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
      if (!isdigit1(s[1])) {
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
      while (isdigit1(*s));
      text = s;
      toksym = intern(tokstart, s - tokstart);
      tok = o_num;
      return;
    case 'c': {
      text = strchr(s, '\n');
#ifdef DEBUG
      std::string line(s, text);
      std::smatch m;
      if (!status &&
          std::regex_match(line, m, std::regex(R"(c .* (SAT|UNSAT) .*)")))
        status = m[1] == "SAT" ? s_Satisfiable : s_Unsatisfiable;
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

  // Terms
  w fn() {
    auto S = toksym;
    lex();
    S->ft = t_bool;
    return tag(S, a_sym);
  }

  // Top level
  DimacsParser(const char *file) : Parser(file) {
    try {
      lex();
      if (tok == 'p') {
        while (isspace1(*text))
          ++text;

        if (!(text[0] == 'c' && text[1] == 'n' && text[2] == 'f'))
          throw "Expected 'cnf'";
        text += 3;
        lex();

        if (tok != o_num)
          throw "Expected count";
        lex();

        if (tok != o_num)
          throw "Expected count";
        lex();
      }
      for (;;)
        switch (tok) {
        case '-':
          neg.push(fn());
          break;
        case 0:
          if (neg.n | pos.n)
            clause();
          return;
        case o_num:
          pos.push(fn());
          break;
        case o_zero:
          clause();
          lex();
          break;
        default:
          throw "Syntax error";
        }
    } catch (const char *e) {
      err(e);
    }
  }
};
} // namespace

void dimacs(const char *file) { DimacsParser parser(file); }
