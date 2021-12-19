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

#include <math.h>

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

#define TEST_ASSERT(condition)                                                                     \
        zassert_true(condition, "expect: >true<\nactual: >%d<", (int)(condition))

#define TEST_ASSERT_TRUE(condition)                                                                \
        zassert_true(condition, "expect: >true<\nactual: >%d<", (int)(condition))

#define TEST_ASSERT_TRUE_MESSAGE(condition, msg)                                                   \
        zassert_true(condition, "expect: >true<\nactual: >%d<\n%s", (int)(condition), msg)

#define TEST_ASSERT_FALSE(condition)                                                               \
        zassert_false(condition, "expect: >false<\nactual: >%d<", (int)(condition))

#define TEST_ASSERT_FALSE_MESSAGE(condition, msg)                                                  \
        zassert_false(condition, "expect: >false<\nactual: >%d<\n%s", (int)(condition), msg)

#define TEST_ASSERT_NULL(pointer)                                                                  \
        zassert_is_null(pointer, "expect: >NULL<\nactual: >%d<", (int)(pointer))

#define TEST_ASSERT_NOT_NULL(pointer)                                                              \
        zassert_not_null(pointer, "expect: >!NULL<\nactual: >%d<", (int)(pointer))

#define TEST_ASSERT_NOT_NULL_MESSAGE(pointer, msg)                                                 \
        zassert_not_null(pointer, "expect: >!NULL<\nactual: >%d<\n%s", (int)(pointer), msg)

#define TEST_ASSERT_EQUAL(expected, actual)                                                        \
        zassert_equal((int)(expected), (int)(actual), "expect: >%d<\nactual: >%d<",                \
                      (int)(expected), (int)(actual))

#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg)                                           \
        zassert_equal((int)(expected), (int)(actual), "expect: >%d<\nactual: >%d<\n%s",            \
                      (int)(expected), (int)(actual), msg)

#define TEST_ASSERT_EQUAL_FLOAT(expected, actual)                                                  \
        do {                                                                                       \
            (void)snprintf(&test_assert_buffer[0], TEST_ASSERT_BUFFER_SIZE,                        \
                           "expect: >%g<\nactual: >%g<", (float)(expected), (float)(actual));      \
            if (isinf((float)(expected))) {                                                        \
                zassert_true(isinf((float)(actual)), "%s", &test_assert_buffer[0]);                \
            } else if (isnan((float)(expected))) {                                                 \
                zassert_true(isnan((float)(actual)), "%s", &test_assert_buffer[0]);                \
            } else {                                                                               \
                zassert_within((float)(expected), (float)(actual), 0.0000001, "%s",                \
                               &test_assert_buffer[0]);                                            \
            }                                                                                      \
        } while(false)

#define TEST_ASSERT_EQUAL_HEX(expected, actual)                                                    \
        zassert_equal((unsigned)(expected), (unsigned)(actual), "expect: >0x%x<\nactual: >0x%x<",  \
                      (unsigned)(expected), (unsigned)(actual))

#define TEST_ASSERT_EQUAL_HEX8(expected, actual)                                                   \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "expect: >0x%x<\nactual: >0x%x<",    \
                      (unsigned)(expected), (unsigned)(actual))

#define TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, actual, msg)                                      \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "expect: >0x%x<\nactual: >0x%x<\n%s",\
                      (unsigned)(expected), (unsigned)(actual), msg)

