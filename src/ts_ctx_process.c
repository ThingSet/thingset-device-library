/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet context (receive) message processing
 * ---------------------------------------------
 */

#include "thingset_env.h"
#include "thingset_time.h"

#include "ts_obj.h"
#include "ts_ctx.h"
#include "ts_msg.h"
#include "ts_macro.h"

#include <string.h>


int ts_ctx_process_lock(thingset_locid_t locid)
{
    int ret;

    if (TS_CTX_IS_CORE(locid)) {
        struct ts_ctx_core_data *data = ts_ctx_core_data(locid);
        ret = pthread_mutex_lock(&data->common.process_mutex);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        ret = pthread_mutex_lock(&data->common.process_mutex);
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

int ts_ctx_process_unlock(thingset_locid_t locid)
{
    int ret;

    if (TS_CTX_IS_CORE(locid)) {
        struct ts_ctx_core_data *data = ts_ctx_core_data(locid);
        ret = pthread_mutex_unlock(&data->common.process_mutex);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        ret = pthread_mutex_unlock(&data->common.process_mutex);
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

int thingset_process(thingset_locid_t locid, struct thingset_msg *msg)
{
    struct thingset_msg *msg_out = NULL;

    int ret = ts_ctx_process_msg(locid, msg, &msg_out);

    if ((ret != 0) || (msg_out == NULL)) {
        return ret;
    }

    thingset_portid_t port_id_dest = ts_msg_proc_get_port_dest(msg_out);
    ret = ts_ctx_transmit(locid, port_id_dest, msg_out, THINGSET_TIMEOUT_FOREVER);

    return ret;
}

int ts_ctx_process_msg(thingset_locid_t locid, struct thingset_msg *msg_in,
                       struct thingset_msg **msg_out)
{
    TS_MSG_ASSERT_SCRATCHTYPE(msg_in, TS_MSG_SCRATCHPAD_PROC);

    int ret = ts_ctx_process_lock(locid);
    if (ret != 0) {
        return ret;
    }

    /* Update message status */
    if (ts_msg_status(msg_in).valid == TS_MSG_VALID_UNSET) {
        uint8_t command_id = *ts_msg_data(msg_in);
        if (strchr("?=+-!#:", command_id) != NULL) {
            /* Text mode message */
            if (command_id == '#') {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_STATEMENT,
                                  0);
            }
            else if (command_id == ':') {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                                  0);
            }
            else {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST,
                                  0);
            }
        }
        else if ((strchr("\x01\x02\x04\x05\x07\x1F", command_id) != NULL) ||
                 (command_id >= 0x80U)) {
            /* Binary mode message */
            if (command_id == TS_MSG_CODE_BIN_STATEMENT) {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_STATEMENT,
                                  0);
            }
            else if (command_id >= 0x80U) {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                                  command_id);
            }
            else {
                ts_msg_status_set(msg_in, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST,
                                  0);
            }
        }
        else {
            TS_ASSERT(false, "%s can not process message with command 0x%.2u '%c'", __func__,
                      (unsigned int)command_id, (char)command_id);
            ret = -ENOTSUP;
            goto ts_ctx_process_msg_unlock;
        }
    }

    /* Remember current state of ThingSet objects authorisation flags in message */
    ts_msg_auth_set(msg_in, thingset_authorisation(locid));

    /* Save processing info - msg_in may be consumed by processing */
    thingset_portid_t port_id_src = ts_msg_proc_get_port_src(msg_in);
    thingset_portid_t port_id_dest = ts_msg_proc_get_port_dest(msg_in);

    if (TS_CTX_IS_COM(locid) && ts_msg_proc_get_port_dest(msg_in) != THINGSET_PORT_ID_INVALID) {
        /* We have a destination port - just route the message to the port */
        *msg_out = msg_in;

        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        if (ts_msg_status_type(msg_in) == TS_MSG_TYPE_REQUEST) {
            /*
            * An application connected to source port is doing a request to another port.
            * Link the two ports for the given unique context identifier until we got a response.
            */
            uint16_t node_idx;
            ret = ts_ctx_node_get(locid, ts_msg_proc_get_ctx_uid(msg_in), &node_idx);
            if (ret != 0) {
                goto ts_ctx_process_msg_unlock;
            }
            /* Check whether we have an ongoing request to this node */
            /** @todo check ongoing request */
            if (false) {
                ret = -EBUSY;
                goto ts_ctx_process_msg_unlock;
            }
            /* Link the ports */
            data->node_table.nodes[node_idx].ctx_uid = *ts_msg_proc_get_ctx_uid(msg_in);
            data->node_table.nodes[node_idx].port_id = port_id_dest;
            data->node_table.nodes[node_idx].response_port_id = port_id_src;
            data->node_table.nodes[node_idx].last_seen_time = thingset_time_ms();
        }
    }
    /*
     * Core context always works on this context.
     * Communication context with no destination port also works on this context.
     */
    else if (ts_msg_status_type(msg_in) == TS_MSG_TYPE_REQUEST) {
        /* Text or binary mode request to this context */
        ret = ts_ctx_process_request(locid, msg_in, msg_out);
        if (ret != 0) {
            goto ts_ctx_process_msg_unlock;
        }
        if (*msg_out != NULL) {
            ret = ts_msg_proc_setup(*msg_out);
            if (ret != 0) {
                goto ts_ctx_process_msg_unlock;
            }
            if (TS_CTX_IS_COM(locid)) {
                ts_msg_proc_set_ctx_uid(*msg_out, ts_ctx_uid(locid));
                ts_msg_proc_set_port_src(*msg_out, THINGSET_PORT_ID_INVALID);
                ts_msg_proc_set_port_dest(*msg_out, port_id_src);
            }
            else if (TS_CTX_IS_CORE(locid)) {
                ts_msg_proc_set_port_src(*msg_out, THINGSET_PORT_ID_INVALID);
                ts_msg_proc_set_port_dest(*msg_out, THINGSET_PORT_ID_INVALID);
            }
        }
    }
    else if (TS_CTX_IS_COM(locid)) {
        /* Success or error response message - look who is waiting for this response message */
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);

        uint16_t node_idx;
        ret = ts_ctx_node_lookup(locid, ts_msg_proc_get_ctx_uid(msg_in), &node_idx);
        if (ret != 0) {
            /* Nobody is waiting - just consume */
            thingset_msg_unref(msg_in);
            *msg_out = NULL;
            ret = 0;
            goto ts_ctx_process_msg_unlock;
        }
        /* Update last seen */
        data->node_table.nodes[node_idx].last_seen_time = thingset_time_ms();

        /* Transfer message to application  port */
        *msg_out = msg_in;
        ts_msg_proc_set_port_dest(*msg_out, data->node_table.nodes[node_idx].response_port_id);
    }
    else {
        TS_ASSERT(false, "%s can not process success or error response message with other than"
                  " communication context", __func__);
        ret = -ENOTSUP;
    }

