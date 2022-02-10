/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet message export and import support
 * ------------------------------------------
 */

#include "ts_msg.h"
#include "ts_obj.h"
#include "ts_ctx.h"

int ts_msg_add_export_cbor(struct thingset_msg* msg, ts_obj_db_id_t did, uint16_t subsets)
{
    /* mark export message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST,
                      TS_MSG_CODE_EXPORT);
    int ret = ts_msg_add_response_status_cbor(msg);
    if (ret != 0) {
        return ret;
    }

    // find out number of elements to be serialized
    int num_elements = 0;
    thingset_oref_t oref = { .db_id = did };
    for (oref.db_oid = 0; ts_obj_db_oref_is_valid(oref); oref.db_oid++) {
        if (ts_obj_subsets(oref) & subsets) {
            num_elements++;
        }
    }

    ret = ts_msg_add_map_cbor(msg, num_elements);
    if (ret != 0) {
        return ret;
    }

    for (oref.db_oid = 0; ts_obj_db_oref_is_valid(oref) && (ret == 0); oref.db_oid++) {
        if (ts_obj_subsets(oref) & subsets) {
            ret = ts_msg_add_u16_cbor(msg, ts_obj_id(oref));
            if (ret != 0) {
                break;
            }
            ret = ts_msg_add_object_cbor(msg, oref);
        }
    }

    if (ret == 0) {
        ret = ts_msg_add_map_end_cbor(msg);
    }

    return ret;
}

int ts_msg_add_export_json(struct thingset_msg* msg, ts_obj_db_id_t did, uint16_t subsets)
{
    /* mark export message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST,
                      TS_MSG_CODE_EXPORT);
    int ret = ts_msg_add_response_status_json(msg);
    if (ret != 0) {
        return ret;
    }

    ret = ts_msg_add_u8(msg, ' ');
    if (ret != 0) {
        return ret;
    }

    // find out number of elements to be serialized
    int num_elements = 0;
    thingset_oref_t oref = { .db_id = did };
    for (oref.db_oid = 0; ts_obj_db_oref_is_valid(oref); oref.db_oid++) {
        if (ts_obj_subsets(oref) & subsets) {
            num_elements++;
        }
    }

    ret = ts_msg_add_map_json(msg, num_elements);
    if (ret != 0) {
        return ret;
    }

    for (oref.db_oid = 0; ts_obj_db_oref_is_valid(oref) && (ret == 0); oref.db_oid++) {
        if (ts_obj_subsets(oref) & subsets) {
            ret = ts_msg_add_string_json(msg, ts_obj_name(oref));
            if (ret != 0) {
                break;
            }
            ret = ts_msg_add_object_json(msg, oref);
        }
    }

    if (ret == 0) {
        ret = ts_msg_add_map_end_json(msg);
    }

    return ret;
}

int ts_msg_pull_export(struct thingset_msg* msg, ts_obj_db_id_t did)
{
    int ret;

    uint8_t method_id = ts_msg_data(msg)[0];
    if (method_id == ':') {
        /* JSON */
        ts_msg_pull(msg, 1);
        ts_msg_status_code_t code;
        const char *description;
        uint16_t len;
        ret = ts_msg_pull_status(msg, &code, &description, &len);
        if ((ret == 0) && (code != TS_MSG_CODE_EXPORT)) {
            ret = -EINVAL;
        }
        if (ret == 0) {
            ret = ts_msg_json_dec_setup(msg);
        }
    }
    else if (method_id != TS_MSG_CODE_EXPORT) {
        ret = -EINVAL;
    }
    else {
        /* CBOR */
        ts_msg_pull(msg, 1);
        ret = ts_msg_cbor_dec_setup(msg);
    }
    if (ret != 0) {
        return ret;
    }

    struct thingset_msg *response = NULL;
    thingset_oref_t oref = { .db_id = did };
    if (method_id == TS_MSG_CODE_EXPORT) {
        /* CBOR */
        ret = ts_ctx_process_patch_cbor(msg, oref, &response);
    }
    else {
        /* JSON */
        ret = ts_ctx_process_patch_json(msg, oref, &response);
    }

    if (response != NULL) {
        thingset_msg_unref(response);
    }
    return ret;
}

int ts_msg_load(struct thingset_msg *msg, const uint8_t *buf, uint16_t buf_len)
{
    thingset_msg_reset(msg);

    return ts_msg_add_mem(msg, buf, buf_len);
}

int ts_msg_save(struct thingset_msg *msg, uint8_t *buf, uint16_t *buf_len)
{
    if (*buf_len < ts_msg_len(msg)) {
        return -ENOMEM;
    }
    *buf_len = ts_msg_len(msg);
    (void)memcpy(buf, ts_msg_data(msg), *buf_len);
    return 0;
}
