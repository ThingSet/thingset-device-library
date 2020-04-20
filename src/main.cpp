/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 */

#if defined(NATIVE_BUILD) && !defined(UNIT_TEST)

#include "thingset.h"
#include "../test/test_data.h"
#include "../test/test_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <iostream>

#include "linenoise.h"

ThingSet ts(data_nodes, sizeof(data_nodes)/sizeof(DataNode));

void pub_thread()
{
    char pub_msg[1000];

    while (1) {
        if (pub_serial_enable) {
            ts.pub_msg_json(pub_msg, sizeof(pub_msg), pub_serial_ids, pub_serial_array.num_elements);
            printf("%s\r\n", pub_msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_serial_interval));
    }
}

int main()
{
    uint8_t resp_buf[1000];

    printf("\n----------------- Data node tree ---------------------\n");

    ts.dump_json();

    printf("\n----------------- ThingSet shell ---------------------\n");

    linenoiseHistoryLoad(".thingset-shell-history.txt"); /* Load the history at startup */

    std::thread t(pub_thread);

    char *line;
    while ((line = linenoise("")) != NULL) {
        if (line[0] != '\0') {
            linenoiseHistoryAdd(line);
            linenoiseHistorySave(".thingset-shell-history.txt");
            ts.process((uint8_t *)line, strlen(line), resp_buf, sizeof(resp_buf));
            printf("%s\n", (char *)resp_buf);
        }
        free(line);
    }
    return 0;
}

void conf_callback()
{
    printf("Conf callback called!\n");
}

void dummy()
{
    // do nothing, only used in unit-tests
}

#endif