ts_ctx_process_msg_unlock:
    int ret_unlock = ts_ctx_process_unlock(locid);
    if (ret == 0) {
        ret = ret_unlock;
    }

    return ret;
}

int thingset_process_buf(thingset_locid_t locid, const uint8_t *request, size_t request_len,
                         uint8_t *response, size_t response_size)
{
    struct thingset_msg *msg = NULL;
    int ret = 0;

    if (request[0] < 0x20) {
        ret = thingset_msg_alloc_cbor((uint16_t)request_len + 1, THINGSET_TIMEOUT_IMMEDIATE, &msg);
    }
    else {
        ret = thingset_msg_alloc_json((uint16_t)request_len + 1, THINGSET_TIMEOUT_IMMEDIATE, &msg);
    }
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_proc_setup(msg);
    if (ret != 0) {
        goto ts_ctx_process_buf_error;
    }
    ret = ts_msg_add_mem(msg, request, (uint16_t)request_len);
    if (ret != 0) {
        goto ts_ctx_process_buf_error;
    }
    if (ts_msg_tailroom(msg) == 0) {
        goto ts_ctx_process_buf_error;
    }
    /* Add hint of response message size to request */
    ts_msg_proc_set_resp_size(msg, response_size);
    /* Assure string end for JSON type messages */
    *ts_msg_tail(msg) = '\0';

#if TS_CONFIG_LOG
    char log_buf[200];
    ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
    TS_LOGD("%s processes message: >%d<, >%.*s<", __func__, ts_msg_len(msg), ret, &log_buf[0]);
#endif

    struct thingset_msg *msg_out = NULL;
    ret = ts_ctx_process_msg(locid, msg, &msg_out);
    if (ret != 0) {
        if (msg_out != NULL) {
            thingset_msg_unref(msg_out);
        }
        goto ts_ctx_process_buf_error;
    }

    unsigned int response_len = 0;
    if (msg_out != NULL) {
#if TS_CONFIG_LOG
        ret = ts_msg_log(msg_out, &log_buf[0], sizeof(log_buf));
        TS_LOGD("%s created reponse: >%d<, >%.*s<", __func__, ts_msg_len(msg_out), ret,
                &log_buf[0]);
#endif
        if (ts_msg_len(msg_out) > response_size) {
            ret = -ENOMEM;
        }
        else {
            response_len = ts_msg_len(msg_out);
            memcpy(response, ts_msg_data(msg_out), response_len);
            ret = response_len;
        }
        thingset_msg_unref(msg_out);
    }
    if (response_size > response_len) {
        /* Assure string end for JSON type reponse messages */
        response[response_len] = '\0';
    }
    return ret;

ts_ctx_process_buf_error:
    response[0] = '\0';
    thingset_msg_unref(msg);
    return ret;
}

int ts_ctx_process_request(thingset_locid_t locid, struct thingset_msg *request,
                           struct thingset_msg **response)
{
    TS_ASSERT(ts_msg_status_type(request) == TS_MSG_TYPE_REQUEST,
              "ThingSet: %s unexpected message type (%d)", __func__,
              (int)ts_msg_status_type(request));

    int ret;
    thingset_oref_t oref = ts_obj_db_oref_any(ts_ctx_obj_db(locid));
    uint16_t response_size_hint = ts_msg_proc_get_resp_size(request);

