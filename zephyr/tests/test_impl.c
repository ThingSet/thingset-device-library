/*
 * Copyright (c) 2022 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet unit test suite for Zephyr implementation.
 */

#include "../../test/test.h"

void test_shell_zephyr(void)
{
    /* Assure test environment for shell app */
    TEST_ASSERT_TRUE(CONFIG_SHELL);
    TEST_ASSERT_TRUE(CONFIG_SHELL_BACKEND_DUMMY);
    TEST_ASSERT_EQUAL(32, TS_CONFIG_SHELL_MEM_SIZE);
}

void tests_impl(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_shell_zephyr);

    UNITY_END();
}
