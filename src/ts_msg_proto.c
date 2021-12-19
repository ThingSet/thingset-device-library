/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet Protocol
 * -----------------
 */

#include "thingset_env.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ts_msg.h"

int ts_msg_add_status(struct thingset_msg *msg, ts_msg_status_code_t code)
{
    char hex[2];

    hex[0] = code >> 4;
    hex[1] = code & 0x0F;
    for (int i = 0; i < 2; i++) {
        if (hex[i] <= 9) {
            hex[i] += '0';
        }
        else {
            hex[i] += 'A' - 10;
        }
    }

    return ts_msg_add_mem(msg, (const uint8_t *)&hex[0], 2);
}

static int ts_msg_add_object_array_cbor(struct thingset_msg *msg,
                                        const struct ts_array_info *array_info, int16_t detail)
{
    if (!array_info) {
        return 0;
    }

    // Add the length field to the beginning of the CBOR buffer and update the CBOR buffer index
    int ret = ts_msg_add_array_cbor(msg, array_info->num_elements);
    if (ret != 0) {
        return ret;
    }

    for (int i = 0; (i < array_info->num_elements) && (ret == 0); i++) {
        switch (array_info->type) {
        case TS_T_UINT64:
            ret = ts_msg_add_u64_cbor(msg, ((uint64_t *)array_info->ptr)[i]);
            break;
        case TS_T_INT64:
            ret = ts_msg_add_i64_cbor(msg, ((int64_t *)array_info->ptr)[i]);
            break;
        case TS_T_UINT32:
            ret = ts_msg_add_u32_cbor(msg, ((uint32_t *)array_info->ptr)[i]);
            break;
        case TS_T_INT32:
            ret = ts_msg_add_i32_cbor(msg, ((int32_t *)array_info->ptr)[i]);
            break;
        case TS_T_UINT16:
            ret = ts_msg_add_u16_cbor(msg, ((uint16_t *)array_info->ptr)[i]);
            break;
        case TS_T_INT16:
            ret = ts_msg_add_i16_cbor(msg, ((int16_t *)array_info->ptr)[i]);
            break;
        case TS_T_FLOAT32:
            ret = ts_msg_add_f32_cbor(msg, ((float *)array_info->ptr)[i], detail);
            break;
        case TS_T_BOOL:
            ret = ts_msg_add_bool_cbor(msg, ((bool *)array_info->ptr)[i]);
            break;
        default:
            /* should not happen */
            TS_LOGD("CBOR encoder: %s - unexpected type (%u)", __func__,
                    (unsigned int)array_info->type);
            break;
        }
    }
    int ret_close = ts_msg_add_array_end_cbor(msg);
    if (ret_close != 0) {
        return ret_close;
    }
    return ret;
}

int ts_msg_add_object_cbor(struct thingset_msg *msg, thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "MSG: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    int ret = 0;
#if 0
    int objects_found;
#endif

    switch (ts_obj_type(oref)) {
    case TS_T_UINT64:
        ret = ts_msg_add_u64_cbor(msg, *ts_obj_u64_data(oref));
        break;
    case TS_T_INT64:
        ret = ts_msg_add_i64_cbor(msg, *ts_obj_i64_data(oref));
        break;
    case TS_T_UINT32:
        ret = ts_msg_add_u32_cbor(msg, *ts_obj_u32_data(oref));
        break;
    case TS_T_INT32:
        ret = ts_msg_add_i32_cbor(msg, *ts_obj_i32_data(oref));
        break;
    case TS_T_UINT16:
        ret = ts_msg_add_u16_cbor(msg, *ts_obj_u16_data(oref));
        break;
    case TS_T_INT16:
        ret = ts_msg_add_i16_cbor(msg, *ts_obj_i16_data(oref));
        break;
    case TS_T_FLOAT32:
        ret = ts_msg_add_f32_cbor(msg, *ts_obj_f32_data(oref), ts_obj_detail(oref));
        break;
    case TS_T_DECFRAC:
        ret = ts_msg_add_decfrac_cbor(msg, *ts_obj_decfrac_mantissa_data(oref),
                                      ts_obj_decfrac_exponent_data(oref));
        break;
    case TS_T_BOOL:
        ret = ts_msg_add_bool_cbor(msg, *ts_obj_bool_data(oref));
        break;
#if 0
    case TS_T_EXEC:
        ret = ts_msg_add_string_json(msg, "[");
        objects_found = 0;
        for (unsigned int i = 0; (i < db->num) && (ret == 0); i++) {
            if (db->objects[i].parent == object->id) {
                objects_found++;
                ret = ts_msg_add_string_json(msg, db->objects[i].name);
                if (ret != 0) {
                    break;
                }
                ret = ts_msg_add_string_json(msg, ",");
            }
        }
        if (ret != 0) {
            break;
        }
        /* Remove trailing [ ... ',' or unnecessary '[' */
        ts_buf_remove(msg, 1);
        if (objects_found) {
            ret = ts_msg_add_u8(msg, ']');
        }
        else {
            ret = ts_msg_add_mem(msg, (const uint8_t *)&"null", 4);
        }
        break;
#endif
    case TS_T_STRING:
        ret = ts_msg_add_string_cbor(msg, ts_obj_string_data(oref));
        break;
    case TS_T_BYTES:
        ret = ts_msg_add_mem_cbor(msg, ts_obj_mem_data(oref), *ts_obj_mem_len(oref));
        break;
#if 0
    case TS_T_SUBSET:
        ret = ts_msg_add_u8(msg, '[');
        objects_found = 0;
        for (unsigned int i = 0; (i < db->num) && (ret == 0); i++) {
            if (db->flags[i].subsets & (uint16_t)object->detail) {
                objects_found++;
                ret = ts_msg_add_string_json(msg, db->objects[i].name);
                if (ret != 0) {
                    break;
                }
                ret = ts_msg_add_u8(msg, ',');
            }
        }
        if (ret != 0) {
            break;
        }
        if (objects_found) {
            /* Remove trailing [ ... ',' */
            ts_buf_remove(msg, 1);
        }
        ret = ts_msg_add_u8(msg, ']');
        break;
#endif
    case TS_T_ARRAY:
        ret = ts_msg_add_object_array_cbor(msg, ts_obj_array_data(oref), ts_obj_detail(oref));
        break;
    default:
        TS_LOGD("CBOR encoder: %s - unexpected type (%u)", __func__,
                (unsigned int)ts_obj_type(oref));
        break;
    }
    return ret;
}

int ts_msg_add_object_json(struct thingset_msg *msg, thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "MSG: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    int ret = 0;

    switch (ts_obj_type(oref)) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        ret = ts_msg_add_u64_json(msg, *ts_obj_u64_data(oref));
        break;
    case TS_T_INT64:
        ret = ts_msg_add_i64_json(msg, *ts_obj_i64_data(oref));
        break;
#endif
    case TS_T_UINT32:
        ret = ts_msg_add_u32_json(msg, *ts_obj_u32_data(oref));
        break;
    case TS_T_INT32:
        ret = ts_msg_add_i32_json(msg, *ts_obj_i32_data(oref));
        break;
    case TS_T_UINT16:
        ret = ts_msg_add_u16_json(msg, *ts_obj_u16_data(oref));
        break;
    case TS_T_INT16:
        ret = ts_msg_add_i16_json(msg, *ts_obj_i16_data(oref));
        break;
    case TS_T_FLOAT32:
        ret = ts_msg_add_f32_json(msg, *ts_obj_f32_data(oref), ts_obj_detail(oref));
        break;
#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        ret = ts_msg_add_decfrac_json(msg, *ts_obj_decfrac_mantissa_data(oref),
                                      ts_obj_decfrac_exponent_data(oref));
        break;
