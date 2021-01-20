#include "main.h"

#ifdef DEBUG
namespace {
void test_gmp() { assert(0); }
} // namespace

void test() { test_gmp(); }
#endif
