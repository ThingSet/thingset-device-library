/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet shell app.
 */

#ifndef TS_IMPL_SHELL_H_
#define TS_IMPL_SHELL_H_

/* Use generic implementation of shell */
#include "../../apps/shell/ts_shell_g.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TS_IMPL_SHELL_CMD_HELP_PRINTED  1

/* forward declaration */
struct thingset_app;
struct ts_impl_shell;

#define ts_impl_shell_print(shell, format, ...) ts_shell_printf_g(format "\n", ##__VA_ARGS__)

#define ts_impl_shell_error(shell, format, ...) ts_shell_printf_g(format "\n", ##__VA_ARGS__)

int ts_impl_shell_init(const struct thingset_app *app);

static inline int ts_impl_shell_run(const struct thingset_app *app)
{
    return ts_shell_run_g();
}

int ts_impl_shell_get_line(const struct ts_impl_shell *shell, const char *promt, char **line);

int ts_impl_shell_execute_cmd(const char* cmd);

static inline int ts_impl_shell_execute_output(size_t *sizep, const char **output)
{
    return ts_shell_execute_output_g(sizep, output);
}

static inline int ts_impl_shell_join(void)
{
    return ts_shell_join_g();
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_SHELL_H_ */