#endif
    case TS_T_BOOL:
        ret = ts_msg_add_bool_json(msg, *ts_obj_bool_data(oref));
        break;
    case TS_T_EXEC:
        {
            ts_obj_id_t parent_id =  ts_obj_id(oref);
            ret = ts_msg_add_array_json(msg, 0);
            if (ret != 0) {
                break;
            }
            TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
                if (ts_obj_parent_id(child_oref) == parent_id) {
                    ret = ts_msg_add_string_json(msg, ts_obj_name(child_oref));
                    if (ret != 0) {
                        break;
                    }
                }
            }
            int ret_end = ts_msg_add_array_end_json(msg);
            if (ret == 0) {
                ret = ret_end;
            }
            if (ret != 0) {
                break;
            }
            if (ts_msg_tail(msg)[-2] == '[') {
                /* This is an empty array - remove and add 'null' */
                ts_msg_remove(msg, 2);
                ret = ts_msg_add_mem(msg, (const uint8_t *)&"null", 4);
            }
        }
        break;
    case TS_T_STRING:
        ret = ts_msg_add_string_json(msg, ts_obj_string_data(oref));
        break;
    case TS_T_GROUP:
        {
            ts_obj_id_t parent_id =  ts_obj_id(oref);
            ret = ts_msg_add_array_json(msg, 0);
            if (ret != 0) {
                break;
            }
            TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
                if (ts_obj_parent_id(child_oref) == parent_id) {
                    ret = ts_msg_add_string_json(msg, ts_obj_name(child_oref));
                    if (ret != 0) {
                        break;
                    }
                }
            }
            int ret_end = ts_msg_add_array_end_json(msg);
            if (ret == 0) {
                ret = ret_end;
            }
        }
        break;
    case TS_T_SUBSET:
        {
            uint16_t subsets = (uint16_t)ts_obj_detail(oref);
            ret = ts_msg_add_array_json(msg, 0);
            if (ret != 0) {
                break;
            }
            TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
                if (ts_obj_subsets(child_oref) & subsets) {
                    ret = ts_msg_add_string_json(msg, ts_obj_name(child_oref));
                    if (ret != 0) {
                        break;
                    }
                }
            }
            int ret_end = ts_msg_add_array_end_json(msg);
            if (ret == 0) {
                ret = ret_end;
            }
        }
        break;
    case TS_T_ARRAY:
        {
            struct ts_array_info *array_info = ts_obj_array_data(oref);
            if (!array_info) {
                break;
            }
            ret = ts_msg_add_array_json(msg, array_info->num_elements);
            for (int i = 0; (i < array_info->num_elements) && (ret == 0); i++) {
                switch (array_info->type) {
                case TS_T_UINT64:
                    ret = ts_msg_add_u64_json(msg, ((uint64_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT64:
                    ret = ts_msg_add_i64_json(msg, ((int64_t *)array_info->ptr)[i]);
                    break;
                case TS_T_UINT32:
                    ret = ts_msg_add_u32_json(msg, ((uint32_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT32:
                    ret = ts_msg_add_i32_json(msg, ((int32_t *)array_info->ptr)[i]);
                    break;
                case TS_T_UINT16:
                    ret = ts_msg_add_u16_json(msg, ((uint16_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT16:
                    ret = ts_msg_add_i16_json(msg, ((int16_t *)array_info->ptr)[i]);
                    break;
                case TS_T_FLOAT32:
                    ret = ts_msg_add_f32_json(msg, ((float *)array_info->ptr)[i],
                                              ts_obj_detail(oref));
                    break;
                default:
                    /* should not happen */
                    TS_ASSERT(false, "Unexpected ThingSet array element type (%u)",
                              (unsigned int)array_info->type);
                    break;
                }
            }
            int ret_end = ts_msg_add_array_end_json(msg);
            if (ret == 0) {
                ret = ret_end;
            }
        }
        break;
    default:
        TS_LOGD("JSON encoder: %s got unexpected object type %u - object: %s", __func__,
                (unsigned int)ts_obj_type(oref), ts_obj_name(oref));
        break;
    }
    return ret;
}

int ts_msg_add_request_get_cbor(struct thingset_msg *msg, ts_obj_id_t object_id, const char *path)
{
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST, 0);

    int ret = ts_msg_add_u8(msg, TS_GET);
    if (ret != 0) {
        return ret;
    }

    ret = ts_msg_cbor_enc_setup(msg);
    if (ret != 0) {
        return ret;
    }

    if (path != NULL) {
        ret = ts_msg_add_string_cbor(msg, path);
    }
    else {
        ret = ts_msg_add_u16_cbor(msg, object_id);
    }
    return ret;
}

int ts_msg_add_request_get_json(struct thingset_msg *msg, const char *path)
{
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);

    int ret = ts_msg_add_u8(msg, '?');
    if (ret != 0) {
        return ret;
    }

    return ts_msg_add_mem(msg, path, strlen(path));
}

int ts_msg_add_response_get_cbor(struct thingset_msg *msg, thingset_oref_t oref,
                                 struct thingset_msg* request)
{
    TS_ASSERT(ts_obj_db_oref_is_valid(oref),
              "MSG CBOR: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    int ret;

    /* initialize response with success message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CONTENT);
    ret = ts_msg_add_response_status_cbor(msg);
    if (ret != 0) {
        return ret;
    }

    ts_obj_id_t parent_id;
    if (ts_obj_db_oref_is_object(oref)) {
        if (ts_obj_type(oref) != TS_T_GROUP && ts_obj_type(oref) != TS_T_EXEC) {
            return ts_msg_add_object_cbor(msg, oref);
        }
        parent_id = ts_obj_id(oref);
    }
    else {
        parent_id = TS_ID_ROOT;
    }

    /* find out number of elements to be serialized */
    int num_elements = 0;
    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
        if (ts_obj_access_read(child_oref, TS_READ_MASK) &&
            (ts_obj_parent_id(child_oref) == parent_id)) {
            num_elements++;
        }
    }

    ts_msg_status_code_t request_code = ts_msg_status_code(request);
    bool include_ids = (request_code == TS_MSG_CODE_REQUEST_GET_IDS) ||
                       (request_code == TS_MSG_CODE_REQUEST_GET_IDS_VALUES);
    bool include_names = (request_code == TS_MSG_CODE_REQUEST_GET_NAMES) ||
                         (request_code == TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    bool include_values = (request_code == TS_MSG_CODE_REQUEST_GET_VALUES) ||
                          (request_code == TS_MSG_CODE_REQUEST_GET_IDS_VALUES) ||
                          (request_code == TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);

    if (ts_obj_db_oref_is_object(oref) && ts_obj_type(oref) == TS_T_EXEC && include_values) {
        // bad request, as we can't read exec object's values
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_BAD_REQUEST);
        return ts_msg_add_response_status_cbor(msg);
    }

    if ((include_ids || include_names) && include_values) {
        ret = ts_msg_add_map_cbor(msg, num_elements);
    }
    else {
        ret = ts_msg_add_array_cbor(msg, num_elements);
    }
    if (ret != 0) {
        goto ts_msg_add_response_get_cbor_error;
    }

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
        if (ts_obj_access_read(child_oref, TS_READ_MASK) &&
            (ts_obj_parent_id(child_oref) == parent_id)) {
            if (include_ids) {
                ret = ts_msg_add_u16_cbor(msg, ts_obj_id(child_oref));
                if (ret != 0) {
                    goto ts_msg_add_response_get_cbor_error;
                }
            }
            if (include_names) {
                ret = ts_msg_add_string_cbor(msg, ts_obj_name(child_oref));
                if (ret != 0) {
                    goto ts_msg_add_response_get_cbor_error;
                }
            }
            if (include_values) {
                if (ts_obj_type(child_oref) == TS_T_GROUP) {
                    /* bad request, as we can't read internal path object's values */
                    thingset_msg_reset(msg);
                    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                                      TS_MSG_CODE_BAD_REQUEST);
                    return ts_msg_add_response_status_cbor(msg);
                }
                ret = ts_msg_add_object_cbor(msg, child_oref);
                if (ret != 0) {
                    goto ts_msg_add_response_get_cbor_error;
                }
            }
        }
    }

    if ((include_ids || include_names) && include_values) {
        ret = ts_msg_add_map_end_cbor(msg);
    }
    else {
        ret = ts_msg_add_array_end_cbor(msg);
    }

    if (ret != 0) {
ts_msg_add_response_get_cbor_error:
#if TS_CONFIG_LOG
        char log_buf[200];
        ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
        TS_LOGD("MSG CBOR: %s on get request 0x%" PRIXPTR " creates too large response 0x%" PRIXPTR
                " message: >%d<, >%.*s<",
                __func__, (uintptr_t)request, (uintptr_t)msg, ts_msg_len(msg), ret, &log_buf[0]);
#endif
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_RESPONSE_TOO_LARGE);
        return ts_msg_add_response_status_cbor(msg);
    }

    return ret;
}

int ts_msg_add_response_get_json(struct thingset_msg *msg, thingset_oref_t oref,
                                 struct thingset_msg* request)
{
    TS_ASSERT(ts_obj_db_oref_is_valid(oref),
              "MSG CBOR: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    int ret;

    /* initialize response with success message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CONTENT);
    ret = ts_msg_add_response_status_json(msg);
    if (ret != 0) {
        return ret;
    }

    ts_obj_id_t parent_id;
    if (ts_obj_db_oref_is_object(oref)) {
        if (ts_obj_type(oref) != TS_T_GROUP && ts_obj_type(oref) != TS_T_EXEC) {
            // get value of data object
            ret = ts_msg_add_u8(msg, ' ');
            if (ret != 0) {
                return ret;
            }
            return ts_msg_add_object_json(msg, oref);
        }
        parent_id = ts_obj_id(oref);
    }
    else {
        parent_id = TS_ID_ROOT;
    }

    ts_msg_status_code_t request_code = ts_msg_status_code(request);
    bool include_names = (request_code == TS_MSG_CODE_REQUEST_GET_NAMES) ||
                         (request_code == TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    bool include_values = (request_code == TS_MSG_CODE_REQUEST_GET_VALUES) ||
                          (request_code == TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    TS_ASSERT(include_names || include_values, "At least one of names or values must be included");

    if (ts_obj_db_oref_is_object(oref) && (ts_obj_type(oref) == TS_T_EXEC) && include_values) {
        // bad request, as we can't read exec object's values
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_BAD_REQUEST);
        return ts_msg_add_response_status_json(msg);
    }

    ret = ts_msg_add_u8(msg, ' ');
    if (ret != 0) {
        return ret;
    }

    if (include_names && include_values) {
        ret = ts_msg_add_map_json(msg, 0);
    }
    else {
        ret = ts_msg_add_array_json(msg, 0);
    }
    if (ret != 0) {
        return ret;
    }

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
        if (ts_obj_access_read(child_oref, TS_READ_MASK) &&
            (ts_obj_parent_id(child_oref) == parent_id)) {
            if (include_names) {
                ret = ts_msg_add_string_json(msg, ts_obj_name(child_oref));
                if (ret != 0) {
                    break;
                }
            }
            if (include_values) {
                if (ts_obj_type(child_oref) == TS_T_GROUP) {
                    /* bad request, as we can't read internal path object's values */
                    TS_LOGD("MSG JSON: %s on get request 0x%" PRIXPTR
                            " tries to get value of group type object '%s'", __func__,
                            (uintptr_t)request, ts_obj_name(child_oref));
                    thingset_msg_reset(msg);
                    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                                    TS_MSG_CODE_BAD_REQUEST);
                    return ts_msg_add_response_status_json(msg);
                    }
                ret = ts_msg_add_object_json(msg, child_oref);
            }
        }
    }

    if (ret == 0) {
        if (include_names && include_values) {
            ret = ts_msg_add_map_end_json(msg);
        }
        else {
            ret = ts_msg_add_array_end_json(msg);
        }
    }
    if (ret != 0) {
#if TS_CONFIG_LOG
        char log_buf[200];
        ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
        TS_LOGD("MSG JSON: %s on get request 0x%" PRIXPTR " creates too large response 0x%" PRIXPTR
                " message: >%d<, >%.*s<",
                __func__, (uintptr_t)request, (uintptr_t)msg, ts_msg_len(msg), ret, &log_buf[0]);
#endif
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_RESPONSE_TOO_LARGE);
        return ts_msg_add_response_status_json(msg);
    }

    return 0;
}

