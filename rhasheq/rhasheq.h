/**
 * @file rhasheq.h
 * @brief LibRHash file checksum verification API with strict C89 error
 * percolation.
 *
 * Originally librhash implementation of libacquire's checksum API.
 */

#ifndef RHASH_EQ_H
#define RHASH_EQ_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(RHASHEQ_HEADER_ONLY_MODE) && !defined(RHASHEQ_IMPLEMENTATION)
#define RHASHEQ_IMPLEMENTATION
#endif

/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#elif !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) && !defined(__bool_true_false_are_defined)
#if defined(__has_include)
#if __has_include("rhasheq_stdbool.h")
#include "rhasheq_stdbool.h"
#endif
#elif !defined(RHASHEQ_SINGLE_HEADER)
#include "rhasheq_stdbool.h"
#endif
#endif

#if !defined(C_CI_NO_DISCARD_H) && !defined(NO_DISCARD)
#if defined(__has_include)
#if __has_include("no_discard.h")
#include "no_discard.h"
#endif
#elif !defined(RHASHEQ_SINGLE_HEADER)
#include "no_discard.h"
#endif
#endif

#include <rhash.h>

#if defined(RHASHEQ_USE_C89STRINGUTILS)
#include <c89stringutils_string_extras.h>
#endif
/* clang-format on */

#if !defined(__cplusplus) && !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) &&  \
    !defined(__bool_true_false_are_defined)
#define _STDBOOL_H
#define __bool_true_false_are_defined 1
/** @brief Boolean type for C89 compatibility. */
typedef size_t rhasheq_bool_t;
/** @brief Standard bool macro mapped to rhasheq_bool_t. */
#define bool rhasheq_bool_t
/** @brief Boolean true constant. */
#define true 1
/** @brief Boolean false constant. */
#define false 0
#endif

/**
 * @brief Attribute macro to warn when an error enum return value is discarded.
 */
#ifndef NO_DISCARD
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/** @brief C23 nodiscard attribute. */
#define NO_DISCARD [[nodiscard]]
#elif defined(__cplusplus) && __cplusplus >= 201703L
/** @brief C++17 nodiscard attribute. */
#define NO_DISCARD [[nodiscard]]
#elif defined(__clang__)
/** @brief Clang warn_unused_result attribute. */
#define NO_DISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
/** @brief MSVC SAL _Check_return_ annotation. */
#define NO_DISCARD _Check_return_
#else
/** @brief Fallback empty no-discard macro. */
#define NO_DISCARD
#endif
#endif /* !NO_DISCARD */

/**
 * @brief Export macro for shared and static library builds across platforms.
 */
#ifndef RHASHEQ_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(RHASHEQ_BUILD_SHARED)
#if defined(RHASHEQ_EXPORTS)
/** @brief Export symbol when building shared library on Windows/Cygwin. */
#define RHASHEQ_EXPORT __declspec(dllexport)
#else
/** @brief Import symbol when consuming shared library on Windows/Cygwin. */
#define RHASHEQ_EXPORT __declspec(dllimport)
#endif
#else
/** @brief Default empty export macro for static library on Windows/Cygwin. */
#define RHASHEQ_EXPORT
#endif
#elif defined(RHASHEQ_BUILD_SHARED) && defined(__GNUC__) && __GNUC__ >= 4
/** @brief Default visibility export macro for GCC/Clang shared library. */
#define RHASHEQ_EXPORT __attribute__((visibility("default")))
#else
/** @brief Default empty export macro. */
#define RHASHEQ_EXPORT
#endif
#endif /* !RHASHEQ_EXPORT */

/**
 * @brief Format specifier for 64-bit signed integers across MSVC and POSIX/C89.
 */
#ifndef NUM_FORMAT
#if defined(_MSC_VER)
/** @brief MSVC 64-bit integer format string. */
#define NUM_FORMAT "%I64d"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/** @brief C99 64-bit integer format string. */
#define NUM_FORMAT "%lld"
#else
/** @brief C89 32/64-bit long integer format string. */
#define NUM_FORMAT "%ld"
#endif
#endif /* !NUM_FORMAT */

