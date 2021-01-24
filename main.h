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
#include <unordered_map>
#include <utility>

#include <gmp.h>

typedef uint16_t ty;
typedef size_t term;

// general
#include "etc.h"
#include "mem.h"

// containers
#include "vec.h"

// specific
#include "sym.h"
#include "term.h"
#include "type.h"

#include "keywords.h"
#include "parsing.h"

// unit tests
#include "test.h"
