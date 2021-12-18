/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet communication buffer queue.
 */

#ifndef TS_IMPL_BUFQ_H_
#define TS_IMPL_BUFQ_H_

#include "ts_impl_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ts_impl_bufq k_fifo

#define TS_IMPL_BUFQ_DEFINE(bufq_name) K_FIFO_DEFINE(bufq_name)

#define TS_IMPL_BUFQ_DECLARE(bufq_name) extern struct k_fifo bufq_name

static inline int ts_impl_bufq_init(struct ts_impl_bufq *bufq)
{
    k_fifo_init(bufq);
    return 0;
}

static inline int ts_impl_bufq_put(struct ts_impl_bufq *bufq, struct ts_impl_buf *buffer)
{
    net_buf_put(bufq, buffer);
    return 0;
}

static inline int ts_impl_bufq_get(struct ts_impl_bufq *bufq, thingset_time_ms_t timeout_ms,
                                   struct ts_impl_buf **buffer)
{
    struct net_buf *got = net_buf_get(bufq, TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms));
    if (got == NULL) {
        return -ETIMEDOUT;
    }
    *buffer = got;
    return 0;
}

static inline int ts_impl_bufq_is_empty(struct ts_impl_bufq *bufq)
{
    return k_fifo_is_empty(bufq);
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_BUFQ_H_ */