/**
 * @brief Format specifier for size_t across MSVC and POSIX/C89.
 */
#ifndef SIZE_T_FORMAT
#if defined(_MSC_VER)
/** @brief MSVC size_t format string. */
#define SIZE_T_FORMAT "%Iu"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/** @brief C99 size_t format string. */
#define SIZE_T_FORMAT "%zu"
#else
/** @brief C89 unsigned long format string for size_t. */
#define SIZE_T_FORMAT "%lu"
#endif
#endif /* !SIZE_T_FORMAT */

/**
 * @brief Character type mapped according to RHASHEQ_UNICODE settings.
 */
#if defined(_WIN32) && defined(RHASHEQ_UNICODE)
/** @brief Wide character type for Unicode builds on Windows. */
typedef wchar_t rhasheq_char_t;
#else
/** @brief Narrow character type for ANSI and POSIX builds. */
typedef char rhasheq_char_t;
#endif

/**
 * @brief Thread-local storage specifier when RHASHEQ_MULTITHREADED is active.
 */
#if defined(RHASHEQ_MULTITHREADED)
#if defined(_MSC_VER)
/** @brief MSVC thread-local storage specifier. */
#define RHASHEQ_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
/** @brief GCC/Clang thread-local storage specifier. */
#define RHASHEQ_THREAD_LOCAL __thread
#else
/** @brief Fallback empty thread-local storage specifier. */
#define RHASHEQ_THREAD_LOCAL
#endif
#else
/** @brief Single-threaded empty storage specifier. */
#define RHASHEQ_THREAD_LOCAL
#endif

/**
 * @brief Error codes returned by all rhasheq functions.
 */
typedef enum NO_DISCARD rhasheq_error {
  /** @brief Operation succeeded without error. */
  RHASHEQ_SUCCESS = 0,
  /** @brief Invalid (e.g. NULL or out-of-range) argument provided. */
  RHASHEQ_INVALID_ARGUMENT = 1,
  /** @brief Memory allocation failed (out of memory). */
  RHASHEQ_OUT_OF_MEMORY = 2,
  /** @brief LibRHash library initialization failed. */
  RHASHEQ_INIT_ERROR = 3,
  /** @brief Computing the file digest failed (e.g. file not found or I/O
     error). */
  RHASHEQ_HASH_ERROR = 4,
  /** @brief Formatting or converting the digest to hexadecimal failed. */
  RHASHEQ_FORMAT_ERROR = 5
} rhasheq_error_t;

/**
 * @brief Maximum hexadecimal digest buffer length (130 bytes for 512-bit
 * hashes).
 */
#define RHASHEQ_DIGEST_HEX_SIZE 130

/**
 * @brief Maximum raw binary digest buffer length (64 bytes for 512-bit hashes).
 */
#define RHASHEQ_DIGEST_RAW_SIZE 64

/**
 * @brief Maximum formatted error message buffer length.
 */
#define RHASHEQ_ERRBUF_SIZE 256

/**
 * @brief Allocates a zero-initialized memory buffer with OOM error percolation.
 * @param size Number of bytes to allocate (must be > 0).
 * @param out_ptr Pointer to store the allocated memory address.
 * @return RHASHEQ_SUCCESS on success, RHASHEQ_INVALID_ARGUMENT or
 * RHASHEQ_OUT_OF_MEMORY on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t rhasheq_malloc(size_t size, void **out_ptr);

/**
 * @brief Frees a memory buffer allocated by rhasheq_malloc.
 * @param ptr Pointer to memory to free (safe to pass NULL).
 */
RHASHEQ_EXPORT void rhasheq_free(void *ptr);