int ts_msg_add_request_fetch_cbor(struct thingset_msg *msg, ts_obj_id_t object_id, const char *path,
                                  uint16_t object_count, ts_obj_id_t *object_ids,
                                  const char **object_names)
{
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST, 0);

    int ret = ts_msg_add_u8(msg, TS_FETCH);
    if (ret != 0) {
        return ret;
    }

    ret = ts_msg_cbor_enc_setup(msg);
    if (ret != 0) {
        return ret;
    }

    if (path != NULL) {
        ret = ts_msg_add_string_cbor(msg, path);
    }
    else {
        ret = ts_msg_add_u16_cbor(msg, object_id);
    }
    if (ret != 0) {
        return ret;
    }

    if (object_count == 0) {
        ret = ts_msg_add_undefined_cbor(msg);
    }
    else {
        if (object_count > 1) {
            ret = ts_msg_add_array_cbor(msg, object_count);
        }
        if (ret != 0) {
            return ret;
        }
        if (object_names == NULL) {
            for (uint16_t i = 0; i < object_count; i++) {
                ret = ts_msg_add_u16_cbor(msg, object_ids[i]);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        else {
            for (uint16_t i = 0; i < object_count; i++) {
                ret = ts_msg_add_string_cbor(msg, object_names[i]);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        if (object_count > 1) {
            ret = ts_msg_add_array_end_cbor(msg);
        }
    }

    return ret;
}

int ts_msg_add_request_fetch_json(struct thingset_msg* msg, const char *path,
                                  uint16_t object_count, const char **object_names)
{
    return -ENOTSUP;
}

int ts_msg_add_response_fetch_cbor(struct thingset_msg *msg, thingset_oref_t oref,
                                   struct thingset_msg *request)
{
    if (ts_msg_status_code(request) == TS_MSG_CODE_REQUEST_FETCH_NAMES) {
        /* Add all names behind endpoint */
        ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_NAMES);
        return ts_msg_add_response_get_cbor(msg, oref, request);
    }
    if (ts_msg_status_code(request) == TS_MSG_CODE_REQUEST_FETCH_IDS) {
        /* Add all ids behind endpoint */
        ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_IDS);
        return ts_msg_add_response_get_cbor(msg, oref, request);
    }
    if ((ts_msg_status_code(request) != TS_MSG_CODE_REQUEST_FETCH_VALUES) &&
        (ts_msg_status_code(request) != TS_MSG_CODE_REQUEST_FETCH_SINGLE)) {
        /* Unexpected fetch request status - this should never happen */
        TS_ASSERT(true, "Unexpected fetch request status code (%02x)",
                  (unsigned int)ts_msg_status_code(request));
        goto ts_msg_add_response_fetch_cbor_bad_request;
    }

    /* Add data defined by names/ids in request array */
    TS_ASSERT(ts_obj_db_oref_is_valid(oref),
              "MSG CBOR: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    /* initialize response with success message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CONTENT);
    int ret = ts_msg_add_response_status_cbor(msg);
    if (ret != 0) {
        return ret;
    }

    uint16_t len;
    if (ts_msg_status_code(request) == TS_MSG_CODE_REQUEST_FETCH_SINGLE) {
        len = 1;
    }
    else {
        ret = ts_msg_pull_array_cbor(request, &len);
        if ((ret != 0) || (len == 0)) {
            goto ts_msg_add_response_fetch_cbor_bad_request;
        }
    }

    if (len > 1) {
        ret = ts_msg_add_array_cbor(msg, len);
        if (ret != 0) {
            return ret;
        }
    }

    uint16_t request_auth = ts_msg_auth(request);
    for (uint16_t i = 0; (i < len) && (ret == 0); i++) {
        thingset_oref_t child_oref;
        CborType t;
        ret = ts_msg_pull_type_cbor(request, &t);
        if (ret != 0) {
            goto ts_msg_add_response_fetch_cbor_bad_request;
        }
        if (t == CborIntegerType) {
            ts_obj_id_t obj_id;
            ret = ts_msg_pull_u16_cbor(request, &obj_id);
            if (ret != 0) {
                goto ts_msg_add_response_fetch_cbor_bad_request;
            }
            ret = ts_obj_db_oref_by_id(oref.db_id, obj_id, &child_oref);
            if (ret != 0) {
                goto ts_msg_add_response_fetch_cbor_not_found;
            }
        }
        else if (t == CborTextStringType) {
            const char *path;
            uint16_t path_len;
            ret = ts_msg_pull_string_cbor(request, &path, &path_len);
            if (ret != 0) {
                goto ts_msg_add_response_fetch_cbor_bad_request;
            }
            ret = ts_obj_by_name(oref, path, path_len, &child_oref);
            if (ret != 0) {
                goto ts_msg_add_response_fetch_cbor_not_found;
            }
        }
        else {
            goto ts_msg_add_response_fetch_cbor_bad_request;
        }

        if (!ts_obj_access_read(child_oref, request_auth)) {
            thingset_msg_reset(msg);
            if (ts_obj_access_read(child_oref, TS_READ_MASK)) {
                TS_LOGD("MSG: %s on fetch request 0x%" PRIXPTR
                        " not authorized: >0x%04x< - object: %s",
                        __func__, (uintptr_t)request, (unsigned int)request_auth,
                        ts_obj_name(child_oref));
                ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                                  TS_MSG_CODE_UNAUTHORIZED);
                ret = ts_msg_add_response_status_cbor(msg);
            }
            else {
                ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                                  TS_MSG_CODE_FORBIDDEN);
                ret = ts_msg_add_response_status_cbor(msg);
            }
            return ret;
        }
        ret = ts_msg_add_object_cbor(msg, child_oref);
    }
    if (ret == 0) {
        if (ts_msg_status_code(request) != TS_MSG_CODE_REQUEST_FETCH_SINGLE) {
            ret = ts_msg_pull_array_end_cbor(request);
            if (ret != 0) {
                goto ts_msg_add_response_fetch_cbor_bad_request;
            }
        }

        if (len > 1) {
            ret = ts_msg_add_array_end_cbor(msg);
        }
    }

    if (ret != 0) {
#if TS_CONFIG_LOG
        char log_buf[200];
        ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
        TS_LOGD("MSG CBOR: %s on fetch request 0x%" PRIXPTR " creates too large response 0x%" PRIXPTR
                " message: >%d<, >%.*s<",
                __func__, (uintptr_t)request, (uintptr_t)msg, ts_msg_len(msg), ret, &log_buf[0]);
#endif
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_RESPONSE_TOO_LARGE);
        return ts_msg_add_response_status_cbor(msg);
    }
    return 0;

