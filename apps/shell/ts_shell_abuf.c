/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell append buffer.
 */

#include "../../src/thingset_env.h"

#include "ts_shell.h"
#include "ts_shell_abuf.h"

#include <stdlib.h>

void ts_shell_abuf_init(struct ts_shell_abuf *ab) {
    ab->b = NULL;
    ab->len = 0;
    ab->size = 0;
}

int ts_shell_abuf_reserve(struct ts_shell_abuf *ab, uint16_t len) {
    if (len > (ab->size - ab->len)) {
        char *new;
        /* Allocate buffer in chunks of 128 bytes */
        uint16_t new_size = ab->size + ((len / 128) + 1) * 128;
        if (new_size > (TS_CONFIG_SHELL_MEM_SIZE / 2)) {
            new_size = TS_CONFIG_SHELL_MEM_SIZE / 2;
        }
        int ret = ts_shell_alloc(new_size, 10, (uint8_t **)&new);
        if (ret != 0) {
            return ret;
        }
        memcpy(new, ab->b, ab->len);
        ts_shell_free(ab->b);
        ab->b = new;
        ab->size = new_size;
    }
    return 0;
}

void ts_shell_abuf_append(struct ts_shell_abuf *ab, const char *s, uint16_t len) {
    int ret = ts_shell_abuf_reserve(ab, len);
    if (ret != 0) {
        return;
    }

    memcpy(&ab->b[ab->len], s, len);
    ab->len += len;
}

void ts_shell_abuf_free(struct ts_shell_abuf *ab) {
    if (ab->b != NULL) {
        ts_shell_free(ab->b);
    }
    ts_shell_abuf_init(ab);
}
