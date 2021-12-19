/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet message core support
 * -----------------------------
 */

#include "thingset_env.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ts_buf.h"
#include "ts_msg.h"
#include "ts_obj.h"


#define MAX_SIZE(a, b) (((a) > (b)) ? (a) : (b))

/**
 * @brief Maximum buffer size needed for message scratchpads.
 */
const uint16_t ts_msg_scratchpad_size_max = sizeof(struct ts_msg_buf_scratchroom) +
    MAX_SIZE(sizeof(struct ts_msg_proc_scratchroom),
    MAX_SIZE(sizeof(struct ts_msg_json_enc_scratchroom) + 1,
    MAX_SIZE(sizeof(struct ts_msg_json_dec_scratchroom) + 16 * sizeof(struct ts_jsmn_token),
    MAX_SIZE(sizeof(struct ts_msg_cbor_enc_scratchroom),
             sizeof(struct ts_msg_cbor_dec_scratchroom)))));

/**
 * @brief Maximum buffer size needed for raw message scratchpads.
 */
const uint16_t ts_msg_scratchpad_size_max_raw = sizeof(struct ts_msg_buf_scratchroom) +
                                                sizeof(struct ts_msg_proc_scratchroom);

/**
 * @brief Maximum buffer size needed for binary message scratchpads.
 */
const uint16_t ts_msg_scratchpad_size_max_cbor = sizeof(struct ts_msg_buf_scratchroom) +
    MAX_SIZE(sizeof(struct ts_msg_cbor_enc_scratchroom),
             sizeof(struct ts_msg_cbor_dec_scratchroom));

/**
 * @brief Maximum buffer size needed for text message scratchpads.
 */
const uint16_t ts_msg_scratchpad_size_max_json = sizeof(struct ts_msg_buf_scratchroom) +
    MAX_SIZE(sizeof(struct ts_msg_json_enc_scratchroom) + 1,
             sizeof(struct ts_msg_json_dec_scratchroom) + 16 * sizeof(struct ts_jsmn_token));

int ts_msg_alloc_raw(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg)
{
    /* Allocate buffer and account for message size + scratchpad size + alignment */
    int ret = ts_buf_alloc(msg_size + ts_msg_scratchpad_size_max_raw + 3, timeout_ms,
                           (struct ts_buf **)msg);
    if (ret != 0) {
        TS_LOGE("MSG: %s faulting - can not alloc message (requested: %u, total %u)",
                __func__, (unsigned int)msg_size,
                (unsigned int)(msg_size + sizeof(struct ts_msg_buf_scratchroom) + 3));
        return ret;
    }
    TS_LOGD("MSG: %s alloc message 0x%" PRIXPTR " (requested: %u, total %u)", __func__,
            (uintptr_t)*msg, (unsigned int)msg_size,
            (unsigned int)(msg_size + sizeof(struct ts_msg_buf_scratchroom) + 3));
    thingset_msg_reset(*msg);
    return 0;
}

int thingset_msg_alloc(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg)
{
    /* Allocate buffer and account for message size + scratchpad size + alignment */
    int ret = ts_buf_alloc(msg_size + ts_msg_scratchpad_size_max + 3, timeout_ms,
                           (struct ts_buf **)msg);
    if (ret != 0) {
        TS_LOGE("MSG: %s faulting - can not alloc message (requested: %u, total %u)",
                __func__, (unsigned int)msg_size,
                (unsigned int)(msg_size + ts_msg_scratchpad_size_max + 3));
        return ret;
    }
    TS_LOGD("MSG: %s alloc message 0x%" PRIXPTR " (requested: %u, total %u)", __func__,
            (uintptr_t)*msg, (unsigned int)msg_size,
            (unsigned int)(msg_size + ts_msg_scratchpad_size_max + 3));
    thingset_msg_reset(*msg);
    return 0;
}

