/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet shell app.
 */

#ifndef TS_IMPL_SHELL_H_
#define TS_IMPL_SHELL_H_

#include <shell/shell.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TS_IMPL_SHELL_CMD_HELP_PRINTED SHELL_CMD_HELP_PRINTED

/* forward declaration */
struct thingset_app;

#define ts_impl_shell shell

#define ts_impl_shell_print(shell, format, ...) shell_print(shell, format, ##__VA_ARGS__)

#define ts_impl_shell_error(shell, format, ...) shell_error(shell, format, ##__VA_ARGS__)

static inline int ts_impl_shell_init(const struct thingset_app *app)
{
    return 0;
}

static inline int ts_impl_shell_run(const struct thingset_app *app)
{
    return 0;
}

static inline int ts_impl_shell_get_line(const struct ts_impl_shell *shell, const char *prompt,
                                         char **line)
{
    return -ENOTSUP;
}

int ts_impl_shell_execute_cmd(const char* cmd);

int ts_impl_shell_execute_output(size_t *sizep, const char **output);

static inline int ts_impl_shell_join(void)
{
    return -ENOTSUP;
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_SHELL_H_ */
