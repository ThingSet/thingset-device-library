/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet shell app.
 */

#include "../../apps/shell/ts_shell.h"

#include <shell/shell_dummy.h>

int ts_impl_shell_execute_cmd(const char* cmd)
{
    const struct shell *exe_shell = shell_backend_dummy_get_ptr();

    return shell_execute_cmd(exe_shell, cmd);
}

int ts_impl_shell_execute_output(size_t *sizep, const char **output)
{
    const struct shell *exe_shell = shell_backend_dummy_get_ptr();

    *output = shell_backend_dummy_get_output(exe_shell, sizep);

    return (*output == NULL) ? -ENAVAIL : 0;
}

int ts_impl_shell_cmd(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ts_argc;
    char **ts_argv;
    int ret = ts_shell_split_cmd(argv[1], &ts_argc, &ts_argv);
    if (ret != 0) {
        return ret;
    }

    ret = ts_shell_cmd_ts(shell, ts_argc, ts_argv);
    ts_shell_free((const uint8_t *)ts_argv);

    return ret;
}

/*
 * Use own subcommands handler to prevent wildcard expansion (?, .) on chars used in ThingSet.
 * Use SHELL_OPT_ARG_RAW to prevent argument split on (", ') used by ThingSet.
 */
SHELL_CMD_ARG_REGISTER(ts, NULL, "ThingSet commands", ts_impl_shell_cmd, 1, SHELL_OPT_ARG_RAW);
