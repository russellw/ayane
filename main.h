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

// General
#include "etc.h"
#include "mem.h"

// Containers
#include "ary.h"
#include "bank.h"
#include "vec.h"

// Data
#include "data.h"

#include "clause.h"
#include "sym.h"
#include "term.h"

#include "keywords.h"

// Parsers
#include "parser.h"

#include "dimacs.h"
#include "tptp.h"

// Algorithms
#include "typeof.h"
#include "unify.h"

// Unit tests
#include "test.h"
