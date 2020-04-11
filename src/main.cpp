/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 */

#if defined(NATIVE_BUILD) && !defined(UNIT_TEST)

#include "thingset.h"
#include "../test/thingset/test_data.h"

measurement_data_t meas;
calibration_data_t cal;

#include <iostream>
#include <string>

int main()
{
    ThingSet ts(data_nodes, sizeof(data_nodes)/sizeof(DataNode));
    uint8_t resp_buf[1000];

    printf("\n----------------- data node tree ---------------------\n");

    ts.dump_json();

    printf("\n------------------------------------------------------\n");

    for (std::string line; std::getline(std::cin, line);) {
        ts.process((uint8_t *)line.c_str(), line.length(), resp_buf, sizeof(resp_buf));
        std::cout << resp_buf << std::endl;
    }
    return 0;
}

void dummy()
{
    ;
}

#endif
