/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 */

#if defined(NATIVE_BUILD) && !defined(UNIT_TEST)

#include "thingset.h"
#include "../test/thingset/test_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linenoise.h"

int main()
{
    ThingSet ts(data_nodes, sizeof(data_nodes)/sizeof(DataNode));
    uint8_t resp_buf[1000];

    printf("\n----------------- Data node tree ---------------------\n");

    ts.dump_json();

    printf("\n----------------- ThingSet shell ---------------------\n");

    linenoiseHistoryLoad(".thingset-shell-history.txt"); /* Load the history at startup */

    char *line;
    while ((line = linenoise("> ")) != NULL) {
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

void reset_function()
{
    printf("Reset function called!\n");
}

void auth_function(const char *args)
{
    printf("Auth function called, password: %s\n", args);
}

void dummy()
{
    // do nothing, only used in unit-tests
}

#endif
