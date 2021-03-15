struct parser {
  // file
  const char *file;
  const char *textStart;

  // current location
  const char *text;

  // current token
  const char *tokStart;
  si tok;
  sym *tokSym;

  parser(const char *file);
  ~parser();

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};
