#include "stdafx.h"
// stdafx.h must be first
#include "main.h"

const char *szs[] = {
    0,
#define _(s) #s,
#include "szs.h"
#undef _
};

bool complete;

#ifdef DEBUG
w status;
#endif

void init_problem() {
  complete = 1;
#ifdef DEBUG
  status = 0;
#endif
}