    if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
        ret = ts_msg_pull_request_cbor(request, &oref);
    }
    else if (ts_msg_status_proto(request) == TS_MSG_PROTO_TXT) {
        ret = ts_msg_pull_request_json(request, &oref);
    }
    else {
        /* Not a ThingSet command --> ignore and set response to None */
        *response = NULL;
        return 0;
    }
    if (ret != 0) {
        return ret;
    }

    if (ts_msg_status_valid(request) != TS_MSG_VALID_OK) {
        /* Request is faulty */
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            ts_msg_status_code(request), response);
    }

    switch (ts_msg_status_code(request)) {
    case TS_MSG_CODE_REQUEST_CREATE:
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = ts_ctx_process_create_cbor(request, oref, response);
        } else {
            ret = ts_ctx_process_create_json(request, oref, response);
        }
        break;
    case TS_MSG_CODE_REQUEST_DELETE:
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = ts_ctx_process_delete_cbor(request, oref, response);
        } else {
            ret = ts_ctx_process_delete_json(request, oref, response);
        }
        break;
    case TS_MSG_CODE_REQUEST_EXEC:
        TS_ASSERT(ts_obj_db_oref_is_object(oref),
                  "OBJ: %s EXEC on invalid object reference (%u:%u)",
                  __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = ts_ctx_process_exec_cbor(request, oref, response);
        } else {
            ret = ts_ctx_process_exec_json(request, oref, response);
        }
        if ((ret == 0) && (ts_msg_status_code(*response) == TS_MSG_CODE_VALID)) {
            /* Call it if processing succeeded */
            ts_obj_exec_t exec = ts_obj_exec_data(oref);
            TS_ASSERT(exec != NULL, "CTX: %s on no exec function (NULL)", __func__);
            (*exec)();
        }
        break;
    case TS_MSG_CODE_REQUEST_FETCH_IDS:
    case TS_MSG_CODE_REQUEST_FETCH_NAMES:
    case TS_MSG_CODE_REQUEST_FETCH_SINGLE:
    case TS_MSG_CODE_REQUEST_FETCH_VALUES:
        if (response_size_hint == 0) {
            response_size_hint = 256;
        }
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = thingset_msg_alloc_cbor(response_size_hint, 0, response);
            if (ret != 0) {
                return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                                    TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
            }
            ret = ts_msg_add_response_fetch_cbor(*response, oref, request);
        } else {
            ret = thingset_msg_alloc_json(response_size_hint, 0, response);
            if (ret != 0) {
                return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                                    TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
            }
            ret = ts_msg_add_response_fetch_json(*response, oref, request);
        }
        thingset_msg_unref(request);
        break;
    case TS_MSG_CODE_REQUEST_GET_IDS:
    case TS_MSG_CODE_REQUEST_GET_IDS_VALUES:
    case TS_MSG_CODE_REQUEST_GET_NAMES:
    case TS_MSG_CODE_REQUEST_GET_NAMES_VALUES:
    case TS_MSG_CODE_REQUEST_GET_VALUES:
        if (response_size_hint == 0) {
            response_size_hint = 256;
        }
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = thingset_msg_alloc_cbor(response_size_hint, 0, response);
            if (ret != 0) {
                return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                                    TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
            }
            ret = ts_msg_add_response_get_cbor(*response, oref, request);
        } else {
            ret = thingset_msg_alloc_json(response_size_hint, 0, response);
            if (ret != 0) {
                return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                                    TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
            }
            ret = ts_msg_add_response_get_json(*response, oref, request);
        }
        thingset_msg_unref(request);
        break;
    case TS_MSG_CODE_REQUEST_PATCH:
        TS_ASSERT(ts_obj_db_oref_is_tree(oref),
                  "OBJ: %s PATCH on invalid object reference (%u:%u)",
                  __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        if (ts_msg_status_proto(request) == TS_MSG_PROTO_BIN) {
            ret = ts_ctx_process_patch_cbor(request, oref, response);
        } else {
            ret = ts_ctx_process_patch_json(request, oref, response);
        }

        if ((ret == 0) && (ts_msg_status_code(*response) == TS_MSG_CODE_CHANGED)) {
            ts_obj_exec_t exec = ts_obj_exec_data(oref);
            if (exec != NULL) {
                /* Call it if processing succeeded and change function available */
                (*exec)();
            }
        }
        break;
    default:
        /* Should never happen */
        TS_ASSERT(false, "Unexpected message status code (0x%02x)",
                  (unsigned int)ts_msg_status_code(request));
        ret = 0;
        break;
    }
    return ret;
}

int ts_ctx_process_request_error(enum ts_msg_proto response_msg_proto,
                                 enum ts_msg_code response_msg_code,
                                 struct thingset_msg **response)
{
    int ret;

    if (response_msg_proto == TS_MSG_PROTO_TXT) {
        if (TS_CONFIG_VERBOSE_STATUS_MESSAGES) {
            ret = ts_msg_alloc_raw(31, 0, response);
            if (ret != 0) {
                ret = ts_msg_alloc_raw(4, 0, response);
            }
        }
        else {
            ret = ts_msg_alloc_raw(4, 0, response);
        }
    }
    else {
        ret = ts_msg_alloc_raw(1, 0, response);
    }
    if (ret != 0) {
        return ret;
    }

    ts_msg_status_set(*response, TS_MSG_VALID_OK, response_msg_proto, TS_MSG_TYPE_RESPONSE,
                      response_msg_code);
    if (response_msg_proto == TS_MSG_PROTO_BIN) {
        ret = ts_msg_add_response_status_cbor(*response);
    }
    else {
        ret = ts_msg_add_response_status_json(*response);
    }

    return ret;
}

