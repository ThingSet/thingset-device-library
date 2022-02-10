/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet environment.
 */

#ifndef TS_IMPL_ENV_H_
#define TS_IMPL_ENV_H_

/*
 * ThingSet configuration (& overrides)
 * ====================================
 */

#include "../../src/ts_config.h"

#if TS_CONFIG_COM && !CONFIG_ARCH_POSIX && !CONFIG_POSIX_API
#error "You need to define CONFIG_POSIX_API."
#endif

/* Make Zephyr CONFIG_LOG overwrite ThingSet logging support configuration */
#if TS_CONFIG_LOG != CONFIG_LOG
#warning "ThingSet TS_CONFIG_LOG does not match Zephyr CONFIG_LOG - CONFIG_LOG will prevail!"
#undef TS_CONFIG_LOG
#define TS_CONFIG_LOG CONFIG_LOG
#endif

/*
 * ThingSet logging
 * ================
 */
#include "ts_impl_log.h"

/*
 * ThingSet Operating System Abstraction Layer (OSAL).
 * ===================================================
 */

#include <zephyr.h>

/*
 * ThingSet unit test
 * ==================
 */

#if TS_CONFIG_UNIT_TEST
#include "ts_impl_unity.h"
#endif

/*
 * ThingSet assert
 * ---------------
 */
#if TS_CONFIG_ASSERT && TS_CONFIG_UNIT_TEST
#define TS_IMPL_ASSERT(test, fmt, ...) zassert_true(test, fmt, ##__VA_ARGS__)
#elif TS_CONFIG_ASSERT
#include <sys/__assert.h>
#define TS_IMPL_ASSERT(test, fmt, ...) __ASSERT(test, fmt, ##__VA_ARGS__)
#elif TS_CONFIG_LOG
#define TS_IMPL_ASSERT(test, fmt, ...) if (!(test)) { TS_IMPL_LOGE(fmt, ##__VA_ARGS__); }
#else
#define TS_IMPL_ASSERT(test, fmt, ...)
#endif

/*
 * Libc support
 * ------------
 */

/* - standard C atomic support */
#ifdef __STDC_NO_ATOMICS__
#error "ThingSet device library needs <stdatomic.h>"
#endif
#if defined(__cplusplus) && __cplusplus <= 201703L
#ifndef _Atomic
#define _Atomic
#endif
#endif
#include <stdatomic.h>

#include <errno.h>

/* - Zephyr minimal c library support */
#if CONFIG_MINIMAL_LIBC

#if !CONFIG_MINIMAL_LIBC_MALLOC
#error "You need to define CONFIG_MINIMAL_LIBC_MALLOC if using CONFIG_MINIMAL_LIBC."
#endif

#if CONFIG_MINIMAL_LIBC_MALLOC_ARENA_SIZE == 0
#error "You need to define CONFIG_MINIMAL_LIBC_MALLOC_ARENA_SIZE > 0 to allow malloc() to work."
#endif

/*
 * Zephyr's minimal libc is missing some functions.
 * Provide !!sufficient!! replacements here.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define isnan(value) __builtin_isnan(value)
#define isinf(value) __builtin_isinf(value)

static inline long long int llroundf(float x)
{
    return __builtin_llroundf(x);
};

double ts_strtod(const char * string, char **endPtr);
static inline double strtod(const char * string, char **endPtr)
{
    return ts_strtod(string, endPtr);
};

static inline long long strtoll(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (long long)strtol(str, endptr, base);
};

static inline unsigned long long strtoull(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (unsigned long long)strtoul(str, endptr, base);
};

#endif /* CONFIG_MINIMAL_LIBC */

char *ts_strnstr(const char *haystack, const char *needle, size_t len);
static inline char *strnstr(const char *haystack, const char *needle, size_t len) {
    return ts_strnstr(haystack, needle, len);
}

/*
 * POSIX.1-2017 support
 * --------------------
 */
#if CONFIG_ARCH_POSIX
#include <time.h>
#include <pthread.h>
#elif CONFIG_POSIX_API
#include <posix/time.h>
#include <posix/pthread.h>
#endif

/*
 * TinyCBOR support
 * ----------------
 */
#if !CONFIG_TINYCBOR
#include "../../native/tinycbor/src/cbor.h"
#endif

/*
 * ts_endian.h
 * ----------
 */
#include "ts_impl_endian.h"

/*
 * ts_macro.h
 * ----------
 */
#include "ts_impl_macro.h"

/*
 * thingset_time.h
 * ---------------
 */
#include "ts_impl_time.h"

/*
 * ts_mem.h
 * --------
 */
#include "ts_impl_mem.h"

/*
 * ts_buf.h
 * --------
 */
#include "ts_impl_buf.h"

/*
 * ts_bufq.h
 * ----------
 */
#include "ts_impl_bufq.h"

/*
 * Shell application support
 * -------------------------
 */
#include "ts_impl_shell.h"


#endif /* TS_IMPL_ENV_H_ */
