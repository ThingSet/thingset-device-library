/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell generic implementation
 */

#ifndef TS_SHELL_G_H_
#define TS_SHELL_G_H_

/**
 * @addtogroup ts_shell_api_priv
 * @{
 */

#include <stdint.h>

void *ts_shell_fun_g(void *ptr);

int ts_shell_run_g(void);

int ts_shell_printf_g(const char *fmt, ...);

int ts_shell_execute_cmd_g(const char* cmd);

int ts_shell_execute_output_g(size_t *sizep, const char **output);

int ts_shell_join_g(void);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

/** @}  <!-- ts_shell_api_priv --> */

#endif /* TS_SHELL_G_H_ */
