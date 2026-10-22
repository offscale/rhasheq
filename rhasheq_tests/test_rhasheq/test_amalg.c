/**
 * @file test_amalg.c
 * @brief Standalone compilation and execution test for the single-header
 * rhasheq_amalgamation.h.
 */

#define RHASHEQ_IMPLEMENTATION

/* clang-format off */
#include "rhasheq_amalgamation.h"
#include <config_for_tests.h>
/* clang-format on */

/**
 * @brief Helper to test amalgamated rhasheq_verify_file and crc32c.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
static rhasheq_error_t run_amalg_checks(void) {
  rhasheq_error_t rc;
  bool matched;

  matched = false;
  rc = crc32c(GREATEST_FILE, GREATEST_CRC32C, &matched);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  if (!matched) {
    return RHASHEQ_HASH_ERROR;
  }

  rc = crc32c(GREATEST_FILE, "00000000", &matched);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  if (matched) {
    return RHASHEQ_HASH_ERROR;
  }

  return RHASHEQ_SUCCESS;
}

/**
 * @brief Entry point for amalgamation test executable.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
  rhasheq_error_t rc;
  rc = run_amalg_checks();
  if (rc != RHASHEQ_SUCCESS) {
    return (int)rc;
  }
  return 0;
}
