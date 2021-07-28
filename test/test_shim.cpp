/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

void test_shim_get_node(void)
{
    ThingSet ts(&data_nodes[0], data_nodes_size);

    ThingSetDataNode *node = ts.get_node(ID_INFO);
    TEST_ASSERT_EQUAL_PTR(&data_nodes[0], node);
}
