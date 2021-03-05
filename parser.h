struct parser {
  // file
  const char *file;
  const char *textstart;

  // current location
  const char *text;

  // current token
  const char *tokstart;
  int tok;
  sym *toksym;

  parser(const char *file);
  ~parser();

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};
