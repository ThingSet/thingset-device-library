/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet communication buffers.
 */

#ifndef TS_IMPL_BUF_H_
#define TS_IMPL_BUF_H_

#include "../../src/thingset_time.h"

#include <net/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ts_impl_buf net_buf

int ts_impl_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_impl_buf **buffer);

int ts_impl_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                struct ts_impl_buf **buffer);

int ts_impl_buf_unref(struct ts_impl_buf *buffer);

static inline int ts_impl_buf_ref(struct ts_impl_buf *buffer)
{
    if (net_buf_ref(buffer) != buffer) {
        return -ENOMEM;
    }
    return 0;
}

static inline uint16_t ts_impl_buf_size(struct ts_impl_buf *buffer)
{
    return buffer->size;
}

static inline uint16_t ts_impl_buf_len(struct ts_impl_buf *buffer)
{
    return buffer->len;
}

static inline uint16_t ts_impl_buf_headroom(struct ts_impl_buf *buffer)
{
    return net_buf_headroom(buffer);
}

static inline uint16_t ts_impl_buf_tailroom(struct ts_impl_buf *buffer)
{
    return net_buf_tailroom(buffer);
}

static inline uint8_t *ts_impl_buf_data(struct ts_impl_buf *buffer)
{
    return buffer->data;
}

static inline uint8_t *ts_impl_buf_tail(struct ts_impl_buf *buffer)
{
    return net_buf_tail(buffer);
}

static inline void ts_impl_buf_reserve(struct ts_impl_buf *buffer, uint16_t reserve)
{
    net_buf_reserve(buffer, reserve);
}

static inline void ts_impl_buf_reset(struct ts_impl_buf *buffer)
{
    net_buf_reset(buffer);
}

int ts_impl_buf_clone(struct ts_impl_buf *buffer, thingset_time_ms_t timeout_ms, struct ts_impl_buf **clone);

static inline uint8_t *ts_impl_buf_add(struct ts_impl_buf *buffer, uint16_t len)
{
    return (uint8_t *)net_buf_add(buffer, (size_t)len);
}

static inline uint8_t *ts_impl_buf_add_mem(struct ts_impl_buf *buffer, const uint8_t *data, uint16_t len)
{
    return (uint8_t *)net_buf_add_mem(buffer, (const void *)data, (size_t)len);
}

static inline uint8_t *ts_impl_buf_add_u8(struct ts_impl_buf *buffer, uint8_t val)
{
    return (uint8_t *)net_buf_add_u8(buffer, val);
}

static inline uint8_t *ts_impl_buf_remove(struct ts_impl_buf *buffer, uint16_t len)
{
    return (uint8_t *)net_buf_remove_mem(buffer, (size_t)len);
}

static inline uint8_t *ts_impl_buf_push(struct ts_impl_buf *buffer, uint16_t len)
{
    return (uint8_t *)net_buf_push(buffer, (size_t)len);
}

static inline uint8_t *ts_impl_buf_pull(struct ts_impl_buf *buffer, uint16_t len)
{
    return (uint8_t *)net_buf_pull(buffer, (size_t)len);
}

int ts_impl_buf_free_all(void);

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_BUF_H_ */
