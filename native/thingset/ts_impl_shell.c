/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet shell app.
 */

#include "../../src/thingset_env.h"

#include "../../apps/shell/ts_shell_g.h"

#include "ts_impl_shell_linenoise.h"

int ts_impl_shell_init(const struct thingset_app *app)
{
    linenoiseHistoryLoad(".thingset-shell-history.txt"); /* Load the history at startup */
    //linenoiseClearScreen();

    return 0;
}

int ts_impl_shell_get_line(const struct ts_impl_shell *shell, const char *prompt, char **line)
{
    char *input = linenoise(prompt);

    if (input == NULL) {
        return -ENAVAIL;
    }

    *line = input;
    return 0;
}

int ts_impl_shell_execute_cmd(const char* cmd)
{
    if ((cmd == NULL) || (cmd[0] == '\0')) {
        return 0;
    }

    linenoiseHistoryAdd(cmd);
    linenoiseHistorySave(".thingset-shell-history.txt");

    /* Let generic shell function ts_shell_execute_cmd_g() do the rest */
    return ts_shell_execute_cmd_g(cmd);
}
