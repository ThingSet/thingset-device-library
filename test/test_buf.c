/*
 * Copyright (c) 2021 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/**
 * @brief Test communication buffer
 *
 * This test verifies message buffer usage:
 * - ts_buf_alloc()
 * - ts_buf_ref()
 * - ts_buf_unref()
 */
void test_buf(void)
{
    int ret;
    struct ts_buf *buffers[TS_CONFIG_BUF_COUNT + 1];

    /* Check buffer pool size for testing */
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(16,  TS_CONFIG_BUF_COUNT);
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(1024, TS_CONFIG_BUF_DATA_SIZE);

    /* Allocate all buffers */
    for (int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        ret = ts_buf_alloc(10, 10, &buffers[i]);
        TEST_ASSERT_EQUAL(0, ret);
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }

    /* Try one more */
    ret = ts_buf_alloc(10, 10, &buffers[TS_CONFIG_BUF_COUNT]);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    /* Double unref on single ref first buffer - Expect second unref to fail */
    ret = ts_buf_unref(buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);
    ret = ts_buf_unref(buffers[0]);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    /* We should now get another buffer */
    ret = ts_buf_alloc(10, 10, &buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_NOT_NULL(buffers[0]);

    /* Double unref on double ref first buffer - Expect 2nd unref to pass */
    ret = ts_buf_ref(buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);
    ret = ts_buf_unref(buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);
    ret = ts_buf_unref(buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);

    /* Expect third unref to fail */
    ret = ts_buf_unref(buffers[0]);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    /* We should now get another buffer again */
    ret = ts_buf_alloc(10, 10, &buffers[0]);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_NOT_NULL(buffers[0]);

    /* Unref all buffers */
    for (int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        ret = ts_buf_unref(buffers[i]);
        TEST_ASSERT_EQUAL(0, ret);
    }

    /* Allocate all buffers again */
    for (int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        ret = ts_buf_alloc(10, 10, &buffers[i]);
        TEST_ASSERT_EQUAL(0, ret);
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }

    /* Unref all buffers */
    for (int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        ret = ts_buf_unref(buffers[i]);
        TEST_ASSERT_EQUAL(0, ret);
    }
}

void tests_buf(void)
{
    UNITY_BEGIN();

    //  Operating system abstraction
    RUN_TEST(test_buf);

    UNITY_END();
}
