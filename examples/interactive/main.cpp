/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../src/thingset.h"

#include "../../apps/shell/ts_shell.h"

#include <thread>
#include <iostream>

#include "things.h"

ThingSet thing;

void pub_thread()
{
    char pub_msg[1000];

    while (1) {
        if (pub_report_enable) {
            thing.txt_statement(pub_msg, sizeof(pub_msg), "report");
            printf("%s\r\n", pub_msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_report_interval));
    }
}

int main()
{
    printf("\n----------------- Data object tree ---------------------\n\n");

    thing.dump_json();

    printf("\n----------------- ThingSet shell ---------------------\n\n");

    std::thread t(pub_thread);

    ts_shell_init(NULL);
    ts_shell_run(NULL);

    ts_shell_join();

    return 0;
}