#define TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, num_elements)                               \
        do {                                                                                       \
            int len = 0;                                                                           \
            int ret;                                                                               \
            ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len,                \
                           "expect: >0x");                                                           \
            len += ret;                                                                            \
            for (int i = 0; (i < num_elements) && (ret >= 0); i++) {                               \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len, "%02x",    \
                               (unsigned int)(((const uint8_t*)(expected))[i]));                   \
                len += ret;                                                                        \
            }                                                                                      \
            if (ret >= 0) {                                                                        \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len,            \
                               "<\nactual: >0x");                                                       \
                len += ret;                                                                        \
            }                                                                                      \
            for (int i = 0; (i < num_elements) && (ret >= 0); i++) {                               \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len, "%02x",    \
                               (unsigned int)(((const uint8_t*)(actual))[i]));                     \
                len += ret;                                                                        \
            }                                                                                      \
            if (ret >= 0) {                                                                        \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len, "<");      \
            }                                                                                      \
            if (ret < 0) {                                                                         \
                (void)snprintf(&test_assert_buffer[0], TEST_ASSERT_BUFFER_SIZE,                    \
                               "expect/ actual conversion error %d", ret);                         \
            }                                                                                      \
            test_assert_buffer[TEST_ASSERT_BUFFER_SIZE - 1] = '\0';                                \
        } while(false);                                                                            \
        zassert_equal(memcmp(expected, actual, num_elements), 0, &test_assert_buffer[0])

#define TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, actual, num_elements, msg)                  \
        do {                                                                                       \
            int len = 0;                                                                           \
            int ret;                                                                               \
            ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len,                \
                           "expect: >0x");                                                           \
            len += ret;                                                                            \
            for (int i = 0; (i < num_elements) && (ret >= 0); i++) {                               \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len, "%02x",    \
                               (unsigned int)(((const uint8_t*)(expected))[i]));                   \
                len += ret;                                                                        \
            }                                                                                      \
            if (ret >= 0) {                                                                        \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len,            \
                               "<\nactual: >0x");                                                       \
                len += ret;                                                                        \
            }                                                                                      \
            for (int i = 0; (i < num_elements) && (ret >= 0); i++) {                               \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len, "%02x",    \
                               (unsigned int)(((const uint8_t*)(actual))[i]));                     \
                len += ret;                                                                        \
            }                                                                                      \
            if (ret >= 0) {                                                                        \
                ret = snprintf(&test_assert_buffer[len], TEST_ASSERT_BUFFER_SIZE - len,            \
                               "<\n%s", msg);                                                      \
            }                                                                                      \
            if (ret < 0) {                                                                         \
                (void)snprintf(&test_assert_buffer[0], TEST_ASSERT_BUFFER_SIZE,                    \
                               "expect/ actual conversion error %d\n%s", ret, msg);                \
            }                                                                                      \
            test_assert_buffer[TEST_ASSERT_BUFFER_SIZE - 1] = '\0';                                \
        } while(false);                                                                            \
        zassert_equal(memcmp(expected, actual, num_elements), 0, &test_assert_buffer[0])

#define TEST_ASSERT_EQUAL_INT(expected, actual)                                                    \
        zassert_equal((int)(expected), (int)(actual), "expect: >%d<\nactual: >%d<",                \
                      (int)(expected), (int)(actual))

#define TEST_ASSERT_EQUAL_INT8(expected, actual)                                                   \
        zassert_equal((int8_t)(expected), (int8_t)(actual), "expect: >%u<\nactual: >%u<",          \
                      (int)(int8_t)(expected), (int)(int8_t)(actual))

#define TEST_ASSERT_EQUAL_INT16(expected, actual)                                                  \
        zassert_equal((int16_t)(expected), (int16_t)(actual), "expect: >%d<\nactual: >0xd<",       \
                      (int)(int16_t)(expected), (int)(int16_t)(actual))

#define TEST_ASSERT_EQUAL_INT32(expected, actual)                                                  \
        zassert_equal((int32_t)(expected), (int32_t)(actual), "expect: >%d<\nactual: >0xd<",       \
                      (int)(int32_t)(expected), (int)(int32_t)(actual))

#define TEST_ASSERT_EQUAL_INT64(expected, actual)                                                  \
        zassert_equal((int64_t)(expected), (int64_t)(actual),                                      \
                      "expect: >%" PRIi64 "<\nactual: >%" PRIi64 "<",                              \
                      (int64_t)(expected), (int64_t)(actual))

