/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_priv.h"

#include "cbor.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>  // for definition of endianness
#include <math.h>       // for rounding of floats

static int cbor_deserialize_simple_value(const uint8_t *buf, void *data, int type, int detail)
{
    switch (type) {
#if TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return cbor_deserialize_uint64(buf, (uint64_t *)data);
    case TS_T_INT64:
        return cbor_deserialize_int64(buf, (int64_t *)data);
#endif
    case TS_T_UINT32:
        return cbor_deserialize_uint32(buf, (uint32_t *)data);
    case TS_T_INT32:
        return cbor_deserialize_int32(buf, (int32_t *)data);
    case TS_T_UINT16:
        return cbor_deserialize_uint16(buf, (uint16_t *)data);
    case TS_T_INT16:
        return cbor_deserialize_int16(buf, (int16_t *)data);
    case TS_T_FLOAT32:
        return cbor_deserialize_float(buf, (float *)data);
#if TS_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        return cbor_deserialize_decfrac(buf, (int32_t *)data, detail);
#endif
    case TS_T_BOOL:
        return cbor_deserialize_bool(buf, (bool *)data);
    case TS_T_STRING:
        return cbor_deserialize_string(buf, (char *)data, detail);
    default:
        return 0;
    }
}

static int cbor_deserialize_data_obj(const uint8_t *buf, const struct ts_data_object *object)
{
    int pos = cbor_deserialize_simple_value(buf, object->data, object->type, object->detail);
    if (pos > 0) {
        return pos;
    }

    switch (object->type) {
#if TS_BYTE_STRING_TYPE_SUPPORT
    case TS_T_BYTES:
        return cbor_deserialize_bytes(buf, ((struct ts_bytes_buffer *)object->data)->bytes,
            object->detail, &(((struct ts_bytes_buffer *)object->data)->num_bytes));
#endif
    case TS_T_ARRAY: {
        struct ts_array *array = (struct ts_array *)object->data;
        if (!array) {
            return 0;
        }

        // Deserialize the buffer length, and calculate the actual number of array elements
        uint16_t num_elements;
        pos = cbor_num_elements(buf, &num_elements);
        if (num_elements > array->max_elements) {
            return 0;
        }

        for (int i = 0; i < num_elements; i++) {
            void *data = (uint8_t *)array->elements + i * array->type_size;
            pos += cbor_deserialize_simple_value(buf + pos, data, array->type, object->detail);
        }
        return pos;
    }
    default:
        return 0;
    }
}

static int cbor_serialize_simple_value(uint8_t *buf, size_t size, void *data, int type, int detail)
{
    switch (type) {
#if TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return cbor_serialize_uint(buf, *((uint64_t *)data), size);
    case TS_T_INT64:
        return cbor_serialize_int(buf, *((int64_t *)data), size);
#endif
    case TS_T_UINT32:
        return cbor_serialize_uint(buf, *((uint32_t *)data), size);
    case TS_T_INT32:
        return cbor_serialize_int(buf, *((int32_t *)data), size);
    case TS_T_UINT16:
        return cbor_serialize_uint(buf, *((uint16_t *)data), size);
    case TS_T_INT16:
        return cbor_serialize_int(buf, *((int16_t *)data), size);
    case TS_T_FLOAT32:
        if (detail == 0) { // round to 0 digits: use int
#if TS_64BIT_TYPES_SUPPORT
            return cbor_serialize_int(buf, llroundf(*((float *)data)), size);
#else
            return cbor_serialize_int(buf, lroundf(*((float *)data)), size);
#endif
        }
        else {
            return cbor_serialize_float(buf, *((float *)data), size);
        }
#if TS_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        return cbor_serialize_decfrac(buf, *((int32_t *)data), detail, size);
#endif
    case TS_T_BOOL:
        return cbor_serialize_bool(buf, *((bool *)data), size);
    case TS_T_STRING:
        return cbor_serialize_string(buf, (char *)data, size);
    default:
        return 0;
    }
}