/**
 * @brief Formats an error code and system errno into a human-readable string
 * using Safe CRT when available.
 * @param err The rhasheq_error code to format.
 * @param sys_errno The system errno value to describe.
 * @param buf Destination character buffer.
 * @param buf_size Size of the destination buffer in bytes.
 * @return RHASHEQ_SUCCESS on success, RHASHEQ_INVALID_ARGUMENT or
 * RHASHEQ_FORMAT_ERROR on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t rhasheq_format_error(rhasheq_error_t err,
                                                    int sys_errno, char *buf,
                                                    size_t buf_size);

/**
 * @brief Initializes the underlying LibRHash library algorithms.
 * @return RHASHEQ_SUCCESS on success.
 */
RHASHEQ_EXPORT rhasheq_error_t rhasheq_init(void);

/**
 * @brief Computes a file's hash using the specified LibRHash algorithm and
 * writes the raw and formatted digest.
 * @param filepath Path to the file to hash.
 * @param hash_id The LibRHash algorithm identifier (e.g. RHASH_CRC32C,
 * RHASH_SHA256).
 * @param print_sum_flags LibRHash formatting flags (e.g. RHPR_HEX).
 * @param digest Output buffer for the raw binary digest (at least 64 bytes).
 * @param gen_hash Output buffer for the formatted hash string (at least 130
 * bytes).
 * @return RHASHEQ_SUCCESS on success, or an error enum code on invalid
 * arguments, OOM, hash failure, or formatting error.
 */
RHASHEQ_EXPORT rhasheq_error_t hash_file(
    const char *filepath, enum rhash_ids hash_id,
    enum rhash_print_sum_flags print_sum_flags, char *digest, char *gen_hash);

/**
 * @brief Verifies a file's hash using the specified LibRHash algorithm against
 * an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param hash_id The LibRHash algorithm identifier.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t rhasheq_verify_file(const char *filepath,
                                                   enum rhash_ids hash_id,
                                                   const char *gold_hash,
                                                   bool *out_equal);

/**
 * @brief Verifies a file's CRC32 checksum against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t crc32(const char *filepath,
                                     const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's MD4 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t md4(const char *filepath, const char *gold_hash,
                                   bool *out_equal);

/**
 * @brief Verifies a file's MD5 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t md5(const char *filepath, const char *gold_hash,
                                   bool *out_equal);

/**
 * @brief Verifies a file's SHA1 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha1(const char *filepath, const char *gold_hash,
                                    bool *out_equal);

/**
 * @brief Verifies a file's Tiger digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t tiger(const char *filepath,
                                     const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's TTH (Tiger Tree Hash) digest against an expected hex
 * digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t tth(const char *filepath, const char *gold_hash,
                                   bool *out_equal);

/**
 * @brief Verifies a file's BTIH (BitTorrent InfoHash) digest against an
 * expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t btih(const char *filepath, const char *gold_hash,
                                    bool *out_equal);

/**
 * @brief Verifies a file's ED2K (eDonkey2000) digest against an expected hex
 * digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t ed2k(const char *filepath, const char *gold_hash,
                                    bool *out_equal);

/**
 * @brief Verifies a file's AICH (Advanced Intelligent Corruption Handler)
 * digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t aich(const char *filepath, const char *gold_hash,
                                    bool *out_equal);

/**
 * @brief Verifies a file's Whirlpool digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t whirlpool(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal);

/**
 * @brief Verifies a file's RIPEMD-160 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t ripemd160(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal);

/**
 * @brief Verifies a file's GOST R 34.11-94 digest against an expected hex
 * digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t gost94(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's GOST R 34.11-94 CryptoPro digest against an expected
 * hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t gost94_cryptopro(const char *filepath,
                                                const char *gold_hash,
                                                bool *out_equal);

/**
 * @brief Verifies a file's HAS-160 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t has160(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's GOST R 34.11-2012 (256-bit) digest against an
 * expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t gost12_256(const char *filepath,
                                          const char *gold_hash,
                                          bool *out_equal);

/**
 * @brief Verifies a file's GOST R 34.11-2012 (512-bit) digest against an
 * expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t gost12_512(const char *filepath,
                                          const char *gold_hash,
                                          bool *out_equal);

/**
 * @brief Verifies a file's SHA-224 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha224(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA-256 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha256(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA-384 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha384(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA-512 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha512(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's EDON-R 256 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t edonr256(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's EDON-R 512 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t edonr512(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA3-224 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha3_224(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA3-256 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha3_256(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA3-384 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha3_384(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's SHA3-512 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t sha3_512(const char *filepath,
                                        const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's CRC32C checksum against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t crc32c(const char *filepath,
                                      const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's Snefru-128 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t snefru128(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal);

/**
 * @brief Verifies a file's Snefru-256 digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t snefru256(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal);

/**
 * @brief Verifies a file's BLAKE2s digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t blake2s(const char *filepath,
                                       const char *gold_hash, bool *out_equal);

/**
 * @brief Verifies a file's BLAKE2b digest against an expected hex digest.
 * @param filepath Path to the file to hash.
 * @param gold_hash Expected hexadecimal hash string.
 * @param out_equal Output boolean set to true on match, false on mismatch.
 * @return RHASHEQ_SUCCESS on success, or an error enum on failure.
 */
