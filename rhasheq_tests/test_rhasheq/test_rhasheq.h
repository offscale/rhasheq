/**
 * @file test_rhasheq.h
 * @brief Unit tests for rhasheq library covering all algorithms, branches, and
 * OOM conditions.
 */

#ifndef TEST_RHASHEQ_H
#define TEST_RHASHEQ_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <rhasheq.h>
#include <config_for_tests.h>
/* clang-format on */

/**
 * @brief Helper to compute the expected hex digest of a file for a given
 * hash_id.
 * @param hash_id LibRHash algorithm identifier.
 * @param filepath Path to file.
 * @param out_hex Output hex buffer (at least 130 bytes).
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
static rhasheq_error_t compute_expected_hex(enum rhash_ids hash_id,
                                            const char *filepath,
                                            char *out_hex) {
  char digest[RHASHEQ_DIGEST_RAW_SIZE];
  rhasheq_error_t rc;
  rc = hash_file(filepath, hash_id, RHPR_HEX, digest, out_hex);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

/**
 * @brief Tests rhasheq_malloc and rhasheq_free success, invalid arguments, and
 * OOM.
 * @return greatest test result.
 */
TEST x_rhasheq_malloc_and_free(void) {
  void *ptr;
  rhasheq_error_t rc;

  ptr = NULL;
  rc = rhasheq_malloc(0, &ptr);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = rhasheq_malloc(16, NULL);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  g_rhasheq_mock_malloc_fail = 1;
  rc = rhasheq_malloc(16, &ptr);
  g_rhasheq_mock_malloc_fail = 0;
  ASSERT_EQ(RHASHEQ_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ptr);

  g_rhasheq_mock_malloc_fail = -1;
  rc = rhasheq_malloc(16, &ptr);
  g_rhasheq_mock_malloc_fail = 0;
  ASSERT_EQ(RHASHEQ_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ptr);

  rc = rhasheq_malloc(16, &ptr);
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);
  ASSERT(ptr != NULL);
  rhasheq_free(ptr);
  rhasheq_free(NULL);
  PASS();
}

/**
 * @brief Tests rhasheq_format_error success, invalid arguments, and formatting
 * failure.
 * @return greatest test result.
 */
TEST x_rhasheq_format_error(void) {
  char buf[RHASHEQ_ERRBUF_SIZE];
  rhasheq_error_t rc;

  rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, ENOENT, NULL, sizeof(buf));
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, ENOENT, buf, 0);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  g_rhasheq_mock_format_fail = 1;
  rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, ENOENT, buf, sizeof(buf));
  g_rhasheq_mock_format_fail = 0;
  ASSERT_EQ(RHASHEQ_FORMAT_ERROR, rc);

  rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, ENOENT, buf, sizeof(buf));
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);
  ASSERT(strlen(buf) > 0);

  rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, -99999, buf, sizeof(buf));
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);

  PASS();
}

/**
 * @brief Tests rhasheq_init idempotency and mock failure.
 * @return greatest test result.
 */
TEST x_rhasheq_init(void) {
  rhasheq_error_t rc;
  rc = rhasheq_init();
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);
  rc = rhasheq_init();
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);

  g_rhasheq_mock_init_fail = 1;
  rc = rhasheq_init();
  g_rhasheq_mock_init_fail = 0;
  ASSERT_EQ(RHASHEQ_INIT_ERROR, rc);
  PASS();
}

/**
 * @brief Tests hash_file and rhasheq_verify_file argument validation, OOM
 * branches, rhash_file failure, and print failure.
 * @return greatest test result.
 */
