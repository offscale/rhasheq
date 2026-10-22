/**
 * @file no_discard.h
 * @brief Cross-platform compiler macro for the nodiscard attribute.
 *
 * This header provides a generic macro to mark functions and types such that
 * discarding their return values emits a compiler warning. It serves as a
 * compatibility layer across various C and C++ standards and compilers.
 */

#ifndef C_CI_NO_DISCARD_H
#define C_CI_NO_DISCARD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @def NO_DISCARD
 * @brief Marks a return type or function such that discarding its return value
 * emits a compiler warning.
 *
 * This macro expands to the appropriate compiler-specific attribute or standard
 * directive based on the language version and compiler being used:
 * - C23 / C++17: Uses the standard `[[nodiscard]]` attribute.
 * - Clang: Uses `__attribute__((warn_unused_result))`.
 * - MSVC: Uses the `_Check_return_` SAL annotation.
 * - Fallback: Expands to nothing if unsupported on enum typedefs.
 *
 * @par Example Usage on a typedef:
 * @code
 *   typedef enum NO_DISCARD { OK, ERR } my_error_t;
 * @endcode
 *
 * @par Example Usage on a function:
 * @code
 *   NO_DISCARD my_error_t do_something(void);
 * @endcode
 *
 * @note If NO_DISCARD is already defined prior to including this header,
 *       the existing definition will be preserved.
 */
#ifndef NO_DISCARD
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/** @brief C23 nodiscard attribute. */
#define NO_DISCARD [[nodiscard]]
#elif defined(__cplusplus) && __cplusplus >= 201703L
/** @brief C++17 nodiscard attribute. */
#define NO_DISCARD [[nodiscard]]
#elif defined(__clang__)
/** @brief Clang warn_unused_result attribute (supported on enum types and
 * functions). */
#define NO_DISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
/* clang-format off */
#include <sal.h>
/* clang-format on */
/** @brief MSVC SAL _Check_return_ annotation. */
#define NO_DISCARD _Check_return_
#else
/** @brief Fallback empty no-discard macro for compilers not supporting
 * warn_unused_result on enum types. */
#define NO_DISCARD
#endif
#endif /* !NO_DISCARD */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CI_NO_DISCARD_H */
