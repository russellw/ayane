#include "main.h"

namespace {
enum {
  o_num = 1,
  o_zero,
};

struct DimacsParser : Parser {
  void lex() {
  loop:
    auto s = text;
    switch (*text) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      text = s + 1;
      goto loop;
    case 'c': {
      text = strchr(s, '\n');
#ifdef DEBUG
      std::string line(s, text);
      std::smatch m;
      if (!status &&
          std::regex_match(line, m, std::regex(R"(c .* (SAT|UNSAT) .*)")))
        status = m[1] == "SAT" ? Satisfiable : Unsatisfiable;
#endif
      goto loop;
    }
    case '0':
      if (!('0' <= s[1] && s[1] <= '9')) {
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
      while (isdigit(*s));
      tokSym = intern(text, s - text);
      text = s;
      tok = o_num;
      return;
    case 0:
      tok = 0;
      return;
    }
    text = s + 1;
    tok = *s;
  }

  w num() {
    auto a = fn(t_bool, tokSym);
    lex();
    return a;
  }

  DimacsParser(const char *file) : Parser(file) {
    try {
      lex();
      if (tok == 'p') {
        while (isspace(*text))
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
          neg.push(num());
          break;
        case o_num:
          pos.push(num());
          break;
        case o_zero:
          clause();
          lex();
          break;
        case 0:
          if (neg.n | pos.n)
            clause();
          return;
        default:
          throw "Syntax error";
        }
    } catch (const char *e) {
      err(e);
    }
  }
};
} // namespace

void readDimacs(const char *file) { DimacsParser parser(file); }
