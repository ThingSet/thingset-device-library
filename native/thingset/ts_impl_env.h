/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet environment.
 */

#ifndef TS_IMPL_ENV_H_
#define TS_IMPL_ENV_H_

/*
 * ThingSet configuration (& overrides)
 * ====================================
 */

#include "../../src/ts_config.h"

#ifdef __cplusplus
extern "C" {
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

/*
 * ThingSet unit test
 * ==================
 */

#if TS_CONFIG_UNIT_TEST

#define TEST_MAIN() int main(void)

#include "../unity/src/unity.h"

#endif

/*
 * ThingSet assert
 * ---------------
 */
#if TS_CONFIG_ASSERT && TS_CONFIG_UNIT_TEST

/**
 * @def TEST_ASSERT_BUFFER_SIZE
 *
 * @brief Size of buffer for assertion message build up.
 */
#define TEST_ASSERT_BUFFER_SIZE 300

/**
 * @brief Buffer for assertion message build up.
 *
 * @note Buffer must be provided by the test application.
 */
extern char test_assert_buffer[TEST_ASSERT_BUFFER_SIZE];

#define TS_IMPL_ASSERT(test, fmt, ...)                                                      \
    if (!(test)) {                                                                          \
        snprintf(&test_assert_buffer[0], TEST_ASSERT_BUFFER_SIZE, fmt, ##__VA_ARGS__);      \
        TEST_FAIL_MESSAGE(&test_assert_buffer[0]);                                          \
    }

#elif TS_CONFIG_ASSERT
#include <assert.h>
#define TS_IMPL_ASSERT(test, fmt, ...) assert(test && fmt)
#else
#define TS_IMPL_ASSERT(test, fmt, ...)
#endif

/*
 * Libc support
 * ------------
 */
#define _POSIX_C_SOURCE 200809L

/* - atomic support */
#ifdef __STDC_NO_ATOMICS__
#error "ThingSet device library needs <stdatomic.h>"
#endif
#if defined(__cplusplus) && __cplusplus <= 201703L
#ifndef _Atomic
#define _Atomic
#endif
#endif
#include <stdbool.h>
#include <stdatomic.h>

#include <errno.h>

char *ts_strnstr(const char *haystack, const char *needle, size_t len);
static inline char *strnstr(const char *haystack, const char *needle, size_t len) {
    return ts_strnstr(haystack, needle, len);
}

/*
 * POSIX.1-2017 support
 * --------------------
 */
#include <time.h>
#include <pthread.h>

/*
 * TinyCBOR support
 * ----------------
 */
#include "../tinycbor/src/cbor.h"

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
 * --------
 */
#include "ts_impl_bufq.h"

/*
 * Shell application support
 * -------------------------
 */
#include "ts_impl_shell.h"

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TS_IMPL_ENV_H_ */