#define TEST_ASSERT_EQUAL_UINT(expected, actual)                                                   \
        zassert_equal((unsigned)(expected), (unsigned)(actual), "expect: >0x%x<\nactual: >0x%x<",  \
                      (unsigned)(expected), (unsigned)(actual))

#define TEST_ASSERT_EQUAL_UINT8(expected, actual)                                                  \
        zassert_equal((uint8_t)(expected), (uint8_t)(actual), "expect: >%u<\nactual: >%u<",        \
                      (unsigned)(uint8_t)(expected), (unsigned)(uint8_t)(actual))

#define TEST_ASSERT_EQUAL_UINT16(expected, actual)                                                 \
        zassert_equal((uint16_t)(expected), (uint16_t)(actual), "expect: >%u<\nactual: >%u<",      \
                      (unsigned)(uint16_t)(expected), (unsigned)(uint16_t)(actual))

#define TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected, actual, msg)                                    \
        zassert_equal((uint16_t)(expected), (uint16_t)(actual), "expect: >%u<\nactual: >%u<\n%s",  \
                      (unsigned)(uint16_t)(expected), (unsigned)(uint16_t)(actual), msg)

#define TEST_ASSERT_EQUAL_UINT32(expected, actual)                                                 \
        zassert_equal((uint32_t)(expected), (uint32_t)(actual), "expect: >%u<\nactual: >%u<",      \
                      (unsigned)(uint32_t)(expected), (unsigned)(uint32_t)(actual))

#define TEST_ASSERT_EQUAL_UINT64(expected, actual)                                                 \
        zassert_equal((uint64_t)(expected), (uint64_t)(actual),                                    \
                      "expect: >%" PRIu64 "<\nactual: >%" PRIu64 "<",                              \
                      (uint64_t)(expected), (uint64_t)(actual))

#define TEST_ASSERT_EQUAL_PTR(expected, actual)                                                    \
        zassert_equal_ptr(expected, actual, "expect: >%d<\nactual: >%d<",                          \
                          (int)(expected), (int)(actual))

#define TEST_ASSERT_EQUAL_PTR_MESSAGE(expected, actual, msg)                                       \
        zassert_equal_ptr(expected, actual, "expect: >%d<\nactual: >%d<\n%s",                      \
                          (int)(expected), (int)(actual), msg)

#define TEST_ASSERT_EQUAL_STRING(expected, actual)                                                 \
        zassert_equal(strcmp(expected, actual), 0, "expect: >%s<\nactual: >%s<", expected, actual)

#define TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len)                                        \
        zassert_equal(strncmp(expected, actual, len), 0, "expect: >%.*s<\nactual: >%.*s<",         \
                      (size_t)len, expected, (size_t)len, actual)

#define TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected, actual, len, msg)                           \
        zassert_equal(strncmp(expected, actual, len), 0, "expect: >%.*s<\nactual: >%.*s<\n%s",     \
                      (size_t)len, expected, (size_t)len, actual, msg)

#define TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, msg)                                    \
        zassert_equal(strcmp(expected, actual), 0, "expect: >%s<\nactual: >%s<\n%s",               \
                      expected, actual, msg)

#define TEST_ASSERT_EQUAL_size_t(expected, actual)                                                 \
        zassert_equal((size_t)(expected),(size_t)(actual), "expect: >%d<\nactual: >%d<",           \
                      (int)(expected), (int)(actual))

#define TEST_ASSERT_GREATER_OR_EQUAL_INT(threshold, actual)                                        \
        zassert_true((int)(threshold) <= (int)(actual), "expect: >= >%d<\nactual: >%d<",           \
                     (int)(threshold), (int)(actual))

#define TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(threshold, actual, msg)                           \
        zassert_true((int)(threshold) <= (int)(actual), "expect: >= >%d<\nactual: >%d<\n%s",       \
                     (int)(threshold), (int)(actual), msg)

