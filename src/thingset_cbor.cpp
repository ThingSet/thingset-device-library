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

int ThingSet::read_cbor(int category)
{
    unsigned int pos = 1;       // position in request (ignore first byte for function code)
    unsigned int len = 0;       // current length of response
    uint16_t num_elements, element = 0;

    len += status_message_cbor(TS_STATUS_CONTENT);   // init response buffer

    pos += cbor_num_elements(&req[1], &num_elements);
    if (num_elements != 1 && (req[1] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    //printf("read request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    req[pos], req[pos+1], req[pos+2], req[pos+3],
    //    req[pos+4], req[pos+5], req[pos+6], req[pos+7]);

    if (num_elements > 1) {
        len += cbor_serialize_array(&resp[len], num_elements, resp_size - len);
    }

    while (pos + 1 < req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        uint16_t id;
        num_bytes = cbor_deserialize_uint16(&req[pos], &id);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos += num_bytes;

        const DataNode* data_node = get_data_node(id);
        if (data_node == NULL) {
            return status_message_cbor(TS_STATUS_NOT_FOUND);
        }
        if (!(data_node->access & TS_READ_ALL)) {
            return status_message_cbor(TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = cbor_serialize_data_node(&resp[len], resp_size - len, data_node);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_RESPONSE_TOO_LARGE);
        } else {
            len += num_bytes;
        }
        element++;
    }

    if (element == num_elements) {
        return len;
    } else {
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
    write_cbor(0, true);
    return resp[0] - 0x80;
}

int ThingSet::write_cbor(int category, bool ignore_access)
{
    unsigned int pos = 1;       // ignore first byte for function code in request
    uint16_t num_elements, element = 0;

    pos += cbor_num_elements(&req[1], &num_elements);
    if ((req[1] & CBOR_TYPE_MASK) != CBOR_MAP) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    //printf("write request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements,
    //    req[pos], req[pos+1], req[pos+2], req[pos+3],
    //    req[pos+4], req[pos+5], req[pos+6], req[pos+7]);

    // loop through all elements to check if request is valid
    while (pos < req_len && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        uint16_t id;
        num_bytes = cbor_deserialize_uint16(&req[pos], &id);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos += num_bytes;

        const DataNode* data_node = get_data_node(id);
        if (data_node == NULL) {
            if (!ignore_access) {
                return status_message_cbor(TS_STATUS_NOT_FOUND);
            }

            // ignore element
            num_bytes = cbor_size(&req[pos]);
        }
        else {
            if (!ignore_access) { // access ignored if direcly called (e.g. to write data from EEPROM)
                if (!(data_node->access & TS_WRITE_ALL)) {
                    return status_message_cbor(TS_STATUS_UNAUTHORIZED);
                }
                if (data_node->parent != category) {
                    return status_message_cbor(TS_STATUS_NOT_FOUND);
                }
            }

            num_bytes = cbor_deserialize_data_node(&req[pos], data_node);
        }

        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_BAD_REQUEST);
        }
        pos += num_bytes;

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
    uint16_t id;
    size_t num_bytes = cbor_deserialize_uint16(&req[1], &id);
    if (num_bytes == 0 || req_len > 4) {
        return status_message_cbor(TS_STATUS_BAD_REQUEST);
    }

    const DataNode* data_node = get_data_node(id);
    if (data_node == NULL) {
        return status_message_cbor(TS_STATUS_NOT_FOUND);
    }
    if (!(data_node->access & TS_EXEC_ALL)) {
        return status_message_cbor(TS_STATUS_UNAUTHORIZED);
    }

    // create function pointer and call function
    void (*fun)(void) = reinterpret_cast<void(*)()>(data_node->data);
    fun();

    return status_message_cbor(TS_STATUS_VALID);
}

int ThingSet::pub_msg_cbor(uint8_t *msg_buf, size_t size, unsigned int channel)
{
    if (channel >= num_channels) {
        return 0;      // unknown channel
    }

    return pub_msg_cbor(msg_buf, size, pub_channels[channel].object_ids, pub_channels[channel].num);
}

int ThingSet::pub_msg_cbor(uint8_t *msg_buf, size_t size, const uint16_t pub_list[],
    size_t num_elements)
{
    msg_buf[0] = TS_PUBMSG;
    int len = 1;

    if (num_elements > 1) {
        len += cbor_serialize_map(&msg_buf[len], num_elements, size - len);
    }

    for (unsigned int element = 0; element < num_elements; element++) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        const DataNode* data_node = get_data_node(pub_list[element]);
        if (data_node == NULL || !(data_node->access & TS_READ_ALL)) {
            continue;
        }

        len += cbor_serialize_uint(&msg_buf[len], data_node->id, size - len);
        num_bytes += cbor_serialize_data_node(&msg_buf[len], size - len, data_node);

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

int ThingSet::list_cbor(int category, bool values, bool ids_only)
{
    unsigned int len = 0;       // current length of response
    len += status_message_cbor(TS_STATUS_CONTENT);   // init response buffer

    // find out number of elements
    int num_elements = 0;
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].access & TS_READ_ALL
            && (data_nodes[i].parent == category))
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

    // actually write elements
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].access & TS_READ_ALL
            && (data_nodes[i].parent == category))
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
