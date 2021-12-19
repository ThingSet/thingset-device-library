/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

TEST_MAIN()
{
    TS_LOGD("Running ThingSet tests");

    // Test environment
    tests_assert();

    // OSAL
    tests_time();
    tests_buf();

    // Libraries
    tests_jsmn();

    // ThingSet basic classes
    tests_obj();
    tests_msg();
    tests_ctx();
    tests_shim();

    // ThingSet protocol
    tests_common();
    tests_bin();
    tests_txt();

    // ThingSet applications
    tests_shell();

    // Test specific to the ThingSet implementation
    tests_impl();
}
