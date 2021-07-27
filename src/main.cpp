/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 */

#if defined(NATIVE_BUILD) && !defined(UNIT_TEST)

#include <thingset.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <iostream>

#include "linenoise.h"

#include "../test/test_data.c"


ThingSet thing(data_nodes, sizeof(data_nodes)/sizeof(DataNode));

//
// Setup functions used in test data included.
//
// Note: Test data is created as C data (not C++).
//
extern "C" {

void reset_function()
{
    printf("Reset function called!\n");
}

void auth_function()
{
    const char pass_exp[] = "expert123";
    const char pass_mkr[] = "maker456";

    if (strlen(pass_exp) == strlen(auth_password) &&
        strncmp(auth_password, pass_exp, strlen(pass_exp)) == 0)
    {
        thing.set_authentication(TS_EXP_MASK | TS_USR_MASK);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        thing.set_authentication(TS_MKR_MASK | TS_USR_MASK);
    }
    else {
        thing.set_authentication(TS_USR_MASK);
    }

    printf("Auth function called, password: %s\n", auth_password);
}

} // extern "C"


void pub_thread()
{
    char pub_msg[1000];

    while (1) {
        if (pub_serial_enable) {
            thing.txt_pub(pub_msg, sizeof(pub_msg), PUB_SER);
            printf("%s\r\n", pub_msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_serial_interval));
    }
}

int main()
{
    uint8_t resp_buf[1000];

    printf("\n----------------- Data node tree ---------------------\n");

    thing.dump_json();

    printf("\n----------------- ThingSet shell ---------------------\n");

    linenoiseHistoryLoad(".thingset-shell-history.txt"); /* Load the history at startup */

    std::thread t(pub_thread);

    char *line;
    while ((line = linenoise("")) != NULL) {
        if (line[0] != '\0') {
            linenoiseHistoryAdd(line);
            linenoiseHistorySave(".thingset-shell-history.txt");
            thing.process((uint8_t *)line, strlen(line), resp_buf, sizeof(resp_buf));
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
