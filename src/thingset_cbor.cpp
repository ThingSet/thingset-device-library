/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "ts_config.h"
#include "thingset.h"
#include "cbor.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>  // for definition of endianness
#include <math.h>       // for rounding of floats

int cbor_deserialize_array_type(uint8_t *buf, const DataNode *data_node);
int cbor_serialize_array_type(uint8_t *buf, size_t size, const DataNode *data_node);

static int cbor_deserialize_data_node(uint8_t *buf, const DataNode *data_node)
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
    case TS_T_ARRAY:
        return cbor_deserialize_array_type(buf, data_node);
    default:
        return 0;
    }
}

int cbor_deserialize_array_type(uint8_t *buf, const DataNode *data_node)
{
    uint16_t num_elements;
    int pos = 0; // Index of the next value in the buffer
    ArrayInfo *array_info;
    array_info = (ArrayInfo *)data_node->data;

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

static int cbor_serialize_data_node(uint8_t *buf, size_t size, const DataNode *data_node)
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
    case TS_T_ARRAY:
        return cbor_serialize_array_type(buf, size, data_node);
    default:
        return 0;
    }
}

int cbor_serialize_array_type(uint8_t *buf, size_t size, const DataNode *data_node)
{
    int pos = 0; // Index of the next value in the buffer
    ArrayInfo *array_info;
    array_info = (ArrayInfo *)data_node->data;

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

int ThingSet::status_message_cbor(uint8_t code)
{
    if (resp_size > 0) {
        resp[0] = code;
        return 1;
    }
    else {
        return 0;
    }
}

int ThingSet::process_cbor()
{
    int pos = 1;    // current position during data processing

    // get endpoint (first parameter of the request)
    const DataNode *endpoint = NULL;
    if ((req[pos] & CBOR_TYPE_MASK) == CBOR_TEXT) {
        uint16_t path_len;
        pos += cbor_num_elements(&req[pos], &path_len);
        endpoint = get_endpoint_node((char *)req + pos, path_len);
    }
    else if ((req[pos] & CBOR_TYPE_MASK) == CBOR_UINT) {
        node_id_t id = 0;
        pos += cbor_deserialize_uint16(&req[pos], &id);
        endpoint = get_data_node(id);
    }
    else if (req[pos] == CBOR_UNDEFINED) {
        pos++;
    }
    else {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    // process data
    if (req[0] == TS_GET && endpoint) {
        return get_cbor(endpoint, req[pos] == 0xA0, req[pos] == 0xF7);
    }
    else if (req[0] == TS_FETCH) {
        return fetch_cbor(endpoint, pos);
    }
    else if (req[0] == TS_PATCH && endpoint) {
        return patch_cbor(endpoint, pos, false);

        // check if endpoint has a callback assigned
        if (endpoint->data != NULL && resp[0] == TS_STATUS_CHANGED) {
            // create function pointer and call function
            void (*fun)(void) = reinterpret_cast<void(*)()>(endpoint->data);
            fun();
        }

    }
    else if (req[0] == TS_EXEC) {
        return exec_cbor();
    }
    return status_message_cbor(TS_STATUS_BAD_REQUEST);
}

int ThingSet::fetch_cbor(const DataNode *parent, unsigned int pos_payload)
{
    /*
     * Remark: the parent node is currently still ignored. Any found data object is fetched.
     */

    unsigned int pos_req = pos_payload;
    unsigned int pos_resp = 0;
    uint16_t num_elements, element = 0;

    pos_resp += status_message_cbor(TS_STATUS_CONTENT);   // init response buffer

    pos_req += cbor_num_elements(&req[pos_req], &num_elements);
    if (num_elements != 1 && (req[pos_payload] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    //printf("fetch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    req[pos_req], req[pos_req+1], req[pos_req+2], req[pos_req+3],
    //    req[pos_req+4], req[pos_req+5], req[pos_req+6], req[pos_req+7]);

    if (num_elements > 1) {
        pos_resp += cbor_serialize_array(&resp[pos_resp], num_elements, resp_size - pos_resp);
    }

    while (pos_req + 1 < req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        node_id_t id;
        num_bytes = cbor_deserialize_uint16(&req[pos_req], &id);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        const DataNode* data_node = get_data_node(id);
        if (data_node == NULL) {
            return status_message_cbor(TS_STATUS_NOT_FOUND);
        }
        if (!(data_node->access & TS_READ_MASK)) {
            return status_message_cbor(TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = cbor_serialize_data_node(&resp[pos_resp], resp_size - pos_resp, data_node);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_RESPONSE_TOO_LARGE);
        }
        pos_resp += num_bytes;
        element++;
    }

    if (element == num_elements) {
        return pos_resp;
    }
    else {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }
}

int ThingSet::init_cbor(uint8_t *cbor_data, size_t len)
{
    uint8_t resp_tmp[1] = {};   // only one character as response expected
    req = cbor_data;
    req_len = len;
    resp = resp_tmp;
    resp_size = sizeof(resp_tmp);
    //patch_cbor(0, true);                  // TODO!!
    return resp[0] - 0x80;
}

int ThingSet::patch_cbor(const DataNode *parent, unsigned int pos_payload, bool ignore_access)
{
    unsigned int pos_req = pos_payload;
    uint16_t num_elements, element = 0;

    if ((req[pos_req] & CBOR_TYPE_MASK) != CBOR_MAP) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }
    pos_req += cbor_num_elements(&req[pos_req], &num_elements);

    //printf("patch request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    req[pos_req], req[pos_req+1], req[pos_req+2], req[pos_req+3],
    //    req[pos_req+4], req[pos_req+5], req[pos_req+6], req[pos_req+7]);

    // loop through all elements to check if request is valid
    while (pos_req < req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        node_id_t id;
        num_bytes = cbor_deserialize_uint16(&req[pos_req], &id);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        const DataNode* data_node = get_data_node(id);
        if (data_node == NULL) {
            if (!ignore_access) {
                return status_message_cbor(TS_STATUS_NOT_FOUND);
            }

            // ignore element
            num_bytes = cbor_size(&req[pos_req]);
        }
        else {
            if (!ignore_access) { // access ignored if direcly called (e.g. to write data from EEPROM)
                if (!(data_node->access & TS_WRITE_MASK)) {
                    return status_message_cbor(TS_STATUS_UNAUTHORIZED);
                }
                if (data_node->parent != parent->id) {
                    return status_message_cbor(TS_STATUS_NOT_FOUND);
                }
            }

            num_bytes = cbor_deserialize_data_node(&req[pos_req], data_node);
        }

        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos_req += num_bytes;

        element++;
    }

    if (element == num_elements) {
        return status_message_cbor(TS_STATUS_CHANGED);
    } else {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }
}

int ThingSet::exec_cbor()
{
    // only a single function call allowed (no array of data nodes)
    node_id_t id;
    size_t num_bytes = cbor_deserialize_uint16(&req[1], &id);
    if (num_bytes == 0 || req_len > 4) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    const DataNode* data_node = get_data_node(id);
    if (data_node == NULL) {
        return status_message_cbor(TS_STATUS_NOT_FOUND);
    }
    if (!(data_node->access & TS_WRITE_MASK)) {
        return status_message_cbor(TS_STATUS_FORBIDDEN);
    }

    // create function pointer and call function
    void (*fun)(void) = reinterpret_cast<void(*)()>(data_node->data);
    fun();

    return status_message_cbor(TS_STATUS_VALID);
}

int ThingSet::pub_msg_cbor(uint8_t *buf, size_t buf_size, const node_id_t node_ids[],
    size_t num_ids)
{
    buf[0] = TS_PUBMSG;
    int len = 1;

    if (num_ids > 1) {
        len += cbor_serialize_map(&buf[len], num_ids, buf_size - len);
    }

    for (unsigned int i = 0; i < num_ids; i++) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        const DataNode* data_node = get_data_node(node_ids[i]);
        if (data_node == NULL || !(data_node->access & TS_READ_MASK)) {
            continue;
        }

        len += cbor_serialize_uint(&buf[len], data_node->id, buf_size - len);
        num_bytes += cbor_serialize_data_node(&buf[len], buf_size - len, data_node);

        if (num_bytes == 0) {
            return 0;
        } else {
            len += num_bytes;
        }
    }
    return len;
}

/*
int ThingSet::name_cbor(void)
{
    resp[0] = TS_OBJ_NAME + 0x80;    // Function ID
    int data_node_id = _req[1] + ((int)_req[2] << 8);

    for (unsigned int i = 0; i < sizeof(data_nodes)/sizeof(DataNode); i++) {
        if (data_nodes[i].id == data_node_id) {
            if (data_nodes[i].access & ACCESS_READ) {
                resp[1] = T_STRING;
                int len = strlen(data_nodes[i].name);
                for (int j = 0; j < len; j++) {
                    resp[j+2] = *(data_nodes[i].name + j);
                }
                #if DEBUG
                serial.printf("Get Data Object Name: %s (id = %d)\n", data_nodes[i].name, data_node_id);
                #endif
                return len + 2;
            }
            else {
                resp[1] = TS_STATUS_UNAUTHORIZED;
                return 2;   // length of response
            }
        }
    }

    // data node not found --> send error message
    resp[1] = TS_STATUS_DATA_UNKNOWN;
    return 2;   // length of response
}
*/

int ThingSet::get_cbor(const DataNode *parent, bool values, bool ids_only)
{
    unsigned int len = 0;       // current length of response
    len += status_message_cbor(TS_STATUS_CONTENT);   // init response buffer

    // find out number of elements
    int num_elements = 0;
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].access & TS_READ_MASK
            && (data_nodes[i].parent == parent->id))
        {
            num_elements++;
        }
    }

    if (values && !ids_only) {
        len += cbor_serialize_map(&resp[len], num_elements, resp_size - len);
    }
    else {
        len += cbor_serialize_array(&resp[len], num_elements, resp_size - len);
    }

    for (unsigned int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].access & TS_READ_MASK
            && (data_nodes[i].parent == parent->id))
        {
            int num_bytes = 0;
            if (ids_only) {
                num_bytes = cbor_serialize_uint(&resp[len], data_nodes[i].id, resp_size - len);
            }
            else {
                num_bytes = cbor_serialize_string(&resp[len], data_nodes[i].name,
                    resp_size - len);
                if (values) {
                    num_bytes += cbor_serialize_data_node(&resp[len + num_bytes],
                        resp_size - len - num_bytes, &data_nodes[i]);
                }
            }

            if (num_bytes == 0) {
                return status_message_cbor(TS_STATUS_RESPONSE_TOO_LARGE);
            } else {
                len += num_bytes;
            }
        }
    }

    return len;
}