static int cbor_serialize_data_obj(uint8_t *buf, size_t size, const struct ts_data_object *object)
{
    int pos = cbor_serialize_simple_value(buf, size, object->data, object->type, object->detail);
    if (pos > 0) {
        return pos;
    }

    switch (object->type) {
#if TS_BYTE_STRING_TYPE_SUPPORT
    case TS_T_BYTES:
        return cbor_serialize_bytes(buf, ((struct ts_bytes_buffer *)object->data)->bytes,
            ((struct ts_bytes_buffer *)object->data)->num_bytes, size);
#endif
    case TS_T_ARRAY: {
        struct ts_array *array = (struct ts_array *)object->data;
        if (!array) {
            return 0;
        }
        // Add the length field to the beginning of the CBOR buffer and update the CBOR buffer index
        pos = cbor_serialize_array(buf, array->num_elements, size);

        for (int i = 0; i < array->num_elements; i++) {
            void *data = (uint8_t *)array->elements + i * array->type_size;
            pos += cbor_serialize_simple_value(buf + pos, size - pos,
                data, array->type, object->detail);
        }
        return pos;
    }
    default:
        return 0;
    }
}

int ts_bin_response(struct ts_context *ts, uint8_t code)
{
    if (ts->resp_size > 0) {
        ts->resp[0] = code;
        return 1;
    }
    else {
        return 0;
    }
}

