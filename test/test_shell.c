/*
 * Copyright (c) 2022 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet unit test suite for Zephyr implementation.
 */

#include "test.h"

#include <string.h>

void test_shell(void)
{
    int ret;
    const char *expected_response;
    const char *found_response;
    const char *shell_response;
    size_t shell_reponse_size;

    /* Assure test environment */
    TEST_ASSERT_TRUE(TS_CONFIG_SHELL);
    TEST_ASSERT_EQUAL_STRING(TEST_SHELL_NAME, TS_CONFIG_SHELL_NAME);
    TEST_ASSERT_EQUAL(TEST_SHELL_LOCID, TS_CONFIG_SHELL_LOCID);
    TEST_ASSERT_EQUAL(TEST_SHELL_PORTID, TS_CONFIG_SHELL_PORTID);

    /* Assure shell memory allocation - allocate some bytes to show working */
    TEST_ASSERT_NOT_EQUAL(0, TS_CONFIG_SHELL_MEM_SIZE);
    ret = ts_shell_alloc(10, THINGSET_TIMEOUT_IMMEDIATE, (uint8_t **)&expected_response);
    TEST_ASSERT_EQUAL(0, ret);
    ret = ts_shell_free((uint8_t *)expected_response);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_shell_execute_cmd("clear");
    (void)ts_shell_execute_output(&shell_reponse_size, &shell_response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, shell_response);

    ret = ts_shell_execute_cmd("ts txt ?conf/");
    (void)ts_shell_execute_output(&shell_reponse_size, &shell_response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, shell_response);

    expected_response =
        ":85 Content. [\"BatCharging_V\",\"LoadDisconnect_V\",\"ui64\",\"i64\",\"ui32\",\"i32\","
        "\"ui16\",\"i16\",\"f32\",\"bool\",\"strbuf\",\"f32_rounded\",\"DecFrac_degC\","
        "\"secret_expert\",\"secret_maker\",\"arrayi32\",\"arrayfloat\",\"bytesbuf\"]";
    /*
     * Shell response contains all the bytes that goes through the shell backend interface
     * including escape codes, NL and CR. The function strnstr() finds the place in the buffer where
     * the interesting data is located.
     */
    found_response = strnstr(shell_response, expected_response, shell_reponse_size);
    TEST_ASSERT_NOT_NULL_MESSAGE(found_response, shell_response);

    (void)ts_shell_execute_cmd("clear");
    ret = ts_shell_execute_cmd("ts txt =.pub/report {\"Enable\":true}");
    (void)ts_shell_execute_output(&shell_reponse_size, &shell_response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, shell_response);

    expected_response = ":84 Changed.";
    found_response = strnstr(shell_response, expected_response, shell_reponse_size);
    TEST_ASSERT_NOT_NULL_MESSAGE(found_response, shell_response);

}

void tests_shell(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_shell);

    UNITY_END();
}