int ts_ctx_process_create_cbor(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response)
{
    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_CBOR_DEC);

    int ret = thingset_msg_alloc_json(16, 0, response);
    if (ret != 0) {
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
    }

    if (ts_obj_type(oref) == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                        TS_MSG_CODE_NOT_IMPLEMENTED);
        return ts_msg_add_response_status_cbor(*response);
    }
    else if (ts_obj_type(oref) == TS_T_SUBSET) {
        thingset_oref_t new_oref;
        ts_obj_id_t id;
        const char *name;
        uint16_t name_len;
        ret = ts_msg_pull_u16_cbor(request, &id);
        if (ret == 0) {
            ret = ts_obj_db_oref_by_id(oref.db_id, id, &new_oref);
            if (ret != 0) {
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                return ts_msg_add_response_status_cbor(*response);
            }
        }
        else {
            ret = ts_msg_pull_string_cbor(request, &name, &name_len);
            if (ret == 0) {
                ret = ts_obj_by_name(oref, name, name_len, &new_oref);
                if (ret != 0) {
                    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                      TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                    return ts_msg_add_response_status_cbor(*response);
                }
            }
        }
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_cbor(*response);
        }

        ret = ts_obj_subsets_add(new_oref, (uint16_t)ts_obj_detail(oref));
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_INTERNAL_SERVER_ERR);
            return ts_msg_add_response_status_cbor(*response);
        }

        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_CREATED);
        return ts_msg_add_response_status_cbor(*response);
    }

    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_METHOD_NOT_ALLOWED);
    return ts_msg_add_response_status_cbor(*response);
}

int ts_ctx_process_create_json(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response)
{
    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_JSON_DEC);

    int ret = thingset_msg_alloc_json(16, 0, response);
    if (ret != 0) {
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
    }

    if (ts_obj_type(oref) == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_NOT_IMPLEMENTED);
        return ts_msg_add_response_status_json(*response);
    }
    else if (ts_obj_type(oref) == TS_T_SUBSET) {
        thingset_oref_t new_oref;
        const char *name;
        uint16_t name_len;
        ret = ts_msg_pull_string_json(request, &name, &name_len);
        if (ret == 0) {
            ret = ts_obj_by_name(ts_obj_db_oref_any(oref.db_id), name, name_len, &new_oref);
            if (ret != 0) {
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                return ts_msg_add_response_status_json(*response);
            }
        }
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_json(*response);
        }

        ret = ts_obj_subsets_add(new_oref, (uint16_t)ts_obj_detail(oref));
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_INTERNAL_SERVER_ERR);
            return ts_msg_add_response_status_cbor(*response);
        }

        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_CREATED);
        return ts_msg_add_response_status_json(*response);
    }

    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_METHOD_NOT_ALLOWED);
    return ts_msg_add_response_status_json(*response);
}

int ts_ctx_process_delete_cbor(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response)
{
    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_CBOR_DEC);

    int ret = thingset_msg_alloc_json(16, 0, response);
    if (ret != 0) {
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
    }

    if (ts_obj_type(oref) == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                        TS_MSG_CODE_NOT_IMPLEMENTED);
        return ts_msg_add_response_status_cbor(*response);
    }
    else if (ts_obj_type(oref) == TS_T_SUBSET) {
        thingset_oref_t del_oref;
        ts_obj_id_t id;
        const char *name;
        uint16_t name_len;
        ret = ts_msg_pull_u16_cbor(request, &id);
        if (ret == 0) {
            ret = ts_obj_db_oref_by_id(oref.db_id, id, &del_oref);
            if (ret != 0) {
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                return ts_msg_add_response_status_cbor(*response);
            }
        }
        else {
            ret = ts_msg_pull_string_cbor(request, &name, &name_len);
            if (ret == 0) {
                ret = ts_obj_by_name(oref, name, name_len, &del_oref);
                if (ret != 0) {
                    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                      TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                    return ts_msg_add_response_status_cbor(*response);
                }
            }
        }
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_cbor(*response);
        }

        ret = ts_obj_subsets_remove(del_oref, (uint16_t)ts_obj_detail(oref));
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_INTERNAL_SERVER_ERR);
            return ts_msg_add_response_status_cbor(*response);
        }

        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_DELETED);
        return ts_msg_add_response_status_cbor(*response);
    }

    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_METHOD_NOT_ALLOWED);
    return ts_msg_add_response_status_cbor(*response);
}

int ts_ctx_process_delete_json(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response)
{
    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_JSON_DEC);

    int ret = thingset_msg_alloc_json(16, 0, response);
    if (ret != 0) {
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
    }

    if (ts_obj_type(oref) == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                        TS_MSG_CODE_NOT_IMPLEMENTED);
        return ts_msg_add_response_status_json(*response);
    }
    else if (ts_obj_type(oref) == TS_T_SUBSET) {
        thingset_oref_t del_oref;
        const char *name;
        uint16_t name_len;
        ret = ts_msg_pull_string_json(request, &name, &name_len);
        if (ret == 0) {
            ret = ts_obj_by_name(ts_obj_db_oref_any(oref.db_id), name, name_len, &del_oref);
            if (ret != 0) {
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_NOT_FOUND);
                return ts_msg_add_response_status_json(*response);
            }
        }
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_json(*response);
        }

        ret = ts_obj_subsets_remove(del_oref, (uint16_t)ts_obj_detail(oref));
        if (ret != 0) {
            ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                              TS_MSG_CODE_INTERNAL_SERVER_ERR);
            return ts_msg_add_response_status_cbor(*response);
        }

        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_DELETED);
        return ts_msg_add_response_status_json(*response);
    }

    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_METHOD_NOT_ALLOWED);
    return ts_msg_add_response_status_json(*response);
}