int ts_bin_process(struct ts_context *ts)
{
    int pos = 1;    // current position during data processing
    uint32_t ret_type = 0;

    // get endpoint (first parameter of the request)
    const struct ts_data_object *endpoint = NULL;
    if ((ts->req[pos] & CBOR_TYPE_MASK) == CBOR_TEXT) {
        char *str_start;
        uint16_t str_len;
        pos += cbor_deserialize_string_zero_copy(&ts->req[pos], &str_start, &str_len);
        endpoint = ts_get_object_by_path(ts, str_start, str_len);
        ret_type = TS_RET_NAMES;
    }
    else if (ts->req[pos] == TS_ID_IDS) {
        ret_type = TS_RET_DISCOVERY | TS_RET_IDS;
        pos++;
    }
    else if (ts->req[pos] == TS_ID_PATHS) {
        ret_type = TS_RET_DISCOVERY | TS_RET_PATHS;
        pos++;
    }
    else if ((ts->req[pos] & CBOR_TYPE_MASK) == CBOR_UINT) {
        ts_object_id_t id = 0;
        pos += cbor_deserialize_uint16(&ts->req[pos], &id);
        endpoint = ts_get_object_by_id(ts, id);
        ret_type = TS_RET_IDS;
    }
    else {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // process data
    if (ts->req[0] == TS_GET && endpoint) {
        ret_type |= TS_RET_VALUES;
        return ts_bin_get(ts, endpoint, ret_type);
    }
    else if (ts->req[0] == TS_FETCH) {
        if (ts->req[pos] != CBOR_UNDEFINED) {
            // undefined is used to discover child nodes, otherwise values are requested
            ret_type |= TS_RET_VALUES;
        }
        return ts_bin_fetch(ts, endpoint, ret_type, pos);
    }
    else if (ts->req[0] == TS_PATCH && endpoint) {
        int response = ts_bin_patch(ts, endpoint, pos, ts->_auth_flags, 0);

        // check if endpoint has a callback assigned
        if (endpoint->data != NULL && ts->resp[0] == TS_STATUS_CHANGED) {
            // create function pointer and call function
            void (*fun)(void) = (void(*)(void))endpoint->data;
            fun();
        }
        return response;
    }
    else if (ts->req[0] == TS_POST) {
        return ts_bin_exec(ts, endpoint, pos);
    }
    return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
}

/*
* @warning The endpoint object is currently still ignored. Any found data object is fetched.
*/
int ts_bin_fetch(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type,
                 unsigned int pos_payload)
{
    unsigned int pos_req = pos_payload;
    unsigned int pos_resp = 0;
    uint16_t num_elements, element = 0;

    if (!(ret_type & TS_RET_VALUES)) {
        return ts_bin_get(ts, endpoint, ret_type);
    }

    pos_resp += ts_bin_response(ts, TS_STATUS_CONTENT);   // init response buffer

    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);
    if (num_elements != 1 && (ts->req[pos_payload] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    //printf("fetch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    ts->req[pos_req], ts->req[pos_req+1], ts->req[pos_req+2], ts->req[pos_req+3],
    //    ts->req[pos_req+4], ts->req[pos_req+5], ts->req[pos_req+6], ts->req[pos_req+7]);

    if (num_elements > 1) {
        pos_resp += cbor_serialize_array(&ts->resp[pos_resp], num_elements,
            ts->resp_size - pos_resp);
    }

    while (pos_req < ts->req_len && element < num_elements) {

        const struct ts_data_object* data_obj = NULL;
        if ((ts->req[pos_req] & CBOR_TYPE_MASK) == CBOR_TEXT) {
            char *str_start = "";
            uint16_t str_len;
            pos_req += cbor_deserialize_string_zero_copy(&ts->req[pos_req], &str_start, &str_len);
            if (ret_type & TS_RET_DISCOVERY) {
                // discovery of ID from path, so the string contains entire path and not only name
                data_obj = ts_get_object_by_path(ts, str_start, str_len);
            }
            else {
                data_obj =
                    ts_get_object_by_name(ts, str_start, str_len, endpoint ? endpoint->id : 0);
            }
        }
        else {
            ts_object_id_t id = 0;
            pos_req += cbor_deserialize_uint16(&ts->req[pos_req], &id);
            data_obj = ts_get_object_by_id(ts, id);
        }
        if (data_obj == NULL) {
            return ts_bin_response(ts, TS_STATUS_NOT_FOUND);
        }
        if (!(data_obj->access & TS_READ_MASK)) {
            return ts_bin_response(ts, TS_STATUS_UNAUTHORIZED);
        }

        size_t num_bytes = 0; // temporary storage of cbor data length in response
        if ((ret_type & TS_RET_DISCOVERY) == 0) {
            // "normal" request to fetch values
            num_bytes = cbor_serialize_data_obj(&ts->resp[pos_resp], ts->resp_size - pos_resp,
                data_obj);
        }
        else if (ret_type & TS_RET_PATHS) {
            // request to determine paths from IDs
            char path[30];
            if (ts_get_path(ts, path, sizeof(path), data_obj) <= 0) {
                return ts_bin_response(ts, TS_STATUS_INTERNAL_SERVER_ERR);
            }

            num_bytes = cbor_serialize_string(&ts->resp[pos_resp], path, ts->resp_size - pos_resp);
        }
        else if (ret_type & TS_RET_IDS) {
            // request to determine IDs from paths
            num_bytes =
                cbor_serialize_uint(&ts->resp[pos_resp], data_obj->id, ts->resp_size - pos_resp);
        }

        if (num_bytes == 0) {
            return ts_bin_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
        }
        pos_resp += num_bytes;
        element++;
    }

    if (element == num_elements) {
        return pos_resp;
    }
    else {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
}

int ts_bin_import(struct ts_context *ts, uint8_t *data, size_t len, uint8_t auth_flags,
                  uint16_t subsets)
{
    uint8_t resp_tmp[1] = {};   // only one character as response expected
    ts->req = data;
    ts->req_len = len;
    ts->resp = resp_tmp;
    ts->resp_size = sizeof(resp_tmp);
    ts_bin_patch(ts, NULL, 0, auth_flags, subsets);
    return ts->resp[0];
}

int ts_bin_patch(struct ts_context *ts, const struct ts_data_object *endpoint,
                 unsigned int pos_payload, uint8_t auth_flags, uint16_t subsets)
{
    unsigned int pos_req = pos_payload;
    uint16_t num_elements, element = 0;
    bool updated = false;

    if ((ts->req[pos_req] & CBOR_TYPE_MASK) != CBOR_MAP) {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);

    //printf("patch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    ts->req[pos_req], ts->req[pos_req+1], ts->req[pos_req+2], ts->req[pos_req+3],
    //    ts->req[pos_req+4], ts->req[pos_req+5], ts->req[pos_req+6], ts->req[pos_req+7]);

    while (pos_req < ts->req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        ts_object_id_t id;
        num_bytes = cbor_deserialize_uint16(&ts->req[pos_req], &id);
        if (num_bytes == 0) {
            return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        const struct ts_data_object* object = ts_get_object_by_id(ts, id);
        if (object) {
            if ((object->access & TS_WRITE_MASK & auth_flags) == 0) {
                if (object->access & TS_WRITE_MASK) {
                    return ts_bin_response(ts, TS_STATUS_UNAUTHORIZED);
                }
                else {
                    return ts_bin_response(ts, TS_STATUS_FORBIDDEN);
                }
            }
            else if (endpoint && object->parent != endpoint->id) {
                return ts_bin_response(ts, TS_STATUS_NOT_FOUND);
            }
            else if (subsets && !(object->subsets & subsets)) {
                // ignore element
                num_bytes = cbor_size(&ts->req[pos_req]);
            }
            else {
                // actually deserialize the data and update object
                num_bytes = cbor_deserialize_data_obj(&ts->req[pos_req], object);
                if (ts->_update_subsets & object->subsets) {
                    updated = true;
                }
            }
        }
        else {
            // object not found
            if (subsets) {
                // ignore element
                num_bytes = cbor_size(&ts->req[pos_req]);
            }
            else {
                return ts_bin_response(ts, TS_STATUS_NOT_FOUND);
            }
        }

        if (num_bytes == 0) {
            return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        element++;
    }

    if (element == num_elements) {
        if (updated && ts->update_cb != NULL) {
            ts->update_cb();
        }
        return ts_bin_response(ts, TS_STATUS_CHANGED);
    } else {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
}

int ts_bin_exec(struct ts_context *ts, const struct ts_data_object *object,
                unsigned int pos_payload)
{
    unsigned int pos_req = pos_payload;
    uint16_t num_elements, element = 0;

    if ((ts->req[pos_req] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);

    if ((object->access & TS_WRITE_MASK) && (object->type == TS_T_EXEC)) {
        // object is generally executable, but are we authorized?
        if ((object->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            return ts_bin_response(ts, TS_STATUS_UNAUTHORIZED);
        }
    }
    else {
        return ts_bin_response(ts, TS_STATUS_FORBIDDEN);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent == object->id) {
            if (element >= num_elements) {
                // more child objects found than parameters were passed
                return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
            }
            int num_bytes = cbor_deserialize_data_obj(&ts->req[pos_req], &ts->data_objects[i]);
            if (num_bytes == 0) {
                // deserializing the value was not successful
                return ts_bin_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
            }
            pos_req += num_bytes;
            element++;
        }
    }

    if (num_elements > element) {
        // more parameters passed than child objects found
        return ts_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // if we got here, finally create function pointer and call function
    void (*fun)(void) = (void(*)(void))object->data;
    fun();

    return ts_bin_response(ts, TS_STATUS_VALID);
}

int ts_bin_statement(struct ts_context *ts, uint8_t *buf, size_t buf_size,
                     struct ts_data_object *object)
{
    buf[0] = TS_STATEMENT;
    int len = 1;

    if (!object || object->parent != 0) {
        // currently only supporting top level objects
        return 0;
    }

    // serialize endpoint
    len += cbor_serialize_uint(&buf[len], object->id, buf_size - len);

    if (object->type == TS_T_SUBSET) {
        uint16_t subsets = object->detail;

        // find out number of elements to be serialized
        int num_ids = 0;
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].subsets & subsets) {
                num_ids++;
            }
        }

        len += cbor_serialize_array(&buf[len], num_ids, buf_size - len);

        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].subsets & subsets) {
                size_t num_bytes = cbor_serialize_data_obj(&buf[len], buf_size - len,
                    &ts->data_objects[i]);
                if (num_bytes == 0) {
                    return 0;
                }
                else {
                    len += num_bytes;
                }
            }
        }
    }
    else if (object->type == TS_T_GROUP) {
        // find out number of elements to be serialized
        int num_ids = 0;
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].parent == object->id) {
                num_ids++;
            }
        }

        len += cbor_serialize_array(&buf[len], num_ids, buf_size - len);

        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].parent == object->id) {
                size_t num_bytes = cbor_serialize_data_obj(&buf[len], buf_size - len,
                    &ts->data_objects[i]);
                if (num_bytes == 0) {
                    return 0;
                }
                else {
                    len += num_bytes;
                }
            }
        }
    }
    else {
        return 0;
    }

    return len;
}

