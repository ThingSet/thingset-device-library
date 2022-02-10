/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet context message support
 * --------------------------------
 */

#include "ts_msg.h"
#include "ts_ctx.h"

int thingset_add_statement_buf(thingset_locid_t locid, thingset_oref_t oref, uint8_t *buf,
                               uint16_t buf_size)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "MSG: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    struct thingset_msg *msg;
    int ret;

    if (ts_ctx_protocol_is_bin(locid)) {
        ret = thingset_msg_alloc_cbor(buf_size, THINGSET_TIMEOUT_IMMEDIATE, &msg);
        if (ret != 0) {
            return ret;
        }
        ret = ts_msg_add_statement_cbor(msg, oref);
    }
    else {
        ret = thingset_msg_alloc_json(buf_size, THINGSET_TIMEOUT_IMMEDIATE, &msg);
        if (ret != 0) {
            return ret;
        }
        ret = ts_msg_add_statement_json(msg, oref);
    }
    if (ret != 0) {
        goto ts_ctx_msg_add_statement_buf_error;
    }

#if TS_CONFIG_LOG
    char log_buf[200];
    ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
    TS_LOGD("ThingSet: %s created message: >%d<, >%.*s<", __func__, ts_msg_len(msg), ret, &log_buf[0]);
#endif

    if (ts_msg_len(msg) > buf_size) {
        TS_LOGE("ThingSet: %s message does not fit into buffer: >%u< vs. >%u<", __func__,
                (unsigned int)ts_msg_len(msg), (unsigned int)buf_size);
        ret = -ENOMEM;
        goto ts_ctx_msg_add_statement_buf_error;
    }

    ret = ts_msg_len(msg);
    (void)memcpy(buf, ts_msg_data(msg), ret);

ts_ctx_msg_add_statement_buf_error:
    thingset_msg_unref(msg);
    return ret;
}