TEST x_rhasheq_hash_file_errors_and_oom(void) {
  bool matched;
  char digest[RHASHEQ_DIGEST_RAW_SIZE];
  char gen_hash[RHASHEQ_DIGEST_HEX_SIZE];
  rhasheq_error_t rc;

  matched = false;
  /* Direct hash_file invalid arguments */
  rc = hash_file(NULL, RHASH_CRC32C, RHPR_HEX, digest, gen_hash);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = hash_file(GREATEST_FILE, (enum rhash_ids)0, RHPR_HEX, digest, gen_hash);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = hash_file(GREATEST_FILE, RHASH_CRC32C, RHPR_HEX, NULL, gen_hash);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = hash_file(GREATEST_FILE, RHASH_CRC32C, RHPR_HEX, digest, NULL);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  /* Direct hash_file init failure */
  g_rhasheq_mock_init_fail = 1;
  rc = hash_file(GREATEST_FILE, RHASH_CRC32C, RHPR_HEX, digest, gen_hash);
  g_rhasheq_mock_init_fail = 0;
  ASSERT_EQ(RHASHEQ_INIT_ERROR, rc);

  /* rhasheq_verify_file invalid arguments */
  rc = rhasheq_verify_file(GREATEST_FILE, (enum rhash_ids)0, GREATEST_CRC32C,
                           &matched);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = rhasheq_verify_file(NULL, RHASH_CRC32C, GREATEST_CRC32C, &matched);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, NULL, &matched);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C, NULL);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);

  /* OOM on first allocation (digest) */
  g_rhasheq_mock_malloc_fail = 1;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_malloc_fail = 0;
  ASSERT_EQ(RHASHEQ_OUT_OF_MEMORY, rc);

  /* OOM on second allocation (gen_hash) */
  g_rhasheq_mock_malloc_fail = 2;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_malloc_fail = 0;
  ASSERT_EQ(RHASHEQ_OUT_OF_MEMORY, rc);

  /* OOM on third allocation (err_buf inside hash_file when rhash_file fails) */
  g_rhasheq_mock_rhash_file_fail = 1;
  g_rhasheq_mock_malloc_fail = 3;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_malloc_fail = 0;
  g_rhasheq_mock_rhash_file_fail = 0;
  ASSERT_EQ(RHASHEQ_OUT_OF_MEMORY, rc);

  /* Format error when rhash_file fails */
  g_rhasheq_mock_rhash_file_fail = 1;
  g_rhasheq_mock_format_fail = 1;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_format_fail = 0;
  g_rhasheq_mock_rhash_file_fail = 0;
  ASSERT_EQ(RHASHEQ_FORMAT_ERROR, rc);

  /* Normal rhash_file failure (nonexistent file) */
  rc = rhasheq_verify_file("nonexistent_file_path_12345.tmp", RHASH_CRC32C,
                           GREATEST_CRC32C, &matched);
  ASSERT_EQ(RHASHEQ_HASH_ERROR, rc);
  ASSERT_EQ(false, matched);

  /* Mocked rhash_file failure */
  g_rhasheq_mock_rhash_file_fail = 1;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_rhash_file_fail = 0;
  ASSERT_EQ(RHASHEQ_HASH_ERROR, rc);
  ASSERT_EQ(false, matched);

  /* Mocked rhash_print_bytes failure */
  g_rhasheq_mock_rhash_print_fail = 1;
  rc = rhasheq_verify_file(GREATEST_FILE, RHASH_CRC32C, GREATEST_CRC32C,
                           &matched);
  g_rhasheq_mock_rhash_print_fail = 0;
  ASSERT_EQ(RHASHEQ_FORMAT_ERROR, rc);
  ASSERT_EQ(false, matched);

  PASS();
}

/**
 * @brief Tests crc32c match and mismatch.
 * @return greatest test result.
 */
TEST x_rhasheq_crc32c_should_succeed(void) {
  bool matched;
  rhasheq_error_t rc;

  matched = false;
  rc = crc32c(GREATEST_FILE, GREATEST_CRC32C, &matched);
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);
  ASSERT_EQ(true, matched);

  rc = crc32c(GREATEST_FILE, "00000000", &matched);
  ASSERT_EQ(RHASHEQ_SUCCESS, rc);
  ASSERT_EQ(false, matched);

  rc = crc32c(NULL, GREATEST_CRC32C, &matched);
  ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);
  PASS();
}

/**
 * @brief Macro to test both success (match + mismatch) and error percolation
 * for a specific algorithm wrapper.
 */
