#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
using std::find;
using std::max;
using std::min;

#include <iterator>
// using std::reverse_iterator;
// not actually using'd because it is used in container contexts where it has to
// be prefixed with std:: to disambiguate from a local use of the name anyway

#include <queue>
using std::priority_queue;

#include <unordered_map>
using std::unordered_map;

#include <unordered_set>
using std::unordered_set;

#include <utility>
using std::make_pair;
using std::pair;

#include <vector>
using std::vector;

#ifdef DEBUG
#include <regex>
using std::regex;
using std::regex_match;
using std::smatch;

#include <string>
using std::string;
#endif

#include <gmp.h>