int ts_ctx_process_exec_cbor(struct thingset_msg *request, thingset_oref_t oref,
                             struct thingset_msg **response)
{
    int ret = ts_ctx_process_set_cbor(request, oref, response);
    if (ret != 0) {
        return ret;
    }

    /* EXEC reports TS_MSG_CODE_VALID on success - set reports TS_MSG_CODE_CHANGED */
    if ((ts_msg_status_valid(*response) == TS_MSG_VALID_OK) &&
        (ts_msg_status_code(*response) == TS_MSG_CODE_CHANGED)) {
        thingset_msg_reset(*response);
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_VALID);
        return ts_msg_add_response_status_cbor(*response);
    }
    return 0;
}

int ts_ctx_process_exec_json(struct thingset_msg *request, thingset_oref_t oref,
                             struct thingset_msg **response)
{
    int ret = ts_ctx_process_set_json(request, oref, response);
    if (ret != 0) {
        return ret;
    }

    /* EXEC reports TS_MSG_CODE_VALID on success - set reports TS_MSG_CODE_CHANGED */
    if ((ts_msg_status_valid(*response) == TS_MSG_VALID_OK) &&
        (ts_msg_status_code(*response) == TS_MSG_CODE_CHANGED)) {
        thingset_msg_reset(*response);
        ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_VALID);
        return ts_msg_add_response_status_json(*response);
    }
    return 0;
}

int ts_ctx_process_get_cbor(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response)
{
    return 0;
}

int ts_ctx_process_get_json(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response)
{
    return 0;
}

int ts_ctx_process_patch_cbor(struct thingset_msg *request, thingset_oref_t oref,
                              struct thingset_msg **response)
{
    return ts_ctx_process_set_cbor(request, oref, response);
}

int ts_ctx_process_patch_json(struct thingset_msg *request, thingset_oref_t oref,
                              struct thingset_msg **response)
{
    return ts_ctx_process_set_json(request, oref, response);
}

