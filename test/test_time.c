/*
 * Copyright (c) 2021 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

void test_time(void)
{
    thingset_time_ms_t ms;
    thingset_time_ms_t delta;

    ms = thingset_time_ms();
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, ms);

    delta = thingset_time_ms_delta(ms - 1);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, delta);

    ms = thingset_time_ms();
    delta = thingset_time_ms_delta(ms + 1);
    TEST_ASSERT_EQUAL_UINT32(THINGSET_TIME_MS_MAX - 1, delta);
}

void tests_time(void)
{
    UNITY_BEGIN();

    //  Operating system abstraction
    RUN_TEST(test_time);

    UNITY_END();
}