ts_msg_add_response_fetch_cbor_bad_request:
    thingset_msg_reset(msg);
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_BAD_REQUEST);
    return ts_msg_add_response_status_cbor(msg);

ts_msg_add_response_fetch_cbor_not_found:
    thingset_msg_reset(msg);
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_NOT_FOUND);
    return ts_msg_add_response_status_cbor(msg);
}

int ts_msg_add_response_fetch_json(struct thingset_msg *msg, thingset_oref_t oref,
                                   struct thingset_msg *request)
{
    TS_ASSERT(ts_obj_db_oref_is_valid(oref),
              "MSG JSON: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    /* initialize response with success message */
    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CONTENT);
    int ret = ts_msg_add_response_status_json(msg);
    if (ret != 0) {
        return ret;
    }

    ret =  ts_msg_json_dec_setup(request);
    if (ret != 0) {
        thingset_msg_reset(msg);
        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          TS_MSG_CODE_INTERNAL_SERVER_ERR);
        return ts_msg_add_response_status_json(msg);
    }
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(request_scratchpad, request);
    struct ts_jsmn_context *jsmn = &request_scratchpad->jsmn;

    ts_msg_add_u8(msg, ' ');

    uint16_t type, size, len;
    const char *start;
    uint8_t token_idx = 0; // current token
    bool bracket = false;
    thingset_oref_t child_oref;
    uint16_t request_auth = ts_msg_auth(request);
    while (ts_jsmn_token_by_index(jsmn, token_idx, &type, &size, &start, &len) == 0) {
        if ((token_idx == 0) &&  (type == TS_JSMN_ARRAY)) {
            bracket = true;
            ts_msg_add_u8(msg, '[');
            token_idx++;
            continue;
        }

        if (type != TS_JSMN_STRING) {
            thingset_msg_reset(msg);
            ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_json(msg);
        }

        ret = ts_obj_by_name(oref, start, len, &child_oref);
        if (ret != 0) {
            thingset_msg_reset(msg);
            ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_NOT_FOUND);
            return ts_msg_add_response_status_json(msg);
        }
        if (ts_obj_type(child_oref) == TS_T_GROUP) {
            /* bad request, as we can't read internal path object's values */
            thingset_msg_reset(msg);
            ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_BAD_REQUEST);
            return ts_msg_add_response_status_json(msg);
        }
        if (!ts_obj_access_read(child_oref, request_auth)) {
            thingset_msg_reset(msg);
            if (ts_obj_access_read(child_oref, TS_READ_MASK)) {
                TS_LOGD("%s: fetch not authorized: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)request_auth, ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                                TS_MSG_CODE_UNAUTHORIZED);
                ret = ts_msg_add_response_status_json(msg);
            }
            else {
                TS_LOGD("%s: fetch forbidden: >0x%04x< - object: %s >0x%04x<", __func__,
                        (unsigned int)request_auth, ts_obj_name(child_oref),
                        (unsigned int)ts_obj_access(child_oref));
                ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                                TS_MSG_CODE_FORBIDDEN);
                ret = ts_msg_add_response_status_json(msg);
            }
            return ret;
        }

        ret = ts_msg_add_object_json(msg, child_oref);
        if (ret == 0) {
            ret = ts_msg_add_u8(msg, ',');
        }

        if (ret != 0) {
#if TS_CONFIG_LOG
            char log_buf[200];
            ret = ts_msg_log(msg, &log_buf[0], sizeof(log_buf));
            TS_LOGD("MSG JSON: %s on fetch request 0x%" PRIXPTR " creates too large response 0x%"
                    PRIXPTR " message: >%d<, >%.*s<",
                    __func__, (uintptr_t)request, (uintptr_t)msg, ts_msg_len(msg), ret, &log_buf[0]);
#endif
            thingset_msg_reset(msg);
            ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                            TS_MSG_CODE_RESPONSE_TOO_LARGE);
            return ts_msg_add_response_status_json(msg);
        }

        token_idx++;
    }

    /* Remove trailing comma */
    ts_msg_remove(msg, 1);
    if (bracket) {
        /* buffer will be long enough as we dropped last 1 character */
        (void)ts_msg_add_u8(msg, (uint8_t)']');
    }

    return 0;
}

int ts_msg_add_response_status_cbor(struct thingset_msg *msg)
{
    TS_ASSERT(ts_msg_status_proto(msg) == TS_MSG_PROTO_BIN,
              "MSG: %s - invalid protocol %u of message 0x%" PRIXPTR, __func__,
              (unsigned int)ts_msg_status_proto(msg), (uintptr_t)msg);

    ts_msg_status_code_t code = ts_msg_status_code(msg);
    TS_ASSERT(code >= TS_STATUS_CREATED,
              "MSG: %s - invalid code 0x%02x to add to message 0x%" PRIXPTR, __func__,
              (unsigned int)code, (uintptr_t)msg);

    int ret = ts_msg_add_u8(msg, code);
    if (ret != 0) {
        return ret;
    }

    if ((code == TS_MSG_CODE_CONTENT) || (code == TS_MSG_CODE_EXPORT)) {
        /* Extra CBOR data will follow - prepare for CBOR encoding */
        return ts_msg_cbor_enc_setup(msg);
    }

    return 0;
}

int ts_msg_add_response_status_json(struct thingset_msg *msg)
{
    TS_ASSERT(ts_msg_status_proto(msg) == TS_MSG_PROTO_TXT,
              "MSG: %s - invalid protocol %u of message 0x%" PRIXPTR, __func__,
              (unsigned int)ts_msg_status_proto(msg), (uintptr_t)msg);
    ts_msg_status_code_t code = ts_msg_status_code(msg);
    TS_ASSERT(code >= TS_STATUS_CREATED,
              "MSG: %s - invalid code 0x%02x to add to message 0x%" PRIXPTR, __func__,
              (unsigned int)code, (uintptr_t)msg);

    if (ts_msg_tailroom(msg) < 4) {
        /* Minimum size of status message */
        return -ENOMEM;
    }

    (void)ts_msg_add_u8(msg, ':');          /* 1 byte */
    (void)ts_msg_add_status(msg, code);     /* 2 bytes */

    if (TS_CONFIG_VERBOSE_STATUS_MESSAGES) {
        const char *status_msg;
        switch (code) {
            // success
            case TS_MSG_CODE_CREATED:
                status_msg = " Created";
                break;
            case TS_MSG_CODE_DELETED:
                status_msg = " Deleted";
                break;
            case TS_MSG_CODE_VALID:
                status_msg = " Valid";
                break;
            case TS_MSG_CODE_CHANGED:
                status_msg = " Changed";
                break;
            case TS_MSG_CODE_CONTENT:
                status_msg = " Content";
                break;
            case TS_MSG_CODE_EXPORT:
                status_msg = " Export";
                break;
            // client errors
            case TS_MSG_CODE_BAD_REQUEST:
                status_msg = " Bad Request";
                break;
            case TS_MSG_CODE_UNAUTHORIZED:
                status_msg = " Unauthorized";
                break;
            case TS_MSG_CODE_FORBIDDEN:
                status_msg = " Forbidden";
                break;
            case TS_MSG_CODE_NOT_FOUND:
                status_msg = " Not Found";
                break;
            case TS_MSG_CODE_METHOD_NOT_ALLOWED:
                status_msg = " Method Not Allowed";
                break;
            case TS_MSG_CODE_REQUEST_INCOMPLETE:
                status_msg = " Request Entity Incomplete";
                break;
            case TS_MSG_CODE_CONFLICT:
                status_msg = " Conflict";
                break;
            case TS_MSG_CODE_REQUEST_TOO_LARGE:
                status_msg = " Request Entity Too Large";
                break;
            case TS_MSG_CODE_UNSUPPORTED_FORMAT:
                status_msg = " Unsupported Content-Format";
                break;
            // server errors
            case TS_MSG_CODE_INTERNAL_SERVER_ERR:
                status_msg = " Internal Server Error";
                break;
            case TS_MSG_CODE_NOT_IMPLEMENTED:
                status_msg = " Not Implemented";
                break;
            // ThingSet specific errors
            case TS_MSG_CODE_RESPONSE_TOO_LARGE:
                status_msg = " Response too large";
                break;
            default:
                status_msg = " Error";
                break;
        };
        uint16_t len = strlen(status_msg);
        if (len < ts_msg_tailroom(msg)) {
            /* Size of string and 1 extra character space for '.' available */
            (void)ts_msg_add_mem(msg, (const uint8_t *)status_msg, len);
        }
    }

    (void)ts_msg_add_u8(msg, '.');  /* 1 byte */

    if ((code == TS_MSG_CODE_CONTENT) || (code == TS_MSG_CODE_EXPORT)) {
        /* Extra JSON data will follow - prepare for json encoding */
        return ts_msg_json_enc_setup(msg);
    }

    return 0;
}

