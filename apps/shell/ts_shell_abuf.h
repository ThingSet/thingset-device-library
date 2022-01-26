/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell append buffer.
 */

#ifndef TS_SHELL_ABUF_H_
#define TS_SHELL_ABUF_H_

/**
 * @addtogroup ts_shell_api_priv
 * @{
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Append buffer.
 *
 * We define a very simple "append buffer" structure, that is a shell memory pool
 * allocated string where we can append to.
 *
 * Taken from linenoise.
 */
struct ts_shell_abuf {
    char *b;
    uint16_t len;
    uint16_t size;
};

void ts_shell_abuf_init(struct ts_shell_abuf *ab);

int ts_shell_abuf_reserve(struct ts_shell_abuf *ab, uint16_t len);

void ts_shell_abuf_append(struct ts_shell_abuf *ab, const char *s, uint16_t len);

void ts_shell_abuf_free(struct ts_shell_abuf *ab);

#ifdef __cplusplus
}
#endif

/** @}  <!-- ts_shell_api_priv --> */

#endif /* TS_SHELL_ABUF_H_ */
