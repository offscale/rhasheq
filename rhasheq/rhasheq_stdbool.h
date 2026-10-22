/**
 * @file rhasheq_stdbool.h
 * @brief C89-compatible boolean type definitions.
 *
 * stdbool isn't always included with every popular C89 implementation.
 * This variant is modified from MUSL.
 */

#ifndef RHASHEQ_STDBOOL_H
#define RHASHEQ_STDBOOL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#if !defined(__cplusplus)
#if !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) &&                           \
    !defined(__bool_true_false_are_defined)
#define _STDBOOL_H
#define __bool_true_false_are_defined 1

#ifdef bool
#undef bool
#endif
#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif

/** @brief Boolean type for C89 compatibility. */
typedef size_t rhasheq_bool_t;
/** @brief Standard bool macro mapped to rhasheq_bool_t. */
#define bool rhasheq_bool_t
/** @brief Boolean true constant. */
#define true 1
/** @brief Boolean false constant. */
#define false 0

#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) &&                     \
          !defined(__bool_true_false_are_defined) */
#endif /* !defined(__cplusplus) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !RHASHEQ_STDBOOL_H */
