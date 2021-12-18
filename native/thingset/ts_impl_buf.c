/*
 * Copyright (c) 2015-2019 Intel Corporation
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 *  @brief Native implementation of ThingSet communication buffers.
 */

#include <malloc.h>

#include "../../src/thingset_env.h"
#include "../../src/thingset_time.h"

#include "../../src/ts_macro.h"
#include "../../src/ts_log.h"

#include "ts_impl_buf.h"

struct ts_impl_buf ts_impl_bufs[TS_CONFIG_BUF_COUNT];

int ts_impl_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_impl_buf **buffer)
{
    struct ts_impl_buf *buf = NULL;

    for (unsigned int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        int_least8_t free_ref = 0;  /* Only 0 indicates free, < 0 indicates concurrent free */
        if (atomic_compare_exchange_weak(&ts_impl_bufs[i].ref, &free_ref, (int_least8_t)(1))) {
            buf = &ts_impl_bufs[i];
            break;
        }
    }

    if (!buf) {
        TS_LOGE("%s():%d: Failed to get free buffer", __func__, __LINE__);
        return -ENOMEM;
    }

    TS_LOGD("buf %p allocated", buf);

    if (size) {
        buf->__buf = malloc(size);
        if (!buf->__buf) {
            TS_LOGE("%s():%d: Failed to allocate data", __func__, __LINE__);
            (void)atomic_fetch_sub(&buf->ref, (int_least8_t)(1U));
            return -ENOMEM;
        }
    }
    else {
        buf->__buf = NULL;
    }

    buf->flags = 0;
    buf->size  = size;

    ts_impl_buf_reset(buf);

    *buffer = buf;
    return 0;
}

int ts_impl_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                struct ts_impl_buf **buffer)
{
    struct ts_impl_buf *buf = NULL;
    int_least8_t free_ref = 0;  /* Only 0 indicates free, < 0 indicates concurrent free */

    for (unsigned int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        if (atomic_compare_exchange_weak(&ts_impl_bufs[i].ref, &free_ref, (int_least8_t)(1))) {
            buf = &ts_impl_bufs[i];
            break;
        }
    }

    if (!buf) {
        TS_LOGE("%s():%d: Failed to get free buffer", __func__, __LINE__);
        return -ENOMEM;
    }

    TS_LOGD("buf %p allocated", buf);

    if (size) {
        buf->__buf = data;
        if (!buf->__buf) {
            TS_LOGE("%s():%d: Failed to provide data", __func__, __LINE__);
            (void)atomic_fetch_sub(&buf->ref, (int_least8_t)(1U));
            return -ENOMEM;
        }
    }
    else {
        buf->__buf = NULL;
    }

    buf->next = NULL;
    buf->flags = TS_IMPL_BUF_EXTERNAL_DATA;
    buf->size  = size;

    ts_impl_buf_reset(buf);

    *buffer = buf;
    return 0;
}

int ts_impl_buf_unref(struct ts_impl_buf *buffer)
{
    TS_LOGD("buf %p unref (%d)", buffer, (int)buffer->ref);

    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    void *free_buf = NULL;
    if (!(buffer->flags & TS_IMPL_BUF_EXTERNAL_DATA)) {
        free_buf = buffer->__buf;
    }

    int_least8_t ref = atomic_fetch_sub(&buffer->ref, (int_least8_t)(1U));
    if (ref == 0) {
        (void)atomic_fetch_add(&buffer->ref, (int_least8_t)(1));
        TS_LOGE("%s():%d: buf %p double free", __func__, __LINE__, buffer);
        return -EINVAL;
    }

    if ((ref == 1) && (free_buf != NULL)) {
        free(free_buf);
    }

    return 0;
}

int ts_impl_buf_ref(struct ts_impl_buf *buffer)
{
    TS_LOGD("buf %p ref (%d)", buffer, (int)buffer->ref);

    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    int_least8_t ref = atomic_fetch_add(&buffer->ref, (int_least8_t)(1));
    if (ref == 0) {
        (void)atomic_fetch_sub(&buffer->ref, (int_least8_t)(1));
        TS_LOGE("%s():%d: buf %p ref on freed buffer", __func__, __LINE__, buffer);
        return -ENOMEM;
    }

    return 0;
}

uint8_t *ts_impl_buf_tail(struct ts_impl_buf *buffer)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    return &buffer->data[buffer->len];
}

void ts_impl_buf_reserve(struct ts_impl_buf *buffer, uint16_t reserve)
{
    TS_LOGD("buf %p reserve %u", buffer, (unsigned int)reserve);

    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    buffer->data = buffer->__buf + reserve;
}

void ts_impl_buf_reset(struct ts_impl_buf *buffer)
{
    TS_LOGD("buf %p reset", buffer);

    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    buffer->data = buffer->__buf;
    buffer->len = 0;
}

