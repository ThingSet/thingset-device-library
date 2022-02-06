/*
 * Copyright (c) 2022 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */
#include <unistd.h>
#include "../../test/test.h"

int main(void)
{
    (void)thingset_init(TEST_CORE_LOCID);
    (void)thingset_init(TEST_INSTANCE_LOCID);
    (void)thingset_init(TEST_NEIGHBOUR_LOCID);
    thingset_authorisation_set(TEST_CORE_LOCID, TS_ANY_RW);
    thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_ANY_RW);
    thingset_authorisation_set(TEST_NEIGHBOUR_LOCID, TS_ANY_RW);

    (void)thingset_run(TEST_INSTANCE_LOCID);
    (void)thingset_run(TEST_NEIGHBOUR_LOCID);

    while (true) {
        if (ts_shell_join() == 0) {
            break;
        }
        /* Sleep a little bit to allow especiall board emulations to work correctly */
        sleep(5);
    }

    return 0;
}