RHASHEQ_EXPORT rhasheq_error_t blake2b(const char *filepath,
                                       const char *gold_hash, bool *out_equal);

#if defined(RHASHEQ_TEST_MOCKS)
/**
 * @brief Mock control flag to simulate malloc out-of-memory failure in tests.
 */
RHASHEQ_EXPORT extern int g_rhasheq_mock_malloc_fail;

/**
 * @brief Mock control flag to simulate rhash_file failure in tests.
 */
RHASHEQ_EXPORT extern int g_rhasheq_mock_rhash_file_fail;

/**
 * @brief Mock control flag to simulate rhash_print_bytes failure in tests.
 */
RHASHEQ_EXPORT extern int g_rhasheq_mock_rhash_print_fail;

/**
 * @brief Mock control flag to simulate Safe CRT formatting failure in tests.
 */
RHASHEQ_EXPORT extern int g_rhasheq_mock_format_fail;

/**
 * @brief Mock control flag to simulate rhasheq_init failure in tests.
 */
RHASHEQ_EXPORT extern int g_rhasheq_mock_init_fail;
#endif /* RHASHEQ_TEST_MOCKS */

#ifdef RHASHEQ_IMPLEMENTATION

#if defined(RHASHEQ_TEST_MOCKS)
int g_rhasheq_mock_malloc_fail = 0;
int g_rhasheq_mock_rhash_file_fail = 0;
int g_rhasheq_mock_rhash_print_fail = 0;
int g_rhasheq_mock_format_fail = 0;
int g_rhasheq_mock_init_fail = 0;
#endif /* RHASHEQ_TEST_MOCKS */

/** @brief Thread-safe/process-wide initialization state flag. */
static RHASHEQ_THREAD_LOCAL int g_rhasheq_initialized = 0;

