/*
 * Copyright (c) 2022 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../../../test/test.h"

void main(void)
{
    (void)thingset_init(TEST_CORE_LOCID);
    (void)thingset_init(TEST_INSTANCE_LOCID);
    (void)thingset_init(TEST_NEIGHBOUR_LOCID);
    thingset_authorisation_set(TEST_CORE_LOCID, TS_ANY_RW);
    thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_ANY_RW);
    thingset_authorisation_set(TEST_NEIGHBOUR_LOCID, TS_ANY_RW);
}
