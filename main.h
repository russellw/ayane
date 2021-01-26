#ifdef _MSC_VER
// not using exceptions
#pragma warning(disable : 4530)
#endif

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <utility>

#include <gmp.h>

// general
#include "etc.h"
#include "mem.h"

// containers
#include "bank.h"
#include "vec.h"

// specific
#include "type.h"

#include "parsing.h"
#include "sym.h"
#include "term.h"
#include "unify.h"

#include "keywords.h"

// unit tests
#include "test.h"