int ts_ctx_process_set_cbor(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(oref),
              "CTX: %s on invalid object reference (%u:%u)", __func__,
              (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    int ret = thingset_msg_alloc_cbor(16, 0, response);
    if (ret != 0) {
        return ts_ctx_process_request_error(ts_msg_status_proto(request),
                                            TS_MSG_CODE_INTERNAL_SERVER_ERR, response);
    }

    /*
     * Check and validate request format
     * ---------------------------------
     */
    uint16_t child_count;
    ret = ts_obj_child_count(oref, &child_count);
    TS_ASSERT(ret == 0, "CTX: %s calling ts_obj_child_count() fails (%d) - should never fail",
              __func__, ret);

    if (child_count == 0) {
        if (ts_msg_len(request) == 0) {
            if (ts_obj_type(oref) == TS_T_EXEC) {
                /* void function */
                goto ts_process_set_cbor_end;
            }
            /* For all other types we expect a value */
            TS_LOGD("CTX: %s on object %s with %d children but no value provided", __func__,
                    ts_obj_name(oref), (unsigned int)child_count);
            goto ts_process_set_cbor_bad_request;
        }
    }

    struct ts_msg_state request_state;
    ts_msg_state_save(request, &request_state);

    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_CBOR_DEC);

    bool set_by_map = false;
    bool set_by_array = false;
    uint16_t num_elements;
    if (child_count == 0) {
        if (ts_obj_type(oref) == TS_T_EXEC) {
            /* Special case - None parameters may be indicated by empty array */
            ret = ts_msg_pull_array_cbor(request, &num_elements);
            if ((ret != 0) || (num_elements != 0)) {
                TS_LOGD("CTX: %s on object %s with %d children but value provided", __func__,
                        ts_obj_name(oref), (unsigned int)child_count);
                goto ts_process_set_cbor_bad_request;
            }
            /* void function */
            goto ts_process_set_cbor_end;
        }
        else {
            TS_ASSERT(!(ts_obj_type(oref) == TS_T_GROUP) &&
                    !(ts_obj_type(oref) == TS_T_SUBSET),
                    "CTX: %s on %s%s %s with no children (0)", __func__,
                    ts_obj_type(oref) == TS_T_GROUP ? "group" : "",
                    ts_obj_type(oref) == TS_T_SUBSET ? "subset" : "",
                    ts_obj_name(oref));
            /* No children - set value of object itself */
            num_elements = 1;
        }
    }
    else if (child_count == 1) {
        /* One child - value may be given by array with one element or by map or by value itself */
        ret = ts_msg_pull_array_cbor(request, &num_elements);
        if (ret == 0) {
            if (num_elements != 1) {
                TS_LOGD("CTX: %s on object %s with %d children but array "
                        "with %u elements provided", __func__, ts_obj_name(oref),
                        (unsigned int)child_count, (unsigned int)num_elements);
                goto ts_process_set_cbor_bad_request;
            }
            set_by_array = true;
        }
        else {
            ret = ts_msg_pull_map_cbor(request, &num_elements);
            if (ret == 0) {
                if (num_elements != 1) {
                    TS_LOGD("CTX: %s on object %s with %d children but map "
                            "with %u elements provided", __func__, ts_obj_name(oref),
                            (unsigned int)child_count, (unsigned int)num_elements);
                    goto ts_process_set_cbor_bad_request;
                }
                set_by_map = true;
            }
            else {
                ret = 0;
                num_elements = 1;
            }
        }
    }
    else {
        /* child_count > 1 - value may be given by map only */
        ret = ts_msg_pull_map_cbor(request, &num_elements);
        if (ret != 0) {
            TS_LOGD("CTX: %s on object %s with %d children but no map provided", __func__,
                    ts_obj_name(oref), (unsigned int)child_count);
            goto ts_process_set_cbor_bad_request;
        }
        set_by_map = true;
    }

    /*
     * Loop through all elements to check if request is valid
     * ------------------------------------------------------
     */
    TS_ASSERT(ret == 0, "ThingSet: %s should handle all errors before here (%d)",  __func__, ret);
    thingset_oref_t child_oref;
    for (uint16_t count = 0;(count < num_elements) && (ret == 0); count++) {
        /* Get child oref */
        if (set_by_map) {
            /* Check whether object is in database */
            const char *name;
            uint16_t name_len;
            ret = ts_msg_pull_string_cbor(request, &name, &name_len);
            if (ret == 0) {
                /* We got the object name */
                ret = ts_obj_by_name(oref, name, name_len, &child_oref);
                if (ret != 0) {
                    goto ts_process_set_cbor_not_found;
                }
            }
            else {
                ts_obj_id_t obj_id;
                ret = ts_msg_pull_u16_cbor(request, &obj_id);
                if (ret != 0) {
                    TS_LOGD("ThingSet: %s set by unsupported key format - object: %s", __func__,
                            ts_obj_name(child_oref));
                    goto ts_process_set_cbor_unsupported_format;
                }
                ret = ts_obj_db_oref_by_id(oref.db_id, obj_id, &child_oref);
                if (ret != 0) {
                    goto ts_process_set_cbor_not_found;
                }
            }
        }
        else if (set_by_array) {
            if (count == 0) {
                if (child_count == 0) {
                    child_oref = oref;
                }
                else {
                    ret = ts_obj_child_first(oref, &child_oref);
                    TS_ASSERT(ret == 0,
                        "ThingSet: %s calling ts_obj_child_next() fails (%d) - should never fail",
                        __func__, ret);
                }
            }
            else {
                ret = ts_obj_child_next(&child_oref);
                TS_ASSERT(ret == 0,
                          "ThingSet: %s calling ts_obj_child_next() fails (%d) - should never fail",
                            __func__, ret);
            }
        }
        else if (count == 0) {
            /* no map, no array -> simple object */
            if (child_count == 0) {
                child_oref = oref;
            }
            else {
                ret = ts_obj_child_first(oref, &child_oref);
                TS_ASSERT(ret == 0,
                    "ThingSet: %s calling ts_obj_child_first() fails (%d) - should never fail",
                    __func__, ret);
            }
        }
        else {
            /* count > 0 */
            TS_ASSERT(false, "ThingSet: %s count > 0 - should never happen for object set",
                      __func__);
            goto ts_process_set_cbor_bad_request;
        }

        /* Check access */
        if (!ts_obj_access_write(child_oref, ts_msg_auth(request))) {
            if (ts_obj_access_write(child_oref, TS_WRITE_MASK)) {
                TS_LOGD("%s: set not authorized: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)ts_msg_auth(request), ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                thingset_msg_unref(request);
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_UNAUTHORIZED);
                return ts_msg_add_response_status_cbor(*response);
            }
            else {
                TS_LOGD("%s: set forbidden: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)ts_msg_auth(request), ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                thingset_msg_unref(request);
                ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN,
                                  TS_MSG_TYPE_RESPONSE, TS_MSG_CODE_FORBIDDEN);
                return ts_msg_add_response_status_cbor(*response);
            }
        }

        /* Check object type and available buffer length vs. value provided */
        ret = ts_msg_pull_object_cbor(request, child_oref, false);
        if (ret != 0) {
            TS_LOGD("ThingSet: %s set by unsupported value format - object: %s", __func__,
                    ts_obj_name(child_oref));
            goto ts_process_set_cbor_unsupported_format;
        }
    }
    if (set_by_map) {
        ret = ts_msg_pull_map_end_cbor(request);
    }
    else if (set_by_array) {
        ret = ts_msg_pull_array_end_cbor(request);
    }
    if (ret != 0) {
        goto ts_process_set_cbor_bad_request;
    }

    /*
     * Actually write data
     * -------------------
     */
    ts_msg_state_restore(request, &request_state);
    ret = ts_msg_cbor_dec_setup(request);
    TS_ASSERT(ret == 0, "Scratchpad init fail after message state restore");

    if (set_by_map) {
        (void)ts_msg_pull_map_cbor(request, &num_elements);
    }
    else if (set_by_array) {
        (void)ts_msg_pull_array_cbor(request, &num_elements);
    }
    for (uint16_t count = 0; count < num_elements; count++) {
        /* Get child oref */
        if (set_by_map) {
            /* Check whether object is in database */
            const char *name;
            uint16_t name_len;
            ret = ts_msg_pull_string_cbor(request, &name, &name_len);
            if (ret == 0) {
                /* We got the object name */
                (void)ts_obj_by_name(oref, name, name_len, &child_oref);
            }
            else {
                ts_obj_id_t obj_id;
                (void)ts_msg_pull_u16_cbor(request, &obj_id);
                (void)ts_obj_db_oref_by_id(oref.db_id, obj_id, &child_oref);
            }
        }
        else if (set_by_array) {
            if (count == 0) {
                if (child_count == 0) {
                    child_oref = oref;
                }
                else {
                    (void)ts_obj_child_first(oref, &child_oref);
                }
            }
            else {
                (void)ts_obj_child_next(&child_oref);
            }
        }
        else {
            if (child_count == 0) {
                child_oref = oref;
            }
            else {
                (void)ts_obj_child_first(oref, &child_oref);
            }
        }

        (void)ts_msg_pull_object_cbor(request, child_oref, true);
    }

ts_process_set_cbor_end:
    thingset_msg_unref(request);
    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CHANGED);
    return ts_msg_add_response_status_cbor(*response);

