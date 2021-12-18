/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../src/thingset_env.h"

#include <sys/slist.h>
#include <net/buf.h>

#include "../../src/ts_buf.h"
#include "../../src/ts_log.h"

/**
 * @brief Device's communication buffers pool.
 *
 * Pool of ThingSet communication buffers used by (all) ThingSet communication of the device.
 */
NET_BUF_POOL_VAR_DEFINE(ts_buf_pool, TS_CONFIG_BUF_COUNT, TS_CONFIG_BUF_DATA_SIZE, 0, NULL);


int ts_impl_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_impl_buf **buffer)
{
    struct net_buf *buf = net_buf_alloc_len(&ts_buf_pool, (size_t)size,
                                            TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms));
    if (buf == NULL) {
        return -ENOMEM;
    }
    *buffer = buf;
    return 0;
}

int ts_impl_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                struct ts_impl_buf **buffer)
{
    struct net_buf *buf = net_buf_alloc_with_data(&ts_buf_pool, data, (size_t)size,
                                                  TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms));
    if (buf == NULL) {
        return -ENOMEM;
    }
    *buffer = buf;
    return 0;
}

int ts_impl_buf_unref(struct ts_impl_buf *buffer)
{
    if (buffer->ref == 0) {
        /*
         * Add some extra safety here as it is not tracked by Zephyr.
         * Will not work in all cases (aka. re-allocation of same message
         * in between).
         */
        TS_LOGE("ThingSet marks already unused buffer unused.");
        return -EALREADY;
    }

    net_buf_unref(buffer);

    return 0;
}

int ts_impl_buf_clone(struct ts_impl_buf *buffer, thingset_time_ms_t timeout_ms,
                      struct ts_impl_buf **clone)
{
    struct net_buf *cbuf = net_buf_clone(buffer, TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms));
    if (cbuf == NULL) {
        return -ETIMEDOUT;
    }
    *clone = cbuf;
    return 0;
}

int ts_impl_buf_free_all(void)
{
    for (int buf_idx = 0; buf_idx < (ts_buf_pool.buf_count - ts_buf_pool.uninit_count); buf_idx++) {
        struct net_buf *buf = &ts_buf_pool.__bufs[buf_idx];
        while(buf->ref > 0) {
            net_buf_unref(buf);
        }
    }
    return 0;
}
