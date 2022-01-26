/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell generic implementation
 */

#include "../../src/thingset_env.h"

#include "ts_shell.h"
#include "ts_shell_g.h"
#include "ts_shell_abuf.h"

#include <stdarg.h>

/* Shell application */

atomic_flag ts_shell_running_g = ATOMIC_FLAG_INIT;

pthread_t ts_shell_thread_g;

struct ts_shell_abuf ts_shell_output_g = {.b = NULL, .len = 0, .size = 0};

void *ts_shell_fun_g(void *ptr)
{
    char *line;
    while(ts_shell_get_line(NULL, "> ", &line) == 0) {
        (void)ts_shell_execute_cmd(line);
        ts_shell_free(line);
    }

    atomic_flag_clear(&ts_shell_running_g);
    pthread_exit(NULL);
}

int ts_shell_run_g(void)
{
    if (atomic_flag_test_and_set(&ts_shell_running_g)) {
        /* Shell already running */
        TS_ASSERT(false, "SHELL: %s requests run on already running shell", __func__);
        return -EALREADY;
    }

    int ret = pthread_create(&ts_shell_thread_g, NULL, ts_shell_fun_g, (void*)NULL);

    return ret;
}


int ts_shell_printf_g(const char *fmt, ...)
{
    va_list arg_ptr;

    /* Get required length */
    va_start(arg_ptr, fmt);
    int len = vsnprintf(NULL, 0, fmt, arg_ptr);
    va_end(arg_ptr);
    if (len < 0) {
        return len;
    }

    /* Reserve memory in output buffer */
    if (len > 0) {
        int ret = ts_shell_abuf_reserve(&ts_shell_output_g, len + 1);
        if (ret != 0) {
            return ret;
        }
    }

    /* Really write to output buffer */
    va_start(arg_ptr, fmt);
    (void)vsnprintf(&ts_shell_output_g.b[ts_shell_output_g.len], len, fmt, arg_ptr);
    va_end(arg_ptr);
    ts_shell_output_g.len += len;
    ts_shell_output_g.b[ts_shell_output_g.len] = '\0';

    return 0;
}

int ts_shell_execute_cmd_g(const char* cmd)
{
    if ((cmd == NULL) || (cmd[0] == '\0')) {
        return 0;
    }

    unsigned int len = strlen(cmd);
    char *command;
    int ret = ts_shell_alloc(len, 10, (uint8_t **)&command);
    if (ret != 0) {
        return ret;
    }
    memcpy(command, cmd, len);

    int argc;
    char **argv;
    ret = ts_shell_split_cmd(command, &argc, &argv);
    if (ret != 0) {
        ts_shell_free(command);
        return ret;
    }
    if ((argc > 0) && (strcmp(argv[0], "clear") == 0)) {
        ts_shell_print(NULL, "\x1b[H\x1b[2J");
        ts_shell_output_g.len = 0;
        return 0;
    }

    /* Free output buffer of last command */
    ts_shell_abuf_free(&ts_shell_output_g);

    /* Now really execute the command */
    ret = ts_shell_cmd(NULL, argc, argv);
    ts_shell_free((const uint8_t *)command);
    ts_shell_free((const uint8_t *)argv);

    /* Print new shell output to stdout */
    if (ts_shell_output_g.len > 0) {
        fputs(&ts_shell_output_g.b[0], stdout);
    }

    return ret;
}

int ts_shell_execute_output_g(size_t *sizep, const char **output)
{
    if ((ts_shell_output_g.b == NULL) ||
        (ts_shell_output_g.size == 0) ||
        (ts_shell_output_g.len == 0)) {
        *sizep = 0;
        *output = NULL;
        return -ENAVAIL;
    }

    *sizep = ts_shell_output_g.len;
    *output = ts_shell_output_g.b;
    return 0;
}

int ts_shell_join_g(void)
{
    return pthread_join(ts_shell_thread_g, NULL);
}
