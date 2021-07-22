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

int cbor_deserialize_array_type(uint8_t *buf, const TsDataNode *data_node);
int cbor_serialize_array_type(uint8_t *buf, size_t size, const TsDataNode *data_node);

static int cbor_deserialize_data_node(uint8_t *buf, const TsDataNode *data_node)
{
    switch (data_node->type) {
#if (TS_64BIT_TYPES_SUPPORT == 1)
    case TS_T_UINT64:
        return cbor_deserialize_uint64(buf, (uint64_t *)data_node->data);
    case TS_T_INT64:
        return cbor_deserialize_int64(buf, (int64_t *)data_node->data);
#endif
    case TS_T_UINT32:
        return cbor_deserialize_uint32(buf, (uint32_t *)data_node->data);
    case TS_T_INT32:
        return cbor_deserialize_int32(buf, (int32_t *)data_node->data);
    case TS_T_UINT16:
        return cbor_deserialize_uint16(buf, (uint16_t *)data_node->data);
    case TS_T_INT16:
        return cbor_deserialize_int16(buf, (int16_t *)data_node->data);
    case TS_T_FLOAT32:
        return cbor_deserialize_float(buf, (float *)data_node->data);
    case TS_T_BOOL:
        return cbor_deserialize_bool(buf, (bool *)data_node->data);
    case TS_T_STRING:
        return cbor_deserialize_string(buf, (char *)data_node->data, data_node->detail);
    case TS_T_BYTES:
        return cbor_deserialize_bytes(buf, ((TsBytesBuffer *)data_node->data)->bytes,
            data_node->detail, &(((TsBytesBuffer *)data_node->data)->num_bytes));
    case TS_T_ARRAY:
        return cbor_deserialize_array_type(buf, data_node);
    default:
        return 0;
    }
}

int cbor_deserialize_array_type(uint8_t *buf, const TsDataNode *data_node)
{
    uint16_t num_elements;
    int pos = 0; // Index of the next value in the buffer
    TsArrayInfo *array_info;
    array_info = (TsArrayInfo *)data_node->data;

    if (!array_info) {
        return 0;
    }

    // Deserialize the buffer length, and calculate the actual number of array elements
    pos = cbor_num_elements(buf, &num_elements);

    if (num_elements > array_info->max_elements) {
        return 0;
    }

    for (int i = 0; i < num_elements; i++) {
        switch (array_info->type) {
#if (TS_64BIT_TYPES_SUPPORT == 1)
        case TS_T_UINT64:
            pos += cbor_deserialize_uint64(&(buf[pos]), &(((uint64_t *)array_info->ptr)[i]));
            break;
        case TS_T_INT64:
            pos += cbor_deserialize_int64(&(buf[pos]), &(((int64_t *)array_info->ptr)[i]));
            break;
#endif
        case TS_T_UINT32:
            pos += cbor_deserialize_uint32(&(buf[pos]), &(((uint32_t *)array_info->ptr)[i]));
            break;
        case TS_T_INT32:
            pos += cbor_deserialize_int32(&(buf[pos]), &(((int32_t *)array_info->ptr)[i]));
            break;
        case TS_T_UINT16:
            pos += cbor_deserialize_uint16(&(buf[pos]), &(((uint16_t *)array_info->ptr)[i]));
            break;
        case TS_T_INT16:
            pos += cbor_deserialize_int16(&(buf[pos]), &(((int16_t *)array_info->ptr)[i]));
            break;
        case TS_T_FLOAT32:
            pos += cbor_deserialize_float(&(buf[pos]), &(((float *)array_info->ptr)[i]));
            break;
        default:
            break;
        }
    }
    return pos;
}

