struct parser {
  // File
  const char *file;
  const char *textstart;

  // Current location
  const char *text;

  // Current token
  const char *tokstart;
  w tok;
  Sym *toksym;

  parser(const char *file);
  ~parser();

  noret err(const char *msg, const char *ts);
  noret err(const char *msg);
};
