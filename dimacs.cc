#include "main.h"

namespace {
enum {
  k_num = 1,
  k_zero,
};

void lex() {
loop:
  auto s = src;
  switch (*src) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    src = s + 1;
    goto loop;
  case 'c': {
    src = strchr(s, '\n');
#ifdef DEBUG
    std::string line(s, src);
    std::smatch m;
    if (!status &&
        std::regex_match(line, m, std::regex(R"(c .* (SAT|UNSAT) .*)")))
      status = m[1] == "SAT" ? Satisfiable : Unsatisfiable;
#endif
    goto loop;
  }
  case '0':
    if (!('0' <= s[1] && s[1] <= '9')) {
      src = s + 1;
      tok = k_zero;
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
    toksym = intern(src, s - src);
    src = s;
    tok = k_num;
    return;
  case 0:
    tok = 0;
    return;
  }
  src = s + 1;
  tok = *s;
}

w num() {
  auto a = fn(t_bool, toksym);
  lex();
  return a;
}
} // namespace

void readDimacs(const char *file) {
  File F(file);
  lex();
  if (tok == 'p') {
    while (isspace(*src))
      ++src;

    if (!(src[0] == 'c' && src[1] == 'n' && src[2] == 'f'))
      err("expected 'cnf'");
    src += 3;
    lex();

    if (tok != k_num)
      err("expected count");
    lex();

    if (tok != k_num)
      err("expected count");
    lex();
  }
  for (;;)
    switch (tok) {
    case '-':
      neg.push(num());
      break;
    case k_num:
      pos.push(num());
      break;
    case k_zero:
      clause();
      lex();
      break;
    case 0:
      if (neg.n | pos.n)
        clause();
      return;
    default:
      err("syntax error");
    }
}