ts_process_set_cbor_bad_request:
    thingset_msg_unref(request);
    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_BAD_REQUEST);
    return ts_msg_add_response_status_cbor(*response);

ts_process_set_cbor_not_found:
    thingset_msg_unref(request);
    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_NOT_FOUND);
    return ts_msg_add_response_status_cbor(*response);

ts_process_set_cbor_unsupported_format:
    thingset_msg_unref(request);
    ts_msg_status_set(*response, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_UNSUPPORTED_FORMAT);
    return ts_msg_add_response_status_cbor(*response);
}

int ts_ctx_process_set_json(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(oref),
              "CTX: %s on invalid object reference (%u:%u)", __func__,
              (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    /*
     * Check and validate request format
     * ---------------------------------
     */
    uint16_t child_count;
    int ret = ts_obj_child_count(oref, &child_count);
    TS_ASSERT(ret == 0, "CTX: %s calling ts_obj_child_count() fails (%d) - should never fail",
              __func__, ret);

    if (child_count == 0) {
        if (ts_msg_len(request) == 0) {
            if (ts_obj_type(oref) == TS_T_EXEC) {
                /* void function */
                goto ts_process_set_json_end;
            }
            /* For all other types we expect a value */
            TS_LOGD("CTX: %s on object %s with %d children but no value provided", __func__,
                    ts_obj_name(oref), (unsigned int)child_count);
            goto ts_process_set_json_bad_request;
        }
    }

    struct ts_msg_state request_state;
    ts_msg_state_save(request, &request_state);

    TS_MSG_ASSERT_SCRATCHTYPE(request, TS_MSG_SCRATCHPAD_JSON_DEC);

    bool set_by_map = false;
    bool set_by_array = false;
    uint16_t num_elements;
    if (child_count == 0) {
        if (ts_obj_type(oref) == TS_T_EXEC) {
            /* Special case - None parameters may be indicated by empty array */
            ret = ts_msg_pull_array_cbor(request, &num_elements);
            if ((ret != 0) || (num_elements != 0)) {
                TS_LOGD("CTX: %s on object %s with %d children but value provided", __func__,
                        ts_obj_name(oref), (unsigned int)child_count);
                goto ts_process_set_json_bad_request;
            }
            /* void function */
            goto ts_process_set_json_end;
        }
        else {
            TS_ASSERT(!(ts_obj_type(oref) == TS_T_GROUP) && !(ts_obj_type(oref) == TS_T_SUBSET),
                      "ThingSet: %s on %s%s %s with no children (0)", __func__,
                      ts_obj_type(oref) == TS_T_GROUP ? "group" : "",
                      ts_obj_type(oref) == TS_T_SUBSET ? "subset" : "",
                      ts_obj_name(oref));
            /* No children - set value of object itself */
            num_elements = 1;
        }
    }
    else if (child_count == 1) {
        /* One child - value may be given by array with one element or by map or by value itself */
        ret = ts_msg_pull_array_json(request, &num_elements);
        if (ret == 0) {
            if (num_elements != 1) {
                TS_LOGD("CTX: %s on object %s with %d children but array "
                        "with %u elements provided", __func__, ts_obj_name(oref),
                        (unsigned int)child_count, (unsigned int)num_elements);
                goto ts_process_set_json_bad_request;
            }
            set_by_array = true;
        }
        else {
            ret = ts_msg_pull_map_json(request, &num_elements);
            if (ret == 0) {
                if (num_elements != 1) {
                    TS_LOGD("CTX: %s on object %s with %d children but map "
                            "with %u elements provided", __func__, ts_obj_name(oref),
                            (unsigned int)child_count, (unsigned int)num_elements);
                    goto ts_process_set_json_bad_request;
                }
                set_by_map = true;
            }
            else {
                ret = 0;
                num_elements = 1;
            }
        }
    }
    else {
        /* child_count > 1 - value may be given by map only */
        ret = ts_msg_pull_map_json(request, &num_elements);
        if (ret != 0) {
            TS_LOGD("CTX: %s on object %s with %d children but no map provided", __func__,
                    ts_obj_name(oref), (unsigned int)child_count);
            goto ts_process_set_json_bad_request;
        }
        set_by_map = true;
    }

    /*
     * Loop through all elements to check if request is valid
     * ------------------------------------------------------
     */
    TS_ASSERT(ret == 0, "CTX: %s should handle all errors before here (%d)",  __func__, ret);
    thingset_oref_t child_oref;
    for (uint16_t count = 0;(count < num_elements) && (ret == 0); count++) {
        /* Get child oref */
        if (set_by_map) {
            /* Check whether object is in database */
            const char *name;
            uint16_t name_len;
            ret = ts_msg_pull_string_json(request, &name, &name_len);
            if (ret == 0) {
                /* We got the object name */
                ret = ts_obj_by_name(oref, name, name_len, &child_oref);
                if (ret != 0) {
                    goto ts_process_set_json_not_found;
                }
            }
            else {
                ts_obj_id_t obj_id;
                ret = ts_msg_pull_u16_json(request, &obj_id);
                if (ret != 0) {
                    TS_LOGD("ThingSet: %s set by unsupported key format - object: %s", __func__,
                            ts_obj_name(child_oref));
                    goto ts_process_set_json_unsupported_format;
                }
                ret = ts_obj_db_oref_by_id(oref.db_id, obj_id, &child_oref);
                if (ret != 0) {
                    goto ts_process_set_json_not_found;
                }
            }
        }
        else if (set_by_array) {
            if (count == 0) {
                if (child_count == 0) {
                    child_oref = oref;
                }
                else {
                    ret = ts_obj_child_first(oref, &child_oref);
                    TS_ASSERT(ret == 0,
                        "ThingSet: %s calling ts_obj_child_next() fails (%d) - should never fail",
                        __func__, ret);
                }
            }
            else {
                ret = ts_obj_child_next(&child_oref);
                TS_ASSERT(ret == 0,
                          "ThingSet: %s calling ts_obj_child_next() fails (%d) - should never fail",
                            __func__, ret);
            }
        }
        else if (count == 0) {
            /* no map, no array -> simple object */
            if (child_count == 0) {
                child_oref = oref;
            }
            else {
                ret = ts_obj_child_first(oref, &child_oref);
                TS_ASSERT(ret == 0,
                    "ThingSet: %s calling ts_obj_child_first() fails (%d) - should never fail",
                    __func__, ret);
            }
        }
        else {
            /* count > 0 */
            TS_ASSERT(false, "ThingSet: %s count > 0 - should never happen for object set",
                      __func__);
            goto ts_process_set_json_bad_request;
        }

        /* Check access */
        if (!ts_obj_access_write(child_oref, ts_msg_auth(request))) {
            if (ts_obj_access_write(child_oref, TS_WRITE_MASK)) {
                TS_LOGD("%s: set not authorized: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)ts_msg_auth(request), ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                thingset_msg_unref(request);
                return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_UNAUTHORIZED,
                                                    response);
            }
            else {
                TS_LOGD("%s: set forbidden: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)ts_msg_auth(request), ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                thingset_msg_unref(request);
                return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_FORBIDDEN,
                                                    response);
            }
        }

        /* Check object type and available buffer length vs. value provided */
        ret = ts_msg_pull_object_json(request, child_oref, false);
        if (ret != 0) {
            TS_LOGD("ThingSet: %s set by unsupported value format - object: %s", __func__,
                    ts_obj_name(child_oref));
            goto ts_process_set_json_unsupported_format;
        }
    }
    if (set_by_map) {
        ret = ts_msg_pull_map_end_json(request);
    }
    else if (set_by_array) {
        ret = ts_msg_pull_array_end_json(request);
    }
    if (ret != 0) {
        goto ts_process_set_json_bad_request;
    }

    /*
     * Actually write data
     * -------------------
     */
    ts_msg_state_restore(request, &request_state);
    ret = ts_msg_json_dec_setup(request);
    TS_ASSERT(ret == 0, "Scratchpad init fail after message state restore");

    if (set_by_map) {
        (void)ts_msg_pull_map_json(request, &num_elements);
    }
    else if (set_by_array) {
        (void)ts_msg_pull_array_json(request, &num_elements);
    }
    for (uint16_t count = 0; count < num_elements; count++) {
        /* Get child oref */
        if (set_by_map) {
            /* Check whether object is in database */
            const char *name;
            uint16_t name_len;
            ret = ts_msg_pull_string_json(request, &name, &name_len);
            if (ret == 0) {
                /* We got the object name */
                (void)ts_obj_by_name(oref, name, name_len, &child_oref);
            }
            else {
                ts_obj_id_t obj_id;
                (void)ts_msg_pull_u16_json(request, &obj_id);
                (void)ts_obj_db_oref_by_id(oref.db_id, obj_id, &child_oref);
            }
        }
        else if (set_by_array) {
            if (count == 0) {
                if (child_count == 0) {
                    child_oref = oref;
                }
                else {
                    (void)ts_obj_child_first(oref, &child_oref);
                }
            }
            else {
                (void)ts_obj_child_next(&child_oref);
            }
        }
        else {
            if (child_count == 0) {
                child_oref = oref;
            }
            else {
                (void)ts_obj_child_first(oref, &child_oref);
            }
        }

        (void)ts_msg_pull_object_json(request, child_oref, true);
    }

ts_process_set_json_end:
    thingset_msg_unref(request);
    return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_CHANGED, response);

ts_process_set_json_bad_request:
    thingset_msg_unref(request);
    return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_BAD_REQUEST, response);

ts_process_set_json_not_found:
    thingset_msg_unref(request);
    return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_NOT_FOUND, response);

ts_process_set_json_unsupported_format:
    thingset_msg_unref(request);
    return ts_ctx_process_request_error(TS_MSG_PROTO_TXT, TS_MSG_CODE_UNSUPPORTED_FORMAT, response);
}