int ts_impl_buf_clone(struct ts_impl_buf *buffer, thingset_time_ms_t timeout_ms,
                      struct ts_impl_buf **clone)
{
    TS_LOGD("buf %p clone", buffer);

    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    struct ts_impl_buf *buf;

    buf = malloc(sizeof(struct ts_impl_buf));
    if (!buf) {
        return -ENOMEM;
    }

    buf->size = buffer->size;
    buf->len = buffer->len;
    buf->flags = buffer->flags;

    /* Make a copy. */
    if (buf->flags & TS_IMPL_BUF_EXTERNAL_DATA) {
        buf->__buf = buffer->__buf;
        buf->data = buffer->data;
    }
    else if (buf->size) {
        buf->__buf = malloc(buf->size);
        if (!buf->__buf) {
            TS_LOGE("%s():%d: Failed to allocate data", __func__, __LINE__);
            free(buf);
            return -ENOMEM;
        }
        memcpy(buf->__buf, buffer->__buf, buf->size);
        buf->data = buf->__buf + ts_impl_buf_headroom(buffer);
    }
    else {
        buf->__buf = NULL;
        buf->data = NULL;
    }

    *clone = buf;
    return 0;
}

uint8_t *ts_impl_buf_add(struct ts_impl_buf *buffer, uint16_t len)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    uint8_t *tail = ts_impl_buf_tail(buffer);

    TS_LOGD("buf %p len %u", buffer, (unsigned int)len);

    TS_ASSERT(ts_impl_buf_tailroom(buffer) >= len, "NATIVE BUF: %s not enough tailroom (%u > %u)",
              __func__, (unsigned int)len, (unsigned int)ts_impl_buf_tailroom(buffer));

    buffer->len += len;
    return tail;
}

uint8_t *ts_impl_buf_add_mem(struct ts_impl_buf *buffer, const uint8_t *data, uint16_t len)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    uint8_t *tail = ts_impl_buf_add(buffer, len);

    if (tail != NULL) {
        memcpy(tail, data, len);
    }
    return tail;
}

uint8_t *ts_impl_buf_add_u8(struct ts_impl_buf *buffer, uint8_t val)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    uint8_t *u8;

    TS_LOGD("buf %p val 0x%02x", buffer, val);

    u8 = ts_impl_buf_add(buffer, 1);
    *u8 = val;

    return u8;
}

uint8_t *ts_impl_buf_remove(struct ts_impl_buf *buffer, uint16_t len)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    TS_LOGD("buf %p len %u", buffer, (unsigned int)len);

    TS_ASSERT(buffer->len >= len, "NATIVE BUF: %s not enough length (%u > %u)",
            __func__, (unsigned int)len, (unsigned int)buffer->len);

    buffer->len -= len;
    return ts_impl_buf_tail(buffer);
}

uint8_t *ts_impl_buf_push(struct ts_impl_buf *buffer, uint16_t len)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    TS_LOGD("buf %p len %u", buffer, (unsigned int)len);

    TS_ASSERT(ts_impl_buf_headroom(buffer) >= len, "NATIVE BUF: %s not enough headroom (%u > %u)",
              __func__, (unsigned int)len, (unsigned int)ts_impl_buf_headroom(buffer));

    buffer->data -= len;
    buffer->len += len;
    return buffer->data;
}

uint8_t *ts_impl_buf_pull(struct ts_impl_buf *buffer, uint16_t len)
{
    TS_ASSERT(buffer != NULL, "NATIVE BUF: %s on invalid buffer pointer NULL", __func__);

    TS_LOGD("buf %p len %u", buffer, (unsigned int)len);

    TS_ASSERT(buffer->len >= len, "NATIVE BUF: %s not enough length (%u > %u)",
            __func__, (unsigned int)len, (unsigned int)buffer->len);

    buffer->len -= len;
    return buffer->data += len;
}

int ts_impl_buf_free_all(void)
{
    for (unsigned int i = 0; i < TS_CONFIG_BUF_COUNT; i++) {
        struct ts_impl_buf *buffer = &ts_impl_bufs[i];
        /* Block buffer */
        int_least8_t ref = atomic_exchange(&buffer->ref, (int_least8_t)(INT8_MIN));

        if ((ref > 0) && (buffer->__buf != NULL) && !(buffer->flags & TS_IMPL_BUF_EXTERNAL_DATA)) {
            free(buffer->__buf);
        }

        buffer->next = NULL;
        buffer->flags = 0;
        buffer->size  = 0;
        buffer->len = 0;
        buffer->data = NULL;
        buffer->__buf = NULL;

        atomic_exchange(&buffer->ref, (int_least8_t)(0));
    }
    return 0;
}
