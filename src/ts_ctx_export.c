/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ts_msg.h"
#include "ts_ctx.h"

int thingset_export_buf(thingset_locid_t locid, uint16_t subsets, uint8_t *buf, uint16_t buf_size)
{
    struct thingset_msg *msg;
    int ret;

    if (ts_ctx_protocol_is_bin(locid)) {
        ret = thingset_msg_alloc_cbor(buf_size, THINGSET_TIMEOUT_IMMEDIATE, &msg);
        if (ret != 0) {
            return ret;
        }
        ret = ts_msg_add_export_cbor(msg, ts_ctx_obj_db(locid), subsets);
    }
    else {
        ret = thingset_msg_alloc_json(buf_size, THINGSET_TIMEOUT_IMMEDIATE, &msg);
        if (ret != 0) {
            return ret;
        }
        ret = ts_msg_add_export_json(msg, ts_ctx_obj_db(locid), subsets);
    }
    if (ret != 0) {
        goto ts_ctx_export_buf_error;
    }

#if TS_CONFIG_LOG
    char log_buf[200];
    ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
    TS_LOGD("CTX: %s created message: >%d<, >%.*s<", __func__, ts_msg_len(msg), ret, &log_buf[0]);
#endif

    if (ts_msg_len(msg) > buf_size) {
        ret = -ENOMEM;
        goto ts_ctx_export_buf_error;
    }
    ret = ts_msg_len(msg);
    (void)memcpy(buf, ts_msg_data(msg), ret);

ts_ctx_export_buf_error:
    thingset_msg_unref(msg);
    return ret;
}

int thingset_import_buf(thingset_locid_t locid, uint8_t *buf, uint16_t buf_len,
                        uint16_t auth_flags, uint16_t subsets)
{
    struct thingset_msg *msg;
    int ret = thingset_msg_alloc(buf_len, THINGSET_TIMEOUT_IMMEDIATE, &msg);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_add_mem(msg, buf, buf_len);
    if (ret != 0) {
        thingset_msg_unref(msg);
        return ret;
    }
    ts_msg_auth_set(msg, auth_flags);

#if TS_CONFIG_LOG
    char log_buf[200];
    ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
    TS_LOGD("CTX: %s created message: >%d<, >%.*s<", __func__, ts_msg_len(msg), ret, &log_buf[0]);
#endif

    return ts_msg_pull_export(msg, ts_ctx_obj_db(locid));
}
