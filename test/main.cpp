/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "thingset.h"
#include "unity.h"

#include "test_data.h"
#include "tests.h"

#include <stdio.h>

#ifdef __WIN32__

// To avoid compilation error of unit tests
void setUp (void) {}
void tearDown (void) {}

#endif

uint8_t req_buf[TS_REQ_BUFFER_LEN];
uint8_t resp_buf[TS_RESP_BUFFER_LEN];

ThingSet ts(data_nodes, sizeof(data_nodes)/sizeof(DataNode));

int main()
{
    ts.set_pub_channels(pub_channels, sizeof(pub_channels)/sizeof(ts_pub_channel_t));

    tests_common();
    tests_json();
    tests_cbor();
}

void reset_function()
{
    printf("Reset function called!\n");
}

void auth_function(const char *args)
{
    printf("Auth function called, password: %s\n", args);
}