int ts_bin_statement_by_path(struct ts_context *ts, uint8_t *buf, size_t buf_size, const char *path)
{
    return ts_bin_statement(ts, buf, buf_size, ts_get_object_by_path(ts, path, strlen(path)));
}

int ts_bin_statement_by_id(struct ts_context *ts, uint8_t *buf, size_t buf_size, ts_object_id_t id)
{
    return ts_bin_statement(ts, buf, buf_size, ts_get_object_by_id(ts, id));
}

int ts_bin_export(struct ts_context *ts, uint8_t *buf, size_t buf_size, uint16_t subsets)
{
    // find out number of elements to be serialized
    int num_ids = 0;
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            num_ids++;
        }
    }

    int len = cbor_serialize_map(buf, num_ids, buf_size);

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            len += cbor_serialize_uint(&buf[len], ts->data_objects[i].id, buf_size - len);
            size_t num_bytes = cbor_serialize_data_obj(&buf[len], buf_size - len,
                &ts->data_objects[i]);
            if (num_bytes == 0) {
                return 0;
            }
            else {
                len += num_bytes;
            }
        }
    }

    return len;
}

int ts_bin_pub_can(struct ts_context *ts, int *start_pos, uint16_t subset, uint8_t can_dev_id,
                   uint32_t *msg_id, uint8_t *msg_data)
{
    int msg_len = -1;

    for (unsigned int i = *start_pos; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subset) {
            *msg_id = TS_CAN_BASE_PUBSUB | TS_CAN_PRIO_PUBSUB_LOW
                | TS_CAN_DATA_ID_SET(ts->data_objects[i].id)
                | TS_CAN_SOURCE_SET(can_dev_id);

            msg_len = cbor_serialize_data_obj(msg_data, 8, &ts->data_objects[i]);

            if (msg_len > 0) {
                // object found and successfully encoded, increase start pos for next run
                *start_pos = i + 1;
                break;
            }
            // else: data too long, take next object
        }
    }

    if (msg_len <= 0) {
        // no more objects found, reset position
        *start_pos = 0;
    }

    return msg_len;
}