static int cbor_serialize_data_node(uint8_t *buf, size_t size, const TsDataNode *data_node)
{
    switch (data_node->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return cbor_serialize_uint(buf, *((uint64_t *)data_node->data), size);
    case TS_T_INT64:
        return cbor_serialize_int(buf, *((int64_t *)data_node->data), size);
#endif
    case TS_T_UINT32:
        return cbor_serialize_uint(buf, *((uint32_t *)data_node->data), size);
    case TS_T_INT32:
        return cbor_serialize_int(buf, *((int32_t *)data_node->data), size);
    case TS_T_UINT16:
        return cbor_serialize_uint(buf, *((uint16_t *)data_node->data), size);
    case TS_T_INT16:
        return cbor_serialize_int(buf, *((int16_t *)data_node->data), size);
    case TS_T_FLOAT32:
        if (data_node->detail == 0) { // round to 0 digits: use int
#ifdef TS_64BIT_TYPES_SUPPORT
            return cbor_serialize_int(buf, llroundf(*((float *)data_node->data)), size);
#else
            return cbor_serialize_int(buf, lroundf(*((float *)data_node->data)), size);
#endif
        }
        else {
            return cbor_serialize_float(buf, *((float *)data_node->data), size);
        }
    case TS_T_BOOL:
        return cbor_serialize_bool(buf, *((bool *)data_node->data), size);
    case TS_T_STRING:
        return cbor_serialize_string(buf, (char *)data_node->data, size);
    case TS_T_BYTES:
        return cbor_serialize_bytes(buf, ((TsBytesBuffer *)data_node->data)->bytes,
            ((TsBytesBuffer *)data_node->data)->num_bytes, size);
    case TS_T_ARRAY:
        return cbor_serialize_array_type(buf, size, data_node);
    default:
        return 0;
    }
}

int cbor_serialize_array_type(uint8_t *buf, size_t size, const TsDataNode *data_node)
{
    int pos = 0; // Index of the next value in the buffer
    TsArrayInfo *array_info;
    array_info = (TsArrayInfo *)data_node->data;

    if (!array_info) {
        return 0;
    }

    // Add the length field to the beginning of the CBOR buffer and update the CBOR buffer index
    pos = cbor_serialize_array(buf, array_info->num_elements, size);

    for (int i = 0; i < array_info->num_elements; i++) {
        switch (array_info->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
        case TS_T_UINT64:
            pos += cbor_serialize_uint(&(buf[pos]), ((uint64_t *)array_info->ptr)[i], size);
            break;
        case TS_T_INT64:
            pos += cbor_serialize_int(&(buf[pos]), ((int64_t *)array_info->ptr)[i], size);
            break;
#endif
        case TS_T_UINT32:
            pos += cbor_serialize_uint(&(buf[pos]), ((uint32_t *)array_info->ptr)[i], size);
            break;
        case TS_T_INT32:
            pos += cbor_serialize_int(&(buf[pos]), ((int32_t *)array_info->ptr)[i], size);
            break;
        case TS_T_UINT16:
            pos += cbor_serialize_uint(&(buf[pos]), ((uint16_t *)array_info->ptr)[i], size);
            break;
        case TS_T_INT16:
            pos += cbor_serialize_int(&(buf[pos]), ((int16_t *)array_info->ptr)[i], size);
            break;
        case TS_T_FLOAT32:
            if (data_node->detail == 0) { // round to 0 digits: use int
#ifdef TS_64BIT_TYPES_SUPPORT
                pos += cbor_serialize_int(&(buf[pos]),
                    llroundf(((float *)array_info->ptr)[i]), size);
#else
                pos += cbor_serialize_int(&(buf[pos]),
                    lroundf(((float *)array_info->ptr)[i]), size);
#endif
            }
            else {
                pos += cbor_serialize_float(&(buf[pos]), ((float *)array_info->ptr)[i], size);
            }
            break;
        default:
            break;
        }
    }
    return pos;
}

int ts_priv_bin_response(ts_object_t *ts, uint8_t code)
{
    if (ts->resp_size > 0) {
        ts->resp[0] = code;
        return 1;
    }
    else {
        return 0;
    }
}

