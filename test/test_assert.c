/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/**
 * @brief Test Asserts
 *
 * This test verifies various assert macros used in the unit tests.
 *
 */
void test_assert(void)
{
    TEST_ASSERT(1);
    TEST_ASSERT_TRUE(1);
    TEST_ASSERT_TRUE_MESSAGE(1, "test_assert");
    TEST_ASSERT_FALSE(0);
    TEST_ASSERT_NULL(NULL);
    TEST_ASSERT_NOT_NULL("foo");
    TEST_ASSERT_EQUAL(1, 1);
    TEST_ASSERT_EQUAL_MESSAGE(1, 1, "test_assert");
    TEST_ASSERT_EQUAL_FLOAT(123.4567890123456789, 123.4567890123456789);
    TEST_ASSERT_EQUAL_HEX(0x1234, 0x1234);
    TEST_ASSERT_EQUAL_HEX8(0x34, 0x34);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x34, 0x34, "test_assert");
    TEST_ASSERT_EQUAL_HEX8_ARRAY("1234", "1234", 4);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE("1234", "1234", 4, "test_assert");
    TEST_ASSERT_EQUAL_INT(2, 2);
    TEST_ASSERT_EQUAL_INT32(-32, -32);
    TEST_ASSERT_EQUAL_UINT(3U, 3U);
    TEST_ASSERT_EQUAL_UINT8(0x103U, 0x203U);
    TEST_ASSERT_EQUAL_UINT16(0x303U, 0x303U);
    TEST_ASSERT_EQUAL_UINT32(0x12345678U, 0x12345678U);
    TEST_ASSERT_EQUAL_PTR(NULL, NULL);
    TEST_ASSERT_EQUAL_STRING("ttt", "ttt");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ttt", "ttt", "test_assert");
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(100, 100);
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(100, 101);
    TEST_ASSERT_LESS_THAN_size_t(100, 99);
    TEST_ASSERT_LESS_THAN_size_t_MESSAGE(100, 99, "test_assert");
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(UINT8_MAX, UINT8_MAX);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(UINT8_MAX, UINT8_MAX - 1);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(0, 0);
    TEST_ASSERT_LESS_OR_EQUAL_size_t(100, 100);
    TEST_ASSERT_LESS_OR_EQUAL_size_t(100, 99);
    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(100, 99, "test_assert");
    TEST_ASSERT_NOT_EQUAL(1, 2);
}

void tests_assert(void)
{
    UNITY_BEGIN();

    // Test asserts
    RUN_TEST(test_assert);

    UNITY_END();
}
