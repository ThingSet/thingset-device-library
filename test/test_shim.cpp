/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

void test_shim_get_object(void)
{
    ThingSet ts(&data_objects[0], data_objects_size);

    ThingSetDataObject *object = ts.get_object(0x10);   // timestamp object "t_s"
    TEST_ASSERT_EQUAL_PTR(&data_objects[0], object);
}