int ts_priv_bin_process(ts_object_t *ts)
{
    int pos = 1;    // current position during data processing

    // get endpoint (first parameter of the request)
    const TsDataNode *endpoint = NULL;
    if ((ts->req[pos] & CBOR_TYPE_MASK) == CBOR_TEXT) {
        uint16_t path_len;
        pos += cbor_num_elements(&ts->req[pos], &path_len);
        endpoint = ts_get_node_by_path(ts, (char *)ts->req + pos, path_len);
    }
    else if ((ts->req[pos] & CBOR_TYPE_MASK) == CBOR_UINT) {
        ts_node_id_t id = 0;
        pos += cbor_deserialize_uint16(&ts->req[pos], &id);
        endpoint = ts_get_node_by_id(ts, id);
    }
    else if (ts->req[pos] == CBOR_UNDEFINED) {
        pos++;
    }
    else {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // process data
    if (ts->req[0] == TS_GET && endpoint) {
        return ts_priv_bin_get(ts, endpoint, ts->req[pos] == 0xA0, ts->req[pos] == 0xF7);
    }
    else if (ts->req[0] == TS_FETCH) {
        return ts_priv_bin_fetch(ts, endpoint, pos);
    }
    else if (ts->req[0] == TS_PATCH && endpoint) {
        int response = ts_priv_bin_patch(ts, endpoint, pos, ts->_auth_flags, 0);

        // check if endpoint has a callback assigned
        if (endpoint->data != NULL && ts->resp[0] == TS_STATUS_CHANGED) {
            // create function pointer and call function
            void (*fun)(void) = (void(*)(void))endpoint->data;
            fun();
        }
        return response;
    }
    else if (ts->req[0] == TS_POST) {
        return ts_priv_bin_exec(ts, endpoint, pos);
    }
    return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
}

int ts_priv_bin_fetch(ts_object_t *ts, const TsDataNode *parent, unsigned int pos_payload)
{
    /*
     * Remark: the parent node is currently still ignored. Any found data object is fetched.
     */

    unsigned int pos_req = pos_payload;
    unsigned int pos_resp = 0;
    uint16_t num_elements, element = 0;

    pos_resp += ts_priv_bin_response(ts, TS_STATUS_CONTENT);   // init response buffer

    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);
    if (num_elements != 1 && (ts->req[pos_payload] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    //printf("fetch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    ts->req[pos_req], ts->req[pos_req+1], ts->req[pos_req+2], ts->req[pos_req+3],
    //    ts->req[pos_req+4], ts->req[pos_req+5], ts->req[pos_req+6], ts->req[pos_req+7]);

    if (num_elements > 1) {
        pos_resp += cbor_serialize_array(&ts->resp[pos_resp], num_elements, ts->resp_size - pos_resp);
    }

    while (pos_req + 1 < ts->req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        ts_node_id_t id;
        num_bytes = cbor_deserialize_uint16(&ts->req[pos_req], &id);
        if (num_bytes == 0) {
            return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        const TsDataNode* data_node = ts_get_node_by_id(ts, id);
        if (data_node == NULL) {
            return ts_priv_bin_response(ts, TS_STATUS_NOT_FOUND);
        }
        if (!(data_node->access & TS_READ_MASK)) {
            return ts_priv_bin_response(ts, TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = cbor_serialize_data_node(&ts->resp[pos_resp], ts->resp_size - pos_resp, data_node);
        if (num_bytes == 0) {
            return ts_priv_bin_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
        }
        pos_resp += num_bytes;
        element++;
    }

    if (element == num_elements) {
        return pos_resp;
    }
    else {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
}

int ts_bin_sub(ts_object_t *ts, uint8_t *cbor_data, size_t len, uint16_t auth_flags,
               uint16_t sub_ch)
{
    uint8_t resp_tmp[1] = {};   // only one character as response expected
    ts->req = cbor_data;
    ts->req_len = len;
    ts->resp = resp_tmp;
    ts->resp_size = sizeof(resp_tmp);
    ts_priv_bin_patch(ts, NULL, 1, auth_flags, sub_ch);
    return ts->resp[0];
}

int ts_priv_bin_patch(ts_object_t *ts, const TsDataNode *parent, unsigned int pos_payload,
                      uint16_t auth_flags, uint16_t sub_ch)
{
    unsigned int pos_req = pos_payload;
    uint16_t num_elements, element = 0;

    if ((ts->req[pos_req] & CBOR_TYPE_MASK) != CBOR_MAP) {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);

    //printf("patch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    ts->req[pos_req], ts->req[pos_req+1], ts->req[pos_req+2], ts->req[pos_req+3],
    //    ts->req[pos_req+4], ts->req[pos_req+5], ts->req[pos_req+6], ts->req[pos_req+7]);

    while (pos_req < ts->req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        ts_node_id_t id;
        num_bytes = cbor_deserialize_uint16(&ts->req[pos_req], &id);
        if (num_bytes == 0) {
            return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        const TsDataNode* node = ts_get_node_by_id(ts, id);
        if (node) {
            if ((node->access & TS_WRITE_MASK & auth_flags) == 0) {
                if (node->access & TS_WRITE_MASK) {
                    return ts_priv_bin_response(ts, TS_STATUS_UNAUTHORIZED);
                }
                else {
                    return ts_priv_bin_response(ts, TS_STATUS_FORBIDDEN);
                }
            }
            else if (parent && node->parent != parent->id) {
                return ts_priv_bin_response(ts, TS_STATUS_NOT_FOUND);
            }
            else if (sub_ch && !(node->pubsub & sub_ch)) {
                // ignore element
                num_bytes = cbor_size(&ts->req[pos_req]);
            }
            else {
                // actually deserialize the data and update node
                num_bytes = cbor_deserialize_data_node(&ts->req[pos_req], node);
            }
        }
        else {
            // node not found
            if (sub_ch) {
                // ignore element
                num_bytes = cbor_size(&ts->req[pos_req]);
            }
            else {
                return ts_priv_bin_response(ts, TS_STATUS_NOT_FOUND);
            }
        }

        if (num_bytes == 0) {
            return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        element++;
    }

    if (element == num_elements) {
        return ts_priv_bin_response(ts, TS_STATUS_CHANGED);
    } else {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
}

int ts_priv_bin_exec(ts_object_t *ts, const TsDataNode *node, unsigned int pos_payload)
{
    unsigned int pos_req = pos_payload;
    uint16_t num_elements, element = 0;

    if ((ts->req[pos_req] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }
    pos_req += cbor_num_elements(&ts->req[pos_req], &num_elements);

    if ((node->access & TS_WRITE_MASK) && (node->type == TS_T_EXEC)) {
        // node is generally executable, but are we authorized?
        if ((node->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            return ts_priv_bin_response(ts, TS_STATUS_UNAUTHORIZED);
        }
    }
    else {
        return ts_priv_bin_response(ts, TS_STATUS_FORBIDDEN);
    }

    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].parent == node->id) {
            if (element >= num_elements) {
                // more child nodes found than parameters were passed
                return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
            }
            int num_bytes = cbor_deserialize_data_node(&ts->req[pos_req], &ts->data_nodes[i]);
            if (num_bytes == 0) {
                // deserializing the value was not successful
                return ts_priv_bin_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
            }
            pos_req += num_bytes;
            element++;
        }
    }

    if (num_elements > element) {
        // more parameters passed than child nodes found
        return ts_priv_bin_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // if we got here, finally create function pointer and call function
    void (*fun)(void) = (void(*)(void))node->data;
    fun();

    return ts_priv_bin_response(ts, TS_STATUS_VALID);
}

int ts_bin_pub(ts_object_t *ts, uint8_t *buf, size_t buf_size, const uint16_t pub_ch)
{
    buf[0] = TS_PUBMSG;
    int len = 1;

    // find out number of elements to be published
    int num_ids = 0;
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].pubsub & pub_ch) {
            num_ids++;
        }
    }

    len += cbor_serialize_map(&buf[len], num_ids, buf_size - len);

    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].pubsub & pub_ch) {
            len += cbor_serialize_uint(&buf[len], ts->data_nodes[i].id, buf_size - len);
            size_t num_bytes = cbor_serialize_data_node(&buf[len], buf_size - len, &ts->data_nodes[i]);
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

int ts_bin_pub_can(ts_object_t *ts, int *start_pos, uint16_t pub_ch, uint8_t can_dev_id,
                   uint32_t *msg_id, uint8_t *msg_data)
{
    int msg_len = -1;

    for (unsigned int i = *start_pos; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].pubsub & pub_ch) {
            *msg_id = TS_CAN_BASE_PUBSUB | TS_CAN_PRIO_PUBSUB_LOW
                | TS_CAN_DATA_ID_SET(ts->data_nodes[i].id)
                | TS_CAN_SOURCE_SET(can_dev_id);

            msg_len = cbor_serialize_data_node(msg_data, 8, &ts->data_nodes[i]);

            if (msg_len > 0) {
                // node found and successfully encoded, increase start pos for next run
                *start_pos = i + 1;
                break;
            }
            // else: data too long, take next node
        }
    }

    if (msg_len <= 0) {
        // no more nodes found, reset position
        *start_pos = 0;
    }

    return msg_len;
}

/*
int ThingSet::name_cbor(void)
{
    ts->resp[0] = TS_OBJ_NAME + 0x80;    // Function ID
    int data_node_id = _req[1] + ((int)_req[2] << 8);

    for (unsigned int i = 0; i < sizeof(ts->data_nodes)/sizeof(DataNode); i++) {
        if (ts->data_nodes[i].id == data_node_id) {
            if (ts->data_nodes[i].access & ACCESS_READ) {
                ts->resp[1] = T_STRING;
                int len = strlen(ts->data_nodes[i].name);
                for (int j = 0; j < len; j++) {
                    ts->resp[j+2] = *(ts->data_nodes[i].name + j);
                }
                #if DEBUG
                serial.printf("Get Data Object Name: %s (id = %d)\n", ts->data_nodes[i].name, data_node_id);
                #endif
                return len + 2;
            }
            else {
                ts->resp[1] = TS_STATUS_UNAUTHORIZED;
                return 2;   // length of response
            }
        }
    }

    // data node not found --> send error message
    ts->resp[1] = TS_STATUS_DATA_UNKNOWN;
    return 2;   // length of response
}
*/

int ts_priv_bin_get(ts_object_t *ts, const TsDataNode *parent, bool values, bool ids_only)
{
    unsigned int len = 0;       // current length of response
    len += ts_priv_bin_response(ts, TS_STATUS_CONTENT);   // init response buffer

    // find out number of elements
    int num_elements = 0;
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].access & TS_READ_MASK
            && (ts->data_nodes[i].parent == parent->id))
        {
            num_elements++;
        }
    }

    if (values && !ids_only) {
        len += cbor_serialize_map(&ts->resp[len], num_elements, ts->resp_size - len);
    }
    else {
        len += cbor_serialize_array(&ts->resp[len], num_elements, ts->resp_size - len);
    }

    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].access & TS_READ_MASK
            && (ts->data_nodes[i].parent == parent->id))
        {
            int num_bytes = 0;
            if (ids_only) {
                num_bytes = cbor_serialize_uint(&ts->resp[len], ts->data_nodes[i].id, ts->resp_size - len);
            }
            else {
                num_bytes = cbor_serialize_string(&ts->resp[len], ts->data_nodes[i].name,
                    ts->resp_size - len);
                if (values) {
                    num_bytes += cbor_serialize_data_node(&ts->resp[len + num_bytes],
                        ts->resp_size - len - num_bytes, &ts->data_nodes[i]);
                }
            }

            if (num_bytes == 0) {
                return ts_priv_bin_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
            } else {
                len += num_bytes;
            }
        }
    }

    return len;
}
