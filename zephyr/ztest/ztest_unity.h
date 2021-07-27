/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZTEST_UNITY_H_
#define ZTEST_UNITY_H_

#if !CONFIG_ZTEST
#error "You need to define CONFIG_ZTEST."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <ztest.h>

#define TEST_ASSERT(condition) \
        zassert_true(condition, "exp: true: actual: %d", (int)(condition))
#define TEST_ASSERT_TRUE(condition) \
        zassert_true(condition, "exp: true: actual: %d", (int)(condition))
#define TEST_ASSERT_TRUE_MESSAGE(condition, msg) \
        zassert_true(condition, "exp: true: actual: %d\n%s", (int)(condition))
#define TEST_ASSERT_FALSE(condition) \
        zassert_false(condition, "exp: false: actual: %d", (int)(condition))
#define TEST_ASSERT_NULL(pointer) \
        zassert_is_null(pointer, "exp: NULL: actual: %d", (int)(pointer))
#define TEST_ASSERT_NOT_NULL(pointer) \
        zassert_not_null(pointer, "exp: !NULL: actual: %d", (int)(pointer))
#define TEST_ASSERT_EQUAL(expected, actual) \
        zassert_equal((int)(expected), (int)(actual), "exp: %d: actual: %d", (int)(expected), (int)(actual))
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg) \
        zassert_equal((int)(expected), (int)(actual), "exp: %d: actual: %d\n%s", (int)(expected), (int)(actual), msg)
#define TEST_ASSERT_EQUAL_FLOAT(expected, actual) \
        do { \
            char buffer[50]; \
            (void)snprintf(buffer, sizeof(buffer), "exp: %g: actual: %g", (float)(expected), (float)(actual)); \
            zassert_within((float)(expected), (float)(actual), 0.0000001, "%s", &buffer[0]); \
        } while(false)
#define TEST_ASSERT_EQUAL_HEX(expected, actual) \
        zassert_equal((unsigned)(expected), (unsigned)(actual), "exp: 0x%x: actual: 0x%x", (unsigned)(expected), (unsigned)(actual))
#define TEST_ASSERT_EQUAL_HEX8(expected, actual) \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "exp: 0x%x: actual: 0x%x", (unsigned)(expected), (unsigned)(actual))
#define TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, actual, msg) \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "exp: 0x%x: actual: 0x%x\n%s", (unsigned)(expected), (unsigned)(actual), msg)
#define TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, num_elements) \
        zassert_equal(memcmp(expected, actual, num_elements), 0, "exp: TBD: actual: TBD")
#define TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, actual, num_elements, msg) \
        zassert_equal(memcmp(expected, actual, num_elements), 0, "exp: TBD: actual: TBD\n%s", msg)
#define TEST_ASSERT_EQUAL_INT(expected, actual) \
        zassert_equal((int)(expected), (int)(actual), "exp: %d: actual: %d", (int)(expected), (int)(actual))
#define TEST_ASSERT_EQUAL_INT32(expected, actual) \
        zassert_equal((int32_t)(expected), (int32_t)(actual), "exp: %d: actual: 0xd", (int)(int32_t)(expected), (int)(int32_t)(actual))
#define TEST_ASSERT_EQUAL_UINT(expected, actual) \
        zassert_equal((unsigned)(expected), (unsigned)(actual), "exp: 0x%x: actual: 0x%x", (unsigned)(expected), (unsigned)(actual))
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "exp: %u: actual: %u", (unsigned)(uint8_t)(expected), (unsigned)(uint8_t)(actual))
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) \
        zassert_equal((uint16_t)(expected), (uint16_t)(actual), "exp: %u: actual: %u", (unsigned)(uint16_t)(expected), (unsigned)(uint16_t)(actual))
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
        zassert_equal((uint32_t)(expected), (uint32_t)(actual), "exp: %u: actual: %u", (unsigned)(uint32_t)(expected), (unsigned)(uint32_t)(actual))
#define TEST_ASSERT_EQUAL_PTR(expected, actual) \
        zassert_equal_ptr(expected, actual, "exp: %d: actual: %d", (int)(expected), (int)(actual))
#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
        zassert_equal(strcmp(expected, actual), 0, "exp: %s: actual: %s", expected, actual)
#define TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, msg) \
        zassert_equal(strcmp(expected, actual), 0, "exp: %s: actual: %s\n%s", expected, actual, msg)
#define TEST_ASSERT_GREATER_OR_EQUAL_size_t(threshold, actual) \
        zassert_true((size_t)(threshold) <= (size_t)(actual), "exp: >= %d: actual: %d", (int)(threshold), (int)(actual))
#define TEST_ASSERT_LESS_THAN_size_t(threshold, actual) \
        zassert_true((size_t)(threshold) > (size_t)(actual), "exp: < %d: actual: %d", (int)(threshold), (int)(actual))
#define TEST_ASSERT_LESS_THAN_size_t_MESSAGE(threshold, actual, msg) \
        zassert_true((size_t)(threshold) > (size_t)(actual), "exp: < %d: actual: %d\n%s", (int)(threshold), (int)(actual), msg)
#define TEST_ASSERT_LESS_OR_EQUAL_size_t(threshold, actual) \
        zassert_true((size_t)(threshold) >= (size_t)(actual), "exp: <= %d: actual: %d", (int)(threshold), (int)(actual))
#define TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(threshold, actual, msg) \
        zassert_true((size_t)(threshold) >= (size_t)(actual), "exp: <= %d: actual: %d\n%s", (int)(threshold), (int)(actual), msg)
#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
        zassert_not_equal(expected, actual, "exp: %d: actual: %d", (int)(expected), (int)(actual))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ZTEST_UNITY_H_ */
