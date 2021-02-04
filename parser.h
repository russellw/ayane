struct Parser {
  // File
  const char *file;
  const char *textStart;

  // Current location
  const char *text;

  // Current token
  const char *tokStart;
  w tok;
  Sym *tokSym;

  Parser(const char *file);
  ~Parser();

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};
