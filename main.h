#ifdef _MSC_VER
// not using exceptions
#pragma warning(disable : 4530)
#endif

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
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
#include "banks.h"
#include "vec.h"

// specific
#include "sym.h"

#include "keywords.h"
#include "parsing.h"

// unit tests
#include "test.h"
