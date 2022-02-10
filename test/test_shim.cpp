/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

void test_shim_get_object(void)
{
    ThingSet ts;

    ThingSetObject *object = ts.get_object(ID_INFO);
    thingset_oref_t oref = ts_obj_db_oref_by_object(object);
    TEST_ASSERT_EQUAL_UINT16(ID_INFO, ts_obj_id(oref));
}

void tests_shim(void)
{
    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(test_shim_get_object);

    UNITY_END();
}
