/* ThingSet protocol client library
 * Copyright (c) 2017-2019 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ts_config.h"
#include "thingset.h"
#include "cbor.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>  // for definition of endianness
#include <math.h>       // for rounding of floats

int cbor_deserialize_array_type(uint8_t *buf, const data_object_t *data_obj);
int cbor_serialize_array_type(uint8_t *buf, size_t size, const data_object_t *data_obj);

static int cbor_deserialize_data_object(uint8_t *buf, const data_object_t *data_obj)
{
    switch (data_obj->type) {
#if (TS_64BIT_TYPES_SUPPORT == 1)
    case TS_T_UINT64:
        return cbor_deserialize_uint64(buf, (uint64_t *)data_obj->data);
    case TS_T_INT64:
        return cbor_deserialize_int64(buf, (int64_t *)data_obj->data);
#endif
    case TS_T_UINT32:
        return cbor_deserialize_uint32(buf, (uint32_t *)data_obj->data);
    case TS_T_INT32:
        return cbor_deserialize_int32(buf, (int32_t *)data_obj->data);
    case TS_T_UINT16:
        return cbor_deserialize_uint16(buf, (uint16_t *)data_obj->data);
    case TS_T_INT16:
        return cbor_deserialize_int16(buf, (int16_t *)data_obj->data);
    case TS_T_FLOAT32:
        return cbor_deserialize_float(buf, (float *)data_obj->data);
    case TS_T_BOOL:
        return cbor_deserialize_bool(buf, (bool *)data_obj->data);
    case TS_T_STRING:
        return cbor_deserialize_string(buf, (char *)data_obj->data, data_obj->detail);
    case TS_T_ARRAY: 
        return cbor_deserialize_array_type(buf, data_obj);
    default:
        return 0;
    }
}

int cbor_deserialize_array_type(uint8_t *buf, const data_object_t *data_obj)
{
    int num_elements;
    int pos = 0; // Index of the next value in the buffer
    ArrayInfo *array_info;
    array_info = (ArrayInfo *)data_obj->data;
    // If the data object is not defined
    if (!array_info) {
        return 0;
    }

    // Deserialize the buffer length, and calculate the actual number of array elements
    pos = cbor_deserialize_array(buf, array_info->max_elements, &num_elements);

    // If the array length is invalid
    if (pos == 0) {
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
        } // switch (data_array->type)
    } // end of 'for' loop
    return pos;
}

static int cbor_serialize_data_object(uint8_t *buf, size_t size, const data_object_t *data_obj)
{
    switch (data_obj->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return cbor_serialize_uint(buf, *((uint64_t *)data_obj->data), size);
    case TS_T_INT64:
        return cbor_serialize_int(buf, *((int64_t *)data_obj->data), size);
#endif
    case TS_T_UINT32:
        return cbor_serialize_uint(buf, *((uint32_t *)data_obj->data), size);
    case TS_T_INT32:
        return cbor_serialize_int(buf, *((int32_t *)data_obj->data), size);
    case TS_T_UINT16:
        return cbor_serialize_uint(buf, *((uint16_t *)data_obj->data), size);
    case TS_T_INT16:
        return cbor_serialize_int(buf, *((int16_t *)data_obj->data), size);
    case TS_T_FLOAT32:
        if (data_obj->detail == 0) { // round to 0 digits: use int
#ifdef TS_64BIT_TYPES_SUPPORT
            return cbor_serialize_int(buf, llroundf(*((float *)data_obj->data)), size);
#else
            return cbor_serialize_int(buf, lroundf(*((float *)data_obj->data)), size);
#endif
        }
        else {
            return cbor_serialize_float(buf, *((float *)data_obj->data), size);
        }
    case TS_T_BOOL:
        return cbor_serialize_bool(buf, *((bool *)data_obj->data), size);
    case TS_T_STRING:
        return cbor_serialize_string(buf, (char *)data_obj->data, size);
    case TS_T_ARRAY: 
        return cbor_serialize_array_type(buf, size, data_obj);
    default:
        return 0;
    } // switch (data_obj->type)
}

int cbor_serialize_array_type(uint8_t *buf, size_t size, const data_object_t *data_obj)
{
    int pos = 0; // Index of the next value in the buffer
    ArrayInfo *array_info;
    array_info = (ArrayInfo *)data_obj->data;
    // If the data object is not defined
    if (!array_info) {
        return 0;
    }

    // Add the length fleld to the beginning of the CBOR buffer and update the CBOR buffer index
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
            if (data_obj->detail == 0) { // round to 0 digits: use int
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
        } // switch (data_array->type)
    } // end of 'for' loop

    return pos; // Returns the final length of the CBOR buffer
}


int ThingSet::status_message_cbor(uint8_t code)
{
    if (resp_size > 0) {
        resp[0] = 0x80 + code;
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

    len += status_message_cbor(TS_STATUS_SUCCESS);   // init response buffer

    pos += cbor_num_elements(&req[1], &num_elements);
    if (num_elements != 1 && (req[1] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return status_message_cbor(TS_STATUS_WRONG_FORMAT);
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
            return status_message_cbor(TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        const data_object_t* data_obj = get_data_object(id);
        if (data_obj == NULL) {
            return status_message_cbor(TS_STATUS_UNKNOWN_DATA_OBJ);
        }
        if (!(data_obj->access & TS_READ_ALL)) {
            return status_message_cbor(TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = cbor_serialize_data_object(&resp[len], resp_size - len, data_obj);
        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_RESPONSE_TOO_LONG);
        } else {
            len += num_bytes;
        }
        element++;
    }

    if (element == num_elements) {
        return len;
    } else {
        return status_message_cbor(TS_STATUS_WRONG_FORMAT);
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
        return status_message_cbor(TS_STATUS_WRONG_FORMAT);
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
            return status_message_cbor(TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        const data_object_t* data_obj = get_data_object(id);
        if (data_obj == NULL) {
            if (!ignore_access) {
                return status_message_cbor(TS_STATUS_UNKNOWN_DATA_OBJ);
            }

            // ignore element
            num_bytes = cbor_size(&req[pos]);
        }
        else {
            if (!ignore_access) { // access ignored if direcly called (e.g. to write data from EEPROM)
                if (!(data_obj->access & TS_WRITE_ALL)) {
                    return status_message_cbor(TS_STATUS_UNAUTHORIZED);
                }
                if (data_obj->category != category) {
                    return status_message_cbor(TS_STATUS_WRONG_CATEGORY);
                }
            }

            num_bytes = cbor_deserialize_data_object(&req[pos], data_obj);
        }

        if (num_bytes == 0) {
            return status_message_cbor(TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        element++;
    }

    if (element == num_elements) {
        return status_message_cbor(TS_STATUS_SUCCESS);
    } else {
        return status_message_cbor(TS_STATUS_WRONG_FORMAT);
    }
}

int ThingSet::exec_cbor()
{
    // only a single function call allowed (no array of data objects)
    uint16_t id;
    size_t num_bytes = cbor_deserialize_uint16(&req[1], &id);
    if (num_bytes == 0 || req_len > 4) {
        return status_message_cbor(TS_STATUS_WRONG_FORMAT);
    }

    const data_object_t* data_obj = get_data_object(id);
    if (data_obj == NULL) {
        return status_message_cbor(TS_STATUS_UNKNOWN_DATA_OBJ);
    }
    if (!(data_obj->access & TS_EXEC_ALL)) {
        return status_message_cbor(TS_STATUS_UNAUTHORIZED);
    }

    // create function pointer and call function
    void (*fun)(void) = reinterpret_cast<void(*)()>(data_obj->data);
    fun();

    return status_message_cbor(TS_STATUS_SUCCESS);
}

int ThingSet::pub_msg_cbor(uint8_t *msg_buf, size_t size, unsigned int channel)
{
    if (channel >= num_channels) {
        return 0;      // unknown channel
    }

    return pub_msg_cbor(msg_buf, size, pub_channels[channel].object_ids, pub_channels[channel].num);
}

int ThingSet::pub_msg_cbor(uint8_t *msg_buf, size_t size, const uint16_t pub_list[], size_t num_elements)
{
    msg_buf[0] = TS_PUBMSG;
    int len = 1;

    if (num_elements > 1) {
        len += cbor_serialize_map(&msg_buf[len], num_elements, size - len);
    }

    for (unsigned int element = 0; element < num_elements; element++) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        const data_object_t* data_obj = get_data_object(pub_list[element]);
        if (data_obj == NULL || !(data_obj->access & TS_READ_ALL)) {
            continue;
        }

        len += cbor_serialize_uint(&msg_buf[len], data_obj->id, size - len);
        num_bytes += cbor_serialize_data_object(&msg_buf[len], size - len, data_obj);

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
    int data_obj_id = _req[1] + ((int)_req[2] << 8);

    for (unsigned int i = 0; i < sizeof(dataObjects)/sizeof(data_object_t); i++) {
        if (dataObjects[i].id == data_obj_id) {
            if (dataObjects[i].access & ACCESS_READ) {
                resp[1] = T_STRING;
                int len = strlen(dataObjects[i].name);
                for (int j = 0; j < len; j++) {
                    resp[j+2] = *(dataObjects[i].name + j);
                }
                #if DEBUG
                serial.printf("Get Data Object Name: %s (id = %d)\n", dataObjects[i].name, data_obj_id);
                #endif
                return len + 2;
            }
            else {
                resp[1] = TS_STATUS_UNAUTHORIZED;
                return 2;   // length of response
            }
        }
    }

    // data object not found --> send error message
    resp[1] = TS_STATUS_DATA_UNKNOWN;
    return 2;   // length of response
}
*/

int ThingSet::list_cbor(int category, bool values, bool ids_only)
{
    unsigned int len = 0;       // current length of response
    len += status_message_cbor(TS_STATUS_SUCCESS);   // init response buffer

    // find out number of elements
    int num_elements = 0;
    for (unsigned int i = 0; i < num_objects; i++) {
        if (data_objects[i].access & TS_READ_ALL
            && (data_objects[i].category == category))
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
    for (unsigned int i = 0; i < num_objects; i++) {
        if (data_objects[i].access & TS_READ_ALL
            && (data_objects[i].category == category))
        {
            int num_bytes = 0;
            if (ids_only) {
                num_bytes = cbor_serialize_uint(&resp[len], data_objects[i].id, resp_size - len);
            }
            else {
                num_bytes = cbor_serialize_string(&resp[len], data_objects[i].name, resp_size - len);
                if (values) {
                    num_bytes += cbor_serialize_data_object(&resp[len + num_bytes], resp_size - len - num_bytes, &data_objects[i]);
                }
            }

            if (num_bytes == 0) {
                return status_message_cbor(TS_STATUS_RESPONSE_TOO_LONG);
            } else {
                len += num_bytes;
            }
        }
    }

    return len;
}
