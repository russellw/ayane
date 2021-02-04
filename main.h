#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <utility>

#ifdef DEBUG
#include <regex>
#include <string>
#endif

#include <gmp.h>

// general
#include "etc.h"
#include "mem.h"

// containers
#include "ary.h"
#include "vec.h"

// data
#include "data.h"
#include "keywords.h"

// parsers
#include "parser.h"

#include "dimacs.h"
#include "tptp.h"

// algorithms
#include "types.h"
#include "unify.h"

// unit tests
#include "test.h"