int ts_msg_add_statement_cbor(struct thingset_msg *msg, thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "MSG CBOR: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (!ts_obj_db_oref_is_object(oref)) {
        TS_LOGE("MSG CBOR: %s on invalid object reference (%u:%u)",
                __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        return -EINVAL;
    }
    if (ts_obj_parent_id(oref) != TS_ID_ROOT) {
        // currently only supporting top level objects
        TS_LOGD("MSG CBOR: %s - object '%s' is not top level - not supported", __func__,
                ts_obj_name(oref));
        return 0;
    }

    int ret = 0;
    if ((ts_obj_type(oref) == TS_T_SUBSET) || (ts_obj_type(oref) == TS_T_GROUP)) {
        ret = ts_msg_add_u8(msg, TS_MSG_CODE_BIN_STATEMENT);
        if (ret != 0) {
            return ret;
        }
        /* Switch to cbor encoding for the rest of the message */
        ret = ts_msg_cbor_enc_setup(msg);
        if (ret != 0) {
            return ret;
        }
        /* serialize object id */
        ret = ts_msg_add_u16_cbor(msg, ts_obj_id(oref));
        if (ret != 0) {
            return ret;
        }

        /* find out number of elements to be serialized */
        int num_elements = 0;
        thingset_oref_t child_oref = oref;
        for (child_oref.db_oid = 0; ts_obj_db_oref_is_object(child_oref); child_oref.db_oid++) {
            if (((ts_obj_type(oref) == TS_T_SUBSET) && (ts_obj_subsets(child_oref)
                                                        & ts_obj_detail(oref))) ||
                ((ts_obj_type(oref) == TS_T_GROUP) && (ts_obj_parent_id(child_oref)
                                                       == ts_obj_id(oref)))) {
                num_elements++;
            }
        }

        /* Add array of element values */
        ret = ts_msg_add_array_cbor(msg, num_elements);
        if (ret != 0) {
            return ret;
        }
        for (child_oref.db_oid = 0; ts_obj_db_oref_is_object(child_oref); child_oref.db_oid++) {
            if (((ts_obj_type(oref) == TS_T_SUBSET) && (ts_obj_subsets(child_oref)
                                                        & ts_obj_detail(oref))) ||
                ((ts_obj_type(oref) == TS_T_GROUP) && (ts_obj_parent_id(child_oref)
                                                       == ts_obj_id(oref)))) {
                ret = ts_msg_add_object_cbor(msg, child_oref);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        ret = ts_msg_add_array_end_cbor(msg);
    }
    return ret;
}

int ts_msg_add_statement_json(struct thingset_msg *msg, thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "MSG JSON: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (!ts_obj_db_oref_is_object(oref)) {
        TS_LOGE("MSG JSON: %s on invalid object reference (%u:%u)",
                __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        return -EINVAL;
    }
    if (ts_obj_parent_id(oref) != TS_ID_ROOT) {
        // currently only supporting top level objects
        TS_LOGD("MSG JSON: %s - object '%s' is not top level - not supported", __func__,
                ts_obj_name(oref));
        return 0;
    }

    int ret = ts_msg_json_enc_setup(msg);
    if (ret != 0) {
        return ret;
    }

    if ((ts_obj_type(oref) == TS_T_SUBSET) || (ts_obj_type(oref) == TS_T_GROUP)) {
        ret = ts_msg_add_u8(msg, TS_MSG_CODE_TXT_STATEMENT);
        if (ret != 0) {
            return ret;
        }
        const char *name = ts_obj_name(oref);
        ret = ts_msg_add_mem(msg, name, strlen(name));
        if (ret != 0) {
            return ret;
        }
        ret = ts_msg_add_u8(msg, ' ');
        if (ret != 0) {
            return ret;
        }

        /* find out number of elements to be serialized */
        int num_elements = 0;
        thingset_oref_t child_oref = oref;
        for (child_oref.db_oid = 0; ts_obj_db_oref_is_object(child_oref); child_oref.db_oid++) {
            if (((ts_obj_type(oref) == TS_T_SUBSET) && (ts_obj_subsets(child_oref)
                                                        & ts_obj_detail(oref))) ||
                ((ts_obj_type(oref) == TS_T_GROUP) && (ts_obj_parent_id(child_oref)
                                                       == ts_obj_id(oref)))) {
                num_elements++;
            }
        }

        /* Add map of elements */
        ret = ts_msg_add_map_json(msg, num_elements);
        if (ret != 0) {
            return ret;
        }
        for (child_oref.db_oid = 0; ts_obj_db_oref_is_object(child_oref); child_oref.db_oid++) {
            if (((ts_obj_type(oref) == TS_T_SUBSET) && (ts_obj_subsets(child_oref)
                                                        & ts_obj_detail(oref))) ||
                ((ts_obj_type(oref) == TS_T_GROUP) && (ts_obj_parent_id(child_oref)
                                                       == ts_obj_id(oref)))) {
                ret = ts_msg_add_string_json(msg, ts_obj_name(child_oref));
                if (ret != 0) {
                    return ret;
                }
                ret = ts_msg_add_object_json(msg, child_oref);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        ret = ts_msg_add_map_end_json(msg);
    }
    return ret;
}

int ts_msg_pull_status(struct thingset_msg *msg, ts_msg_status_code_t *code,
                       const char **description,  uint16_t *len)
{
    if (ts_msg_len(msg) < 2) {
        return -ENOMEM;
    }

    const uint8_t *s = ts_msg_data(msg);
    if (s >= ts_msg_tail(msg) - 3) {
        return -ENOMEM;
    }
    int status_code = 0;
    for(int i = 0; i < 2; i++) {
        status_code = status_code * 16;
        /* Find the decimal representation of hex[i] */
        if(s[i] >= '0' && s[i] <= '9')
        {
            status_code += s[i] - (uint8_t)'0';
        }
        else if(s[i] >= 'a' && s[i] <= 'f')
        {
            status_code += s[i] - (uint8_t)'a' + 10;
        }
        else if(s[i] >= 'A' && s[i] <= 'F')
        {
            status_code += s[i] - (uint8_t)'A' + 10;
        }
        else {
            return -EINVAL;
        }
    }
    if (s[2] == '.') {
        /* No description */
        *description = &s[2];
        *len = 1;
        *code = status_code;
        (void)ts_msg_pull(msg, 3);
        return 0;
    }
    s = &s[2];
    /* skip space */
    while (isspace((char)*s) && (s < ts_msg_tail(msg))) {
        s++;
    }
    if (s >= ts_msg_tail(msg)) {
        return -ENOMEM;
    }
    /* search '.' */
    const char *desc = (const char *)s;
    while (s < ts_msg_tail(msg)) {
        if ((char)*s == '.') {
            /* Description found */
            *description = desc;
            *len = s + 1 - (const uint8_t *)desc;
            *code = status_code;
            (void)ts_msg_pull(msg, s + 1 - ts_msg_data(msg));
            return 0;
        }
        s++;
    }
    return -EINVAL;
}

int ts_msg_pull_object_cbor(struct thingset_msg *msg, thingset_oref_t oref, bool patch)
{
    int ret = 0;

    switch (ts_obj_type(oref)) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        {
            uint64_t val_u64;
            ret = ts_msg_pull_u64_cbor(msg, &val_u64);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u64_data(oref) = val_u64;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u64_data(oref) = (uint64_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT64:
        {
            int64_t val_i64;
            ret = ts_msg_pull_i64_cbor(msg, &val_i64);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i64_data(oref) = val_i64;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i64_data(oref) = (int64_t)val_f32;
                }
                break;
            }
        }
        break;
#endif
    case TS_T_UINT32:
        {
            uint32_t val_u32;
            ret = ts_msg_pull_u32_cbor(msg, &val_u32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u32_data(oref) = val_u32;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u32_data(oref) = (uint32_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT32:
        {
            int32_t val_i32;
            ret = ts_msg_pull_i32_cbor(msg, &val_i32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i32_data(oref) = val_i32;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i32_data(oref) = (int32_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_UINT16:
        {
            uint16_t val_u16;
            ret = ts_msg_pull_u16_cbor(msg, &val_u16);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u16_data(oref) = val_u16;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u16_data(oref) = (uint16_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT16:
        {
            int16_t val_i16;
            ret = ts_msg_pull_i16_cbor(msg, &val_i16);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i16_data(oref) = val_i16;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i16_data(oref) = (int16_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_FLOAT32:
        if (patch) {
            ret = ts_msg_pull_f32_cbor(msg, ts_obj_f32_data(oref));
        }
        else {
            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
        }
        break;
#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        {
            int32_t val_mantissa;
            int16_t val_exponent;
            ret = ts_msg_pull_decfrac_cbor(msg, &val_mantissa, &val_exponent);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);
                    for (int16_t i = val_exponent; i < exponent; i++) {
                        val_mantissa /= 10;
                    }
                    for (int16_t i = val_exponent; i > exponent; i--) {
                        val_mantissa *= 10;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = val_mantissa;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_cbor(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);

                    for (int16_t i = 0; i < exponent; i++) {
                        val_f32 /= 10.0F;
                    }
                    for (int i = 0; i > exponent; i--) {
                        val_f32 *= 10.0F;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = (int32_t)val_f32;
                }
                break;
            }

            ret = ts_msg_pull_i32_cbor(msg, &val_mantissa);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);

                    for (int16_t i = 0; i < exponent; i++) {
                        val_mantissa /= 10;
                    }
                    for (int i = 0; i > exponent; i--) {
                        val_mantissa *= 10;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = val_mantissa;
                }
                break;
            }
        }
        break;
#endif
    case TS_T_BOOL:
        if (patch) {
            ret = ts_msg_pull_bool_cbor(msg, ts_obj_bool_data(oref));
        }
        else {
            bool val_b;
            ret = ts_msg_pull_bool_cbor(msg, &val_b);
        }
        break;
    case TS_T_STRING:
        {
            const char *s;
            uint16_t len;
            ret = ts_msg_pull_string_cbor(msg, &s, &len);
            if (ret != 0) {
                break;
            }
            if ((len + 1) >= ts_obj_detail(oref)) {
                /* String does not fit into object */
                ret = -ENOMEM;
                break;
            }
            if (patch) {
                (void)strncpy(ts_obj_string_data(oref), s, (size_t)len);
                ts_obj_string_data(oref)[len] = '\0';
            }
        }
        break;
#if TS_CONFIG_BYTE_STRING_TYPE_SUPPORT
    case TS_T_BYTES:
        {
            const uint8_t *mem_data;
            uint16_t mem_len;
            ret = ts_msg_pull_mem_cbor(msg, &mem_data, &mem_len);
            if (ret != 0) {
                break;
            }
            if (mem_len > ts_obj_detail(oref)) {
                ret = -ENOMEM;
                break;
            }
            if (patch) {
                (void)memcpy(ts_obj_mem_data(oref), mem_data, mem_len);
                *ts_obj_mem_len(oref) = mem_len;
            }
        }
        break;
#endif
    case TS_T_ARRAY:
        {
            struct ts_array_info *array_info = ts_obj_array_data(oref);
            uint16_t num_elements;
            ret = ts_msg_pull_array_cbor(msg, &num_elements);
            if (ret != 0) {
                break;
            }
            if (num_elements > array_info->max_elements) {
                ret = -ENOMEM;
                break;
            }
            for (int i = 0; (i < num_elements) && (ret == 0); i++) {
                switch (array_info->type) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
                case TS_T_UINT64:
                    if (patch) {
                        ret = ts_msg_pull_u64_cbor(msg, &(((uint64_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint64_t val_u64;
                        ret = ts_msg_pull_u64_cbor(msg, &val_u64);
                    }
                    break;
                case TS_T_INT64:
                    if (patch) {
                        ret = ts_msg_pull_i64_cbor(msg, &(((int64_t *)array_info->ptr)[i]));
                    }
                    else {
                        int64_t val_i64;
                        ret = ts_msg_pull_i64_cbor(msg, &val_i64);
                    }
                    break;
#endif
                case TS_T_UINT32:
                    if (patch) {
                        ret = ts_msg_pull_u32_cbor(msg, &(((uint32_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint32_t val_u32;
                        ret = ts_msg_pull_u32_cbor(msg, &val_u32);
                    }
                    break;
                case TS_T_INT32:
                    if (patch) {
                        ret = ts_msg_pull_i32_cbor(msg, &(((int32_t *)array_info->ptr)[i]));
                    }
                    else {
                        int32_t val_i32;
                        ret = ts_msg_pull_i32_cbor(msg, &val_i32);
                    }
                    break;
                case TS_T_UINT16:
                    if (patch) {
                        ret = ts_msg_pull_u16_cbor(msg, &(((uint16_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint16_t val_u16;
                        ret = ts_msg_pull_u16_cbor(msg, &val_u16);
                    }
                    break;
                case TS_T_INT16:
                    if (patch) {
                        ret = ts_msg_pull_i16_cbor(msg, &(((int16_t *)array_info->ptr)[i]));
                    }
                    else {
                        int16_t val_i16;
                        ret = ts_msg_pull_i16_cbor(msg, &val_i16);
                    }
                    break;
                case TS_T_FLOAT32:
                    if (patch) {
                        ret = ts_msg_pull_f32_cbor(msg, &(((float *)array_info->ptr)[i]));
                    }
                    else {
                        float val_f32;
                        ret = ts_msg_pull_f32_cbor(msg, &val_f32);
                    }
                    break;
                default:
                    TS_LOGD("ThingSet JSON: %s - unexpected type (%u)", __func__,
                            (unsigned int)ts_obj_type(oref));
                    if (!patch) {
                        ret = -EINVAL;
                    }
                    break;
                }
            }
            if (ret != 0) {
                break;
            }
            return ts_msg_pull_array_end_cbor(msg);
        }
        break;
    default:
        TS_LOGD("ThingSet JSON: %s - unexpected type (%u)", __func__,
                (unsigned int)ts_obj_type(oref));
        if (!patch) {
            ret = -EINVAL;
        }
        break;
    }
    return ret;
}

int ts_msg_pull_object_json(struct thingset_msg *msg, thingset_oref_t oref, bool patch)
{
    int ret = 0;

    switch (ts_obj_type(oref)) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        {
            uint64_t val_u64;
            ret = ts_msg_pull_u64_json(msg, &val_u64);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u64_data(oref) = val_u64;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u64_data(oref) = (uint64_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT64:
        {
            int64_t val_i64;
            ret = ts_msg_pull_i64_json(msg, &val_i64);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i64_data(oref) = val_i64;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i64_data(oref) = (int64_t)val_f32;
                }
                break;
            }
        }
        break;
#endif
    case TS_T_UINT32:
        {
            uint32_t val_u32;
            ret = ts_msg_pull_u32_json(msg, &val_u32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u32_data(oref) = val_u32;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u32_data(oref) = (uint32_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT32:
        {
            int32_t val_i32;
            ret = ts_msg_pull_i32_json(msg, &val_i32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i32_data(oref) = val_i32;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i32_data(oref) = (int32_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_UINT16:
        {
            uint16_t val_u16;
            ret = ts_msg_pull_u16_json(msg, &val_u16);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u16_data(oref) = val_u16;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_u16_data(oref) = (uint16_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_INT16:
        {
            int16_t val_i16;
            ret = ts_msg_pull_i16_json(msg, &val_i16);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i16_data(oref) = val_i16;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    *ts_obj_i16_data(oref) = (int16_t)val_f32;
                }
                break;
            }
        }
        break;
    case TS_T_FLOAT32:
        if (patch) {
            ret = ts_msg_pull_f32_json(msg, ts_obj_f32_data(oref));
        }
        else {
            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
        }
        break;
#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        {
            int32_t val_mantissa;
            int16_t val_exponent;
            ret = ts_msg_pull_decfrac_json(msg, &val_mantissa, &val_exponent);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);
                    for (int16_t i = val_exponent; i < exponent; i++) {
                        val_mantissa /= 10;
                    }
                    for (int16_t i = val_exponent; i > exponent; i--) {
                        val_mantissa *= 10;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = val_mantissa;
                }
                break;
            }

            float val_f32;
            ret = ts_msg_pull_f32_json(msg, &val_f32);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);

                    for (int16_t i = 0; i < exponent; i++) {
                        val_f32 /= 10.0F;
                    }
                    for (int i = 0; i > exponent; i--) {
                        val_f32 *= 10.0F;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = (int32_t)val_f32;
                }
                break;
            }

            ret = ts_msg_pull_i32_json(msg, &val_mantissa);
            if (ret == 0) {
                if (patch) {
                    int16_t exponent = ts_obj_decfrac_exponent_data(oref);

                    for (int16_t i = 0; i < exponent; i++) {
                        val_mantissa /= 10;
                    }
                    for (int i = 0; i > exponent; i--) {
                        val_mantissa *= 10;
                    }
                    *ts_obj_decfrac_mantissa_data(oref) = val_mantissa;
                }
                break;
            }
        }
        break;
#endif
    case TS_T_BOOL:
        if (patch) {
            ret = ts_msg_pull_bool_json(msg, ts_obj_bool_data(oref));
        }
        else {
            bool val_b;
            ret = ts_msg_pull_bool_json(msg, &val_b);
        }
        break;
    case TS_T_STRING:
        {
            const char *s;
            uint16_t len;
            ret = ts_msg_pull_string_json(msg, &s, &len);
            if (ret != 0) {
                break;
            }
            if ((len + 1) >= ts_obj_detail(oref)) {
                /* String does not fit into object */
                ret = -ENOMEM;
                break;
            }
            if (patch) {
                (void)strncpy(ts_obj_string_data(oref), s, (size_t)len);
                ts_obj_string_data(oref)[len] = '\0';
            }
        }
        break;
#if TS_CONFIG_BYTE_STRING_TYPE_SUPPORT
    case TS_T_BYTES:
        return -ENOTSUP;
#endif
    case TS_T_ARRAY:
        {
            struct ts_array_info *array_info = ts_obj_array_data(oref);
            uint16_t num_elements;
            ret = ts_msg_pull_array_json(msg, &num_elements);
            if (ret != 0) {
                break;
            }
            if (num_elements > array_info->max_elements) {
                ret = -ENOMEM;
                break;
            }
            for (int i = 0; (i < num_elements) && (ret == 0); i++) {
                switch (array_info->type) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
                case TS_T_UINT64:
                    if (patch) {
                        ret = ts_msg_pull_u64_json(msg, &(((uint64_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint64_t val_u64;
                        ret = ts_msg_pull_u64_json(msg, &val_u64);
                    }
                    break;
                case TS_T_INT64:
                    if (patch) {
                        ret = ts_msg_pull_i64_json(msg, &(((int64_t *)array_info->ptr)[i]));
                    }
                    else {
                        int64_t val_i64;
                        ret = ts_msg_pull_i64_json(msg, &val_i64);
                    }
                    break;
#endif
                case TS_T_UINT32:
                    if (patch) {
                        ret = ts_msg_pull_u32_json(msg, &(((uint32_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint32_t val_u32;
                        ret = ts_msg_pull_u32_json(msg, &val_u32);
                    }
                    break;
                case TS_T_INT32:
                    if (patch) {
                        ret = ts_msg_pull_i32_json(msg, &(((int32_t *)array_info->ptr)[i]));
                    }
                    else {
                        int32_t val_i32;
                        ret = ts_msg_pull_i32_json(msg, &val_i32);
                    }
                    break;
                case TS_T_UINT16:
                    if (patch) {
                        ret = ts_msg_pull_u16_json(msg, &(((uint16_t *)array_info->ptr)[i]));
                    }
                    else {
                        uint16_t val_u16;
                        ret = ts_msg_pull_u16_json(msg, &val_u16);
                    }
                    break;
                case TS_T_INT16:
                    if (patch) {
                        ret = ts_msg_pull_i16_json(msg, &(((int16_t *)array_info->ptr)[i]));
                    }
                    else {
                        int16_t val_i16;
                        ret = ts_msg_pull_i16_json(msg, &val_i16);
                    }
                    break;
                case TS_T_FLOAT32:
                    if (patch) {
                        ret = ts_msg_pull_f32_json(msg, &(((float *)array_info->ptr)[i]));
                    }
                    else {
                        float val_f32;
                        ret = ts_msg_pull_f32_json(msg, &val_f32);
                    }
                    break;
                default:
                    TS_LOGD("ThingSet JSON: %s - unexpected type (%u)", __func__,
                            (unsigned int)ts_obj_type(oref));
                    if (!patch) {
                        ret = -EINVAL;
                    }
                    break;
                }
            }
            if (ret != 0) {
                break;
            }
            return ts_msg_pull_array_end_json(msg);
        }
        break;
    default:
        TS_LOGD("ThingSet JSON: %s - unexpected type (%u)", __func__,
                (unsigned int)ts_obj_type(oref));
        if (!patch) {
            ret = -EINVAL;
        }
        break;
    }
    return ret;
}

int ts_msg_pull_request_cbor(struct thingset_msg *msg, thingset_oref_t *oref)
{
    TS_ASSERT(oref->db_id != TS_OBJ_DB_ID_INVALID, "object reference has to declare database");
    TS_ASSERT(ts_msg_status_type(msg) == TS_MSG_TYPE_REQUEST, "Unexpected message type (%d)",
              (int)ts_msg_status_type(msg));
    TS_ASSERT(ts_msg_status_proto(msg) == TS_MSG_PROTO_BIN, "Unexpected message proto (%d)",
              (int)ts_msg_status_proto(msg));

    uint8_t req_method_id;
    int ret = ts_msg_pull_u8(msg, &req_method_id);
    if (ret != 0) {
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_cbor_end;
    }

    ret = ts_msg_cbor_dec_setup(msg);
    if (ret != 0) {
        /* Something went wrong in msg init */
        ts_msg_status_error_set(msg, TS_MSG_CODE_INTERNAL_SERVER_ERR);
        goto ts_msg_pull_request_cbor_end;
    }

    CborType path_type;
    ret = ts_msg_pull_type_cbor(msg, &path_type);
    if (ret != 0) {
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_cbor_end;
    }

    /* Get data object (first parameter of the request) */
    if (path_type == CborIntegerType) {
        ts_obj_id_t obj_id;
        ret = ts_msg_pull_u16_cbor(msg, &obj_id);
        if (ret != 0) {
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            goto ts_msg_pull_request_cbor_end;
        }
        ret = ts_obj_db_oref_by_id(oref->db_id, obj_id, oref);
        if (ret != 0) {
            /* We did not get the object reference - assure it is invalid */
            ts_obj_db_oref_init(oref->db_id, oref);
        }
    }
    else if (path_type == CborTextStringType) {
        const char *path;
        uint16_t path_len;
        ret = ts_msg_pull_string_cbor(msg, &path, &path_len);
        if (ret != 0) {
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            goto ts_msg_pull_request_cbor_end;
        }
        if (path[0] == '/') {
            if ((req_method_id == TS_GET) &&  (path_len == 1)) {
                *oref = ts_obj_db_oref_root(oref->db_id);
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_NAMES);
                goto ts_msg_pull_request_cbor_end;
            }
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            goto ts_msg_pull_request_cbor_end;
        }
        if ((path[path_len - 1] == '/')) {
            path_len -= 1;
        }
        ret = ts_obj_by_path(ts_obj_db_oref_any(oref->db_id), path, path_len, oref);
        if (ret != 0) {
            /* We did not get the object reference - assure it is invalid */
            ts_obj_db_oref_init(oref->db_id, oref);
            if (path_len > 0) {
                ts_msg_status_error_set(msg, TS_MSG_CODE_NOT_FOUND);
                goto ts_msg_pull_request_cbor_end;
            }
        }
    } else {
        /* First data object is of unexpected type */
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_cbor_end;
    }

    if (req_method_id == TS_GET) {
        if (path_type == CborTextStringType) {
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
        }
        else {
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_IDS_VALUES);
        }
    }
    else if (ts_msg_len(msg) == 0) {
        /* no payload data */
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
    }
    else if (req_method_id == TS_POST) {
        if (ts_obj_type(*oref) == TS_T_EXEC) {
            // object is generally executable, but are we authorized?
            if (ts_obj_access_write(*oref, TS_WRITE_MASK)) {
                if (!ts_obj_access_write(*oref, ts_msg_auth(msg))) {
                    ts_msg_status_error_set(msg, TS_MSG_CODE_UNAUTHORIZED);
                }
                else {
                    ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_EXEC);
                }
            } else {
                ts_msg_status_error_set(msg, TS_MSG_CODE_FORBIDDEN);
            }
        }
        else {
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_CREATE);
        }
    }
    else if (req_method_id == TS_DELETE) {
        if ((ts_obj_type(*oref) != TS_T_ARRAY) && (ts_obj_type(*oref) != TS_T_SUBSET)) {
            ts_msg_status_error_set(msg, TS_MSG_CODE_METHOD_NOT_ALLOWED);
        }
        else if (ts_obj_access_write(*oref, TS_WRITE_MASK)) {
            if (!ts_obj_access_write(*oref, ts_msg_auth(msg))) {
                ts_msg_status_error_set(msg, TS_MSG_CODE_UNAUTHORIZED);
            }
            else {
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_DELETE);
            }
        } else {
            ts_msg_status_error_set(msg, TS_MSG_CODE_FORBIDDEN);
        }
    }
    else if (req_method_id == TS_FETCH) {
        CborType t;
        ret = ts_msg_pull_type_cbor(msg, &t);
        if (ret != 0) {
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        }
        else if (t == CborUndefinedType) {
            /* Request all ids/ names behind endpoint */
            if (path_type == CborTextStringType) {
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_FETCH_NAMES);
            }
            else {
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_FETCH_IDS);
            }
        }
        else if (t == CborArrayType) {
            /* Request data defined by names/ids in array */
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_FETCH_VALUES);
        }
        else if (((t == CborTextStringType)  && (path_type == CborTextStringType)) ||
                 ((t == CborIntegerType)  && (path_type == CborIntegerType))) {
            /* Request single data defined by name/id */
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_FETCH_SINGLE);
        }
        else {
            /* Unexpected fetch request */
            TS_LOGD("%s: unexpected fetch value of type %u", __func__, (unsigned int)t);
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        }
    }
    else if (req_method_id == TS_PATCH) {
        if (ts_obj_db_oref_is_object(*oref)) {
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_PATCH);
        }
        else {
            ts_msg_status_error_set(msg, TS_MSG_CODE_NOT_FOUND);
        }
    }
    else {
        /* Unknown/ invalid command */
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
    }

ts_msg_pull_request_cbor_end:
    if (ts_msg_status_valid(msg) != TS_MSG_VALID_OK) {
        /* Assure object reference is invalid */
        ts_obj_db_oref_init(oref->db_id, oref);
    }

    return 0;
}

int ts_msg_pull_request_json(struct thingset_msg *msg, thingset_oref_t *oref)
{
    TS_ASSERT(oref->db_id != TS_OBJ_DB_ID_INVALID, "object reference has to declare database");
    TS_ASSERT(ts_msg_status_type(msg) == TS_MSG_TYPE_REQUEST, "Unexpected message type (%d)",
              (int)ts_msg_status_type(msg));
    TS_ASSERT(ts_msg_status_proto(msg) == TS_MSG_PROTO_TXT, "Unexpected message type (%d)",
              (int)ts_msg_status_proto(msg));

    uint8_t req_method_id;
    int ret = ts_msg_pull_u8(msg, &req_method_id);
    if (ret != 0) {
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_json_end;
    }

    const char *path;
    uint16_t path_len;
    ret = ts_msg_pull_path(msg, &path, &path_len);
    if (ret != 0) {
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_json_end;
    }

    /* Get data object (first parameter of the request) */
    if (path_len == 0) {
        /* Take empty path as root path */
        *oref = ts_obj_db_oref_root(oref->db_id);
    }
    else if (path[0] == '/') {
        if ((req_method_id == '?') &&  (path_len == 1)) {
            *oref = ts_obj_db_oref_root(oref->db_id);
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_NAMES);
            goto ts_msg_pull_request_json_end;
        }
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        goto ts_msg_pull_request_json_end;
    }
    else {
        uint16_t object_path_len = path_len;
        if ((path[path_len - 1] == '/')) {
            object_path_len -= 1;
        }
        ret = ts_obj_by_path(ts_obj_db_oref_any(oref->db_id), path, object_path_len, oref);
        if (ret != 0) {
            /* We did not get the object reference - assure it is invalid */
            ts_obj_db_oref_init(oref->db_id, oref);
            if (object_path_len > 0) {
                ts_msg_status_error_set(msg, TS_MSG_CODE_NOT_FOUND);
                goto ts_msg_pull_request_json_end;
            }
        }
    }

    if (req_method_id == '?') {
        /* GET, FETCH */
        if (ts_msg_len(msg) == 0) {
            // no payload data
            if ((path_len > 0) && (path[path_len - 1] == '/')) {
                if (ts_obj_db_oref_is_tree(*oref) &&
                    (ts_obj_type(*oref) == TS_T_GROUP || ts_obj_type(*oref) == TS_T_EXEC)) {
                    ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_NAMES);
                }
                else {
                    // device discovery is only allowed for internal objects
                    ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
                }
            }
            else if (ts_obj_db_oref_is_object(*oref) && (ts_obj_type(*oref) == TS_T_EXEC)) {
                // bad request, as we can't read exec object's values
                ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            }
            else {
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
            }
        }
        else {
            ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_FETCH_VALUES);
        }
    }
    else if (req_method_id == '!') {
        /* EXEC */
        if (ts_obj_db_oref_is_object(*oref) && (ts_obj_type(*oref) == TS_T_EXEC)) {
            // object is generally executable, but are we authorized?
            if (ts_obj_access_write(*oref, TS_WRITE_MASK)) {
                if (!ts_obj_access_write(*oref, ts_msg_auth(msg))) {
                    ts_msg_status_error_set(msg, TS_MSG_CODE_UNAUTHORIZED);
                }
                else {
                    ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_EXEC);
                }
            } else {
               ts_msg_status_error_set(msg, TS_MSG_CODE_FORBIDDEN);
            }
        }
        else {
            ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
        }
    }
    else if (ts_msg_len(msg) == 0) {
        // no payload data
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
    }
    else if (req_method_id == '=') {
        /* PATCH */
        ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_PATCH);
    }
    else if (req_method_id == '+') {
        /* CREATE */
        ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_CREATE);
    }
    else if (req_method_id == '-') {
        /* DELETE */
        if ((ts_obj_type(*oref) != TS_T_ARRAY) && (ts_obj_type(*oref) != TS_T_SUBSET)) {
            ts_msg_status_error_set(msg, TS_MSG_CODE_METHOD_NOT_ALLOWED);
        }
        else if (ts_obj_access_write(*oref, TS_WRITE_MASK)) {
            if (!ts_obj_access_write(*oref, ts_msg_auth(msg))) {
                ts_msg_status_error_set(msg, TS_MSG_CODE_UNAUTHORIZED);
            }
            else {
                ts_msg_status_code_set(msg, TS_MSG_CODE_REQUEST_DELETE);
            }
        } else {
            ts_msg_status_error_set(msg, TS_MSG_CODE_FORBIDDEN);
        }
    }
    else {
        /* Unknown/ invalid command */
        ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
    }

ts_msg_pull_request_json_end:
    if (ts_msg_status_valid(msg) != TS_MSG_VALID_OK) {
        /* Assure object reference is invalid */
        ts_obj_db_oref_init(oref->db_id, oref);
    }
    else if (ts_msg_len(msg) > 0) {
        /*  There is some extra content in the message */
        ret = ts_msg_json_dec_setup(msg);
        if (ret != 0) {
            ts_obj_db_oref_init(oref->db_id, oref);
            if (ret == -ENOMEM) {
                /* Could not parse - not enough memory */
                ts_msg_status_error_set(msg, TS_MSG_CODE_REQUEST_TOO_LARGE);
            }
            else if ((ret == -EINVAL) || (ret == -ENAVAIL)) {
                /* Invalid character inside JSON string or the string is not a full JSON packet. */
                ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            }
            else if (ret == -ENODATA) {
                /* No data in JSON string */
                ts_msg_status_error_set(msg, TS_MSG_CODE_BAD_REQUEST);
            }
            else {
                /* Should not happen */
                ts_msg_status_error_set(msg, TS_MSG_CODE_INTERNAL_SERVER_ERR);
            }
        }
    }

    return 0;
}