#define TEST_ASSERT_GREATER_OR_EQUAL_UINT32(threshold, actual)                                     \
        zassert_true((uint32_t)(threshold) <= (uint32_t)(actual), "expect: >= >%u<\nactual: >%u<", \
                     (unsigned int)(uint32_t)(threshold), (unsigned int)(uint32_t)(actual))

#define TEST_ASSERT_GREATER_OR_EQUAL_size_t(threshold, actual)                                     \
        zassert_true((size_t)(threshold) <= (size_t)(actual), "expect: >= >%d<\nactual: >%d<",     \
                     (int)(threshold), (int)(actual))

#define TEST_ASSERT_GREATER_THAN_UINT16(threshold, actual)                                         \
        zassert_true((uint16_t)(threshold) < (uint16_t)(actual), "expect: > >%u<\nactual: >%u<",   \
                     (unsigned int)(uint16_t)(threshold), (unsigned int)(uint16_t)(actual))

#define TEST_ASSERT_LESS_THAN_INT(threshold, actual)                                               \
        zassert_true((int)(threshold) > (int)(actual), "expect: < >%d<\nactual: >%d<",             \
                     (int)(threshold), (int)(actual))

#define TEST_ASSERT_LESS_THAN_INT_MESSAGE(threshold, actual, msg)                                  \
        zassert_true((int)(threshold) > (int)(actual), "expect: < >%d<\nactual: >%d<\n%s",         \
                     (int)(threshold), (int)(actual), msg)

#define TEST_ASSERT_LESS_THAN_size_t(threshold, actual)                                            \
        zassert_true((size_t)(threshold) > (size_t)(actual), "expect: < >%d<\nactual: >%d<",       \
                     (int)(threshold), (int)(actual))

#define TEST_ASSERT_LESS_THAN_size_t_MESSAGE(threshold, actual, msg)                               \
        zassert_true((size_t)(threshold) > (size_t)(actual), "expect: < >%d<\nactual: >%d<\n%s",   \
                     (int)(threshold), (int)(actual), msg)

#define TEST_ASSERT_LESS_THAN_UINT16(threshold, actual)                                            \
        zassert_true((uint16_t)(threshold) > (uint16_t)(actual), "expect: < >%u<\nactual: >%u<",   \
                     (unsigned)(uint16_t)(threshold), (unsigned)(uint16_t)(actual))

#define TEST_ASSERT_LESS_OR_EQUAL_UINT8(threshold, actual)                                         \
        zassert_true((uint8_t)(threshold) >= (uint8_t)(actual), "expect: <= >%u<\nactual: >%u<",   \
                     (unsigned)(uint8_t)(threshold), (unsigned)(uint8_t)(actual))

#define TEST_ASSERT_LESS_OR_EQUAL_size_t(threshold, actual)                                        \
        zassert_true((size_t)(threshold) >= (size_t)(actual), "expect: <= >%d<\nactual: >%d<",     \
                     (int)(threshold), (int)(actual))

#define TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(threshold, actual, msg)                           \
        zassert_true((size_t)(threshold) >= (size_t)(actual), "expect: <= >%d<\nactual: >%d<\n%s", \
                     (int)(threshold), (int)(actual), msg)

#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    \
        zassert_not_equal(expected, actual, "expect: ! >%d<\nactual: >%d<",                        \
                          (int)(expected), (int)(actual))

#define TEST_ASSERT_NOT_EQUAL_PTR(expected, actual)                                                \
        zassert_not_equal((void *)expected, (void *)actual, "expect: ! >%d<\nactual: >",           \
                          (int)(expected), (int)(actual))

#define TEST_ASSERT_NOT_EQUAL_PTR_MESSAGE(expected, actual, msg)                                   \
        zassert_not_equal((void *)expected, (void *)actual, "expect: ! >%d<\nactual: >%d<\n%s",    \
                          (int)(expected), (int)(actual), msg)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ZTEST_UNITY_H_ */