int thingset_msg_alloc_cbor(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg)
{
    /* Allocate buffer and account for message size + scratchpad size + alignment */
    int ret = ts_buf_alloc(msg_size + ts_msg_scratchpad_size_max_cbor + 3, timeout_ms,
                           (struct ts_buf **)msg);
    if (ret != 0) {
        TS_LOGE("MSG: %s faulting - can not alloc message (requested: %u, total %u)",
                __func__, (unsigned int)msg_size,
                (unsigned int)(msg_size + ts_msg_scratchpad_size_max_cbor + 3));
        return ret;
    }
    TS_LOGD("MSG: %s alloc message 0x%" PRIXPTR " (requested: %u, total %u)", __func__,
            (uintptr_t)*msg, (unsigned int)msg_size,
            (unsigned int)(msg_size + ts_msg_scratchpad_size_max_cbor + 3));
    thingset_msg_reset(*msg);
    return 0;
}

int thingset_msg_alloc_json(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg)
{
    /* Allocate buffer and account for message size + scratchpad size + alignment */
    int ret = ts_buf_alloc(msg_size + ts_msg_scratchpad_size_max_json + 3, timeout_ms,
                           (struct ts_buf **)msg);
    if (ret != 0) {
        TS_LOGE("MSG: %s faulting - can not alloc message (requested: %u, total %u)",
                __func__, (unsigned int)msg_size,
                (unsigned int)(msg_size + ts_msg_scratchpad_size_max_json + 3));
        return ret;
    }
    TS_LOGD("MSG: %s alloc message 0x%" PRIXPTR " (requested: %u, total %u)", __func__,
            (uintptr_t)*msg, (unsigned int)msg_size,
            (unsigned int)(msg_size + ts_msg_scratchpad_size_max_json + 3));
    thingset_msg_reset(*msg);
    return 0;
}

int thingset_msg_clone(struct thingset_msg *msg, thingset_time_ms_t timeout_ms, struct thingset_msg **clone)
{
    TS_LOGD("MSG: %s clone message 0x%" PRIXPTR " to 0x%" PRIXPTR, __func__,
            (uintptr_t)msg, (uintptr_t)*clone);
    return ts_buf_clone((struct ts_buf *)msg, timeout_ms, (struct ts_buf **)clone);
}

void thingset_msg_reset(struct thingset_msg *msg)
{
    struct ts_buf *buf = (struct ts_buf *)msg;
    ts_buf_reset(buf);

    /* Initialize message buffer scratchpad */
    struct ts_msg_buf_scratchroom *scratchpad = (struct ts_msg_buf_scratchroom *)ts_buf_data(buf);
    scratchpad->status.valid = TS_MSG_VALID_UNSET;
    scratchpad->status.type = 0;
    scratchpad->status.proto = 0;
    scratchpad->status.encoding = TS_MSG_ENCODING_NONE;
    scratchpad->status.code = 0;
    scratchpad->scratchtype.thingset = TS_MSG_THINGSET;
    scratchpad->scratchtype.id = TS_MSG_SCRATCHPAD_STD;
    scratchpad->scratchroom.size = 0;
    scratchpad->auth = 0;

    ts_buf_reserve(buf, sizeof(struct ts_msg_buf_scratchroom));

    TS_MSG_ASSERT(msg);
    TS_LOGD("MSG: %s reset message 0x%" PRIXPTR, __func__, (uintptr_t)msg);
}

int thingset_msg_ref(struct thingset_msg *msg)
{
    TS_LOGD("MSG: %s ref message 0x%" PRIXPTR, __func__, (uintptr_t)msg);
    return ts_buf_ref((struct ts_buf *)msg);
}

int thingset_msg_unref(struct thingset_msg *msg)
{
    TS_LOGD("MSG: %s unref message 0x%" PRIXPTR, __func__, (uintptr_t)msg);
    return ts_buf_unref((struct ts_buf *)msg);
}