int ts_bin_get(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type)
{
    unsigned int len = 0;       // current length of response
    len += ts_bin_response(ts, TS_STATUS_CONTENT);   // init response buffer

    if (endpoint->type != TS_T_GROUP) {
        len += cbor_serialize_data_obj(&ts->resp[len], ts->resp_size - len, endpoint);
        return len;
    }

    // find out number of elements
    int num_elements = 0;
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].access & TS_READ_MASK
            && (ts->data_objects[i].parent == endpoint->id))
        {
            num_elements++;
        }
    }

    if (ret_type & TS_RET_VALUES) {
        len += cbor_serialize_map(&ts->resp[len], num_elements, ts->resp_size - len);
    }
    else {
        len += cbor_serialize_array(&ts->resp[len], num_elements, ts->resp_size - len);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].access & TS_READ_MASK
            && (ts->data_objects[i].parent == endpoint->id))
        {
            int num_bytes = 0;
            if (ret_type & TS_RET_IDS) {
                num_bytes = cbor_serialize_uint(&ts->resp[len], ts->data_objects[i].id,
                    ts->resp_size - len);
            }
            else if (ret_type & TS_RET_NAMES) {
                num_bytes = cbor_serialize_string(&ts->resp[len], ts->data_objects[i].name,
                    ts->resp_size - len);
            }

            if (ret_type & TS_RET_VALUES) {
                num_bytes += cbor_serialize_data_obj(&ts->resp[len + num_bytes],
                    ts->resp_size - len - num_bytes, &ts->data_objects[i]);
            }

            if (num_bytes == 0) {
                return ts_bin_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
            }
            else {
                len += num_bytes;
            }
        }
    }

    return len;
}
