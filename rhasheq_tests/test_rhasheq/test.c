/**
 * @file test.c
 * @brief Main test runner for rhasheq unit tests.
 */

/* clang-format off */
#include <greatest.h>

#include "test_rhasheq.h"
/* clang-format on */

GREATEST_MAIN_DEFS();

/**
 * @brief Entry point for rhasheq test executable.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit status code.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(rhasheq_suite);
  GREATEST_MAIN_END();
}