RHASHEQ_EXPORT rhasheq_error_t rhasheq_malloc(size_t size, void **out_ptr) {
  void *ptr;
  if (out_ptr == NULL || size == 0) {
    return RHASHEQ_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;
#if defined(RHASHEQ_TEST_MOCKS)
  if (g_rhasheq_mock_malloc_fail > 0) {
    g_rhasheq_mock_malloc_fail--;
    if (g_rhasheq_mock_malloc_fail == 0) {
      return RHASHEQ_OUT_OF_MEMORY;
    }
  } else if (g_rhasheq_mock_malloc_fail < 0) {
    return RHASHEQ_OUT_OF_MEMORY;
  }
#endif
  ptr = malloc(size);
#if !defined(RHASHEQ_TEST_MOCKS)
  if (ptr == NULL) {
    return RHASHEQ_OUT_OF_MEMORY;
  }
#endif
  memset(ptr, 0, size);
  *out_ptr = ptr;
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT void rhasheq_free(void *ptr) {
  if (ptr != NULL) {
    free(ptr);
  }
}

RHASHEQ_EXPORT rhasheq_error_t rhasheq_format_error(rhasheq_error_t err,
                                                    int sys_errno, char *buf,
                                                    size_t buf_size) {
  char err_desc[128];
  if (buf == NULL || buf_size < 64) {
    return RHASHEQ_INVALID_ARGUMENT;
  }
  buf[0] = '\0';
  err_desc[0] = '\0';
#if defined(RHASHEQ_TEST_MOCKS)
  if (g_rhasheq_mock_format_fail != 0) {
    return RHASHEQ_FORMAT_ERROR;
  }
#endif
#if defined(_MSC_VER)
  {
    int str_rc;
    str_rc = (int)strerror_s(err_desc, sizeof(err_desc), sys_errno);
    sprintf_s(buf, buf_size, "rhasheq error %d (errno %d/%d: %s)", (int)err,
              sys_errno, str_rc, err_desc);
  }
#elif defined(RHASHEQ_USE_C89STRINGUTILS)
  {
    int str_rc;
    str_rc =
        (int)c89stringutils_strerror_s(err_desc, sizeof(err_desc), sys_errno);
    c89stringutils_snprintf_s(buf, buf_size,
                              "rhasheq error %d (errno %d/%d: %s)", (int)err,
                              sys_errno, str_rc, err_desc);
  }
#else
  {
    const char *s;
    s = strerror(sys_errno);
    if (s != NULL) {
      strncpy(err_desc, s, sizeof(err_desc) - 1);
      err_desc[sizeof(err_desc) - 1] = '\0';
    }
    sprintf(buf, "rhasheq error %d (errno %d: %.32s)", (int)err, sys_errno,
            err_desc);
  }
#endif
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t rhasheq_init(void) {
#if defined(RHASHEQ_TEST_MOCKS)
  if (g_rhasheq_mock_init_fail != 0) {
    return RHASHEQ_INIT_ERROR;
  }
#endif
  if (g_rhasheq_initialized == 0) {
    rhash_library_init();
    g_rhasheq_initialized = 1;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t hash_file(
    const char *filepath, enum rhash_ids hash_id,
    enum rhash_print_sum_flags print_sum_flags, char *digest, char *gen_hash) {
  rhasheq_error_t rc;
  char *err_buf;
  int rhash_rc;
  size_t print_len;

  if (filepath == NULL || digest == NULL || gen_hash == NULL ||
      (unsigned int)hash_id == 0U) {
    return RHASHEQ_INVALID_ARGUMENT;
  }
  err_buf = NULL;

  rc = rhasheq_init();
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }

#if defined(RHASHEQ_TEST_MOCKS)
  if (g_rhasheq_mock_rhash_file_fail != 0) {
    rhash_rc = -1;
    errno = ENOENT;
  } else {
    rhash_rc =
        rhash_file((unsigned int)hash_id, filepath, (unsigned char *)digest);
  }
#else
  rhash_rc =
      rhash_file((unsigned int)hash_id, filepath, (unsigned char *)digest);
#endif

  if (rhash_rc < 0) {
    rc = rhasheq_malloc(RHASHEQ_ERRBUF_SIZE, (void **)&err_buf);
    if (rc != RHASHEQ_SUCCESS) {
      return rc;
    }
    rc = rhasheq_format_error(RHASHEQ_HASH_ERROR, errno, err_buf,
                              RHASHEQ_ERRBUF_SIZE);
    if (rc != RHASHEQ_SUCCESS) {
      rhasheq_free(err_buf);
      return rc;
    }
    fprintf(stderr, "LibRHash error: %s: %s\n", filepath, err_buf);
    rhasheq_free(err_buf);
    return RHASHEQ_HASH_ERROR;
  }

#if defined(RHASHEQ_TEST_MOCKS)
  if (g_rhasheq_mock_rhash_print_fail != 0) {
    print_len = 0;
  } else {
    print_len =
        rhash_print_bytes(gen_hash, (const unsigned char *)digest,
                          (size_t)rhash_get_digest_size((unsigned int)hash_id),
                          (int)print_sum_flags);
  }
#else
  print_len =
      rhash_print_bytes(gen_hash, (const unsigned char *)digest,
                        (size_t)rhash_get_digest_size((unsigned int)hash_id),
                        (int)print_sum_flags);
#endif

  if (print_len == 0) {
    return RHASHEQ_FORMAT_ERROR;
  }

  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t rhasheq_verify_file(const char *filepath,
                                                   enum rhash_ids hash_id,
                                                   const char *gold_hash,
                                                   bool *out_equal) {
  rhasheq_error_t rc;
  char *digest;
  char *gen_hash;

  if (filepath == NULL || gold_hash == NULL || out_equal == NULL ||
      (unsigned int)hash_id == 0U) {
    return RHASHEQ_INVALID_ARGUMENT;
  }
  *out_equal = false;
  digest = NULL;
  gen_hash = NULL;

  rc = rhasheq_malloc(RHASHEQ_DIGEST_RAW_SIZE, (void **)&digest);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }

  rc = rhasheq_malloc(RHASHEQ_DIGEST_HEX_SIZE, (void **)&gen_hash);
  if (rc != RHASHEQ_SUCCESS) {
    rhasheq_free(digest);
    return rc;
  }

  rc = hash_file(filepath, hash_id, RHPR_HEX, digest, gen_hash);
  if (rc != RHASHEQ_SUCCESS) {
    rhasheq_free(gen_hash);
    rhasheq_free(digest);
    return rc;
  }

  *out_equal = (strcmp(gen_hash, gold_hash) == 0) ? true : false;
  rhasheq_free(gen_hash);
  rhasheq_free(digest);
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t crc32(const char *filepath,
                                     const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_CRC32, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t md4(const char *filepath, const char *gold_hash,
                                   bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_MD4, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t md5(const char *filepath, const char *gold_hash,
                                   bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_MD5, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha1(const char *filepath, const char *gold_hash,
                                    bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA1, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t tiger(const char *filepath,
                                     const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_TIGER, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t tth(const char *filepath, const char *gold_hash,
                                   bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_TTH, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t btih(const char *filepath, const char *gold_hash,
                                    bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_BTIH, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t ed2k(const char *filepath, const char *gold_hash,
                                    bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_ED2K, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t aich(const char *filepath, const char *gold_hash,
                                    bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_AICH, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t whirlpool(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_WHIRLPOOL, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t ripemd160(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_RIPEMD160, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t gost94(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_GOST94, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t gost94_cryptopro(const char *filepath,
                                                const char *gold_hash,
                                                bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_GOST94_CRYPTOPRO, gold_hash,
                           out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t has160(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_HAS160, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t gost12_256(const char *filepath,
                                          const char *gold_hash,
                                          bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_GOST12_256, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t gost12_512(const char *filepath,
                                          const char *gold_hash,
                                          bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_GOST12_512, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha224(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA224, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha256(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA256, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha384(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA384, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha512(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA512, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t edonr256(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_EDONR256, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t edonr512(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_EDONR512, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha3_224(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA3_224, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha3_256(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA3_256, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha3_384(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA3_384, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t sha3_512(const char *filepath,
                                        const char *gold_hash,
                                        bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SHA3_512, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t crc32c(const char *filepath,
                                      const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_CRC32C, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t snefru128(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SNEFRU128, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t snefru256(const char *filepath,
                                         const char *gold_hash,
                                         bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_SNEFRU256, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t blake2s(const char *filepath,
                                       const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_BLAKE2S, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

RHASHEQ_EXPORT rhasheq_error_t blake2b(const char *filepath,
                                       const char *gold_hash, bool *out_equal) {
  rhasheq_error_t rc;
  rc = rhasheq_verify_file(filepath, RHASH_BLAKE2B, gold_hash, out_equal);
  if (rc != RHASHEQ_SUCCESS) {
    return rc;
  }
  return RHASHEQ_SUCCESS;
}

#endif /* RHASHEQ_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !RHASH_EQ_H */