#define CHECK_HASH_ALGO(func, id)                                              \
  do {                                                                         \
    char expected[RHASHEQ_DIGEST_HEX_SIZE];                                    \
    bool matched;                                                              \
    rhasheq_error_t rc;                                                        \
    rc = compute_expected_hex((id), GREATEST_FILE, expected);                  \
    ASSERT_EQ(RHASHEQ_SUCCESS, rc);                                            \
    matched = false;                                                           \
    rc = func(GREATEST_FILE, expected, &matched);                              \
    ASSERT_EQ(RHASHEQ_SUCCESS, rc);                                            \
    ASSERT_EQ(true, matched);                                                  \
    rc = func(GREATEST_FILE, "invalid_hash", &matched);                        \
    ASSERT_EQ(RHASHEQ_SUCCESS, rc);                                            \
    ASSERT_EQ(false, matched);                                                 \
    rc = func(NULL, expected, &matched);                                       \
    ASSERT_EQ(RHASHEQ_INVALID_ARGUMENT, rc);                                   \
  } while (0)

/**
 * @brief Tests all 30 hash wrapper functions for both success and error
 * percolation.
 * @return greatest test result.
 */
TEST x_rhasheq_all_algorithms(void) {
  CHECK_HASH_ALGO(crc32, RHASH_CRC32);
  CHECK_HASH_ALGO(md4, RHASH_MD4);
  CHECK_HASH_ALGO(md5, RHASH_MD5);
  CHECK_HASH_ALGO(sha1, RHASH_SHA1);
  CHECK_HASH_ALGO(tiger, RHASH_TIGER);
  CHECK_HASH_ALGO(tth, RHASH_TTH);
  CHECK_HASH_ALGO(btih, RHASH_BTIH);
  CHECK_HASH_ALGO(ed2k, RHASH_ED2K);
  CHECK_HASH_ALGO(aich, RHASH_AICH);
  CHECK_HASH_ALGO(whirlpool, RHASH_WHIRLPOOL);
  CHECK_HASH_ALGO(ripemd160, RHASH_RIPEMD160);
  CHECK_HASH_ALGO(gost94, RHASH_GOST94);
  CHECK_HASH_ALGO(gost94_cryptopro, RHASH_GOST94_CRYPTOPRO);
  CHECK_HASH_ALGO(has160, RHASH_HAS160);
  CHECK_HASH_ALGO(gost12_256, RHASH_GOST12_256);
  CHECK_HASH_ALGO(gost12_512, RHASH_GOST12_512);
  CHECK_HASH_ALGO(sha224, RHASH_SHA224);
  CHECK_HASH_ALGO(sha256, RHASH_SHA256);
  CHECK_HASH_ALGO(sha384, RHASH_SHA384);
  CHECK_HASH_ALGO(sha512, RHASH_SHA512);
  CHECK_HASH_ALGO(edonr256, RHASH_EDONR256);
  CHECK_HASH_ALGO(edonr512, RHASH_EDONR512);
  CHECK_HASH_ALGO(sha3_224, RHASH_SHA3_224);
  CHECK_HASH_ALGO(sha3_256, RHASH_SHA3_256);
  CHECK_HASH_ALGO(sha3_384, RHASH_SHA3_384);
  CHECK_HASH_ALGO(sha3_512, RHASH_SHA3_512);
  CHECK_HASH_ALGO(crc32c, RHASH_CRC32C);
  CHECK_HASH_ALGO(snefru128, RHASH_SNEFRU128);
  CHECK_HASH_ALGO(snefru256, RHASH_SNEFRU256);
  CHECK_HASH_ALGO(blake2s, RHASH_BLAKE2S);
  CHECK_HASH_ALGO(blake2b, RHASH_BLAKE2B);
  PASS();
}

SUITE(rhasheq_suite) {
  RUN_TEST(x_rhasheq_malloc_and_free);
  RUN_TEST(x_rhasheq_format_error);
  RUN_TEST(x_rhasheq_init);
  RUN_TEST(x_rhasheq_hash_file_errors_and_oom);
  RUN_TEST(x_rhasheq_crc32c_should_succeed);
  RUN_TEST(x_rhasheq_all_algorithms);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_RHASHEQ_H */
