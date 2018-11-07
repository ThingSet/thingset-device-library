/* ThingSet protocol library
 * Copyright (c) 2017-2018 Martin Jäger (www.libre.solar)
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

const data_object_t* thingset_data_object_by_id(ts_data_t *data, uint16_t id) {
    for (unsigned int i = 0; i < data->size; i++) {
        if (data->objects[i].id == id) {
            return &(data->objects[i]);
        }
    }
    return NULL;
}

int _status_msg(ts_buffer_t *resp, uint8_t code)
{
    resp->data.bin[0] = 0x80 + code;
    resp->pos = 1;
    return code;
}

int _deserialize_data_object(uint8_t *buf, const data_object_t* data_obj)
{
    switch (data_obj->type) {
#if (TS_64BIT_TYPES_SUPPORT == 1)
    case TS_T_UINT64:
        return cbor_deserialize_uint64(buf, (uint64_t*)data_obj->data);
        break;
    case TS_T_INT64:
        return cbor_deserialize_int64(buf, (int64_t*)data_obj->data);
        break;
#endif
    case TS_T_UINT32:
        return cbor_deserialize_uint32(buf, (uint32_t*)data_obj->data);
        break;
    case TS_T_INT32:
        return cbor_deserialize_int32(buf, (int32_t*)data_obj->data);
        break;
    case TS_T_UINT16:
        return cbor_deserialize_uint16(buf, (uint16_t*)data_obj->data);
        break;
    case TS_T_INT16:
        return cbor_deserialize_int16(buf, (int16_t*)data_obj->data);
        break;
    case TS_T_FLOAT32:
        return cbor_deserialize_float(buf, (float*)data_obj->data);
        break;
    case TS_T_BOOL:
        return cbor_deserialize_bool(buf, (bool*)data_obj->data);
        break;
    case TS_T_STRING:
        return cbor_deserialize_string(buf, (char*)data_obj->data, data_obj->detail);
        break;
    default:
        return 0;
        break;
    }
}

int _cbor_serialize_data_object(ts_buffer_t *buf, const data_object_t* data_obj)
{
    switch (data_obj->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return cbor_serialize_uint(&buf->data.bin[buf->pos], *((uint64_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_INT64:
        return cbor_serialize_int(&buf->data.bin[buf->pos], *((int64_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
#endif
    case TS_T_UINT32:
        return cbor_serialize_uint(&buf->data.bin[buf->pos], *((uint32_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_INT32:
        return cbor_serialize_int(&buf->data.bin[buf->pos], *((int32_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_UINT16:
        return cbor_serialize_uint(&buf->data.bin[buf->pos], *((uint16_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_INT16:
        return cbor_serialize_int(&buf->data.bin[buf->pos], *((int16_t*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_FLOAT32:
        return cbor_serialize_float(&buf->data.bin[buf->pos], *((float*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_BOOL:
        return cbor_serialize_bool(&buf->data.bin[buf->pos], *((bool*)data_obj->data), TS_RESP_BUFFER_LEN - buf->pos);
    case TS_T_STRING:
        return cbor_serialize_string(&buf->data.bin[buf->pos], (char*)data_obj->data, TS_RESP_BUFFER_LEN - buf->pos);
    default:
        return 0;
    }
}

int thingset_read_cbor(ts_buffer_t *req, ts_buffer_t *resp, ts_data_t *data)
{
    unsigned int pos = 1;       // ignore first byte for function code in request
    uint16_t num_elements, element = 0;
    
    _status_msg(resp, TS_STATUS_SUCCESS);   // init response buffer

    pos += cbor_num_elements(&req->data.bin[1], &num_elements);
    if (num_elements != 1 && (req->data.bin[1] & CBOR_TYPE_MASK) != CBOR_ARRAY) {
        return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
    }

    //printf("read request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements, 
    //    req->data.bin[pos], req->data.bin[pos+1], req->data.bin[pos+2], req->data.bin[pos+3], 
    //    req->data.bin[pos+4], req->data.bin[pos+5], req->data.bin[pos+6], req->data.bin[pos+6]);

    if (num_elements > 1) {
        resp->pos += cbor_serialize_array(&resp->data.bin[resp->pos], num_elements, resp->size - resp->pos);
    }

    while (pos + 1 < req->pos && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        uint16_t id;
        num_bytes = cbor_deserialize_uint16(&req->data.bin[pos], &id);
        if (num_bytes == 0) {
            return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        const data_object_t* data_obj = thingset_data_object_by_id(data, id);
        if (data_obj == NULL) {
            return _status_msg(resp, TS_STATUS_UNKNOWN_DATA_OBJ);
        }
        if (!(data_obj->access & TS_ACCESS_READ)) {
            return _status_msg(resp, TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = _cbor_serialize_data_object(resp, data_obj);

        if (num_bytes == 0) {
            return _status_msg(resp, TS_STATUS_RESPONSE_TOO_LONG);
        } else {
            resp->pos += num_bytes;
        }
        element++;
    }

    if (element == num_elements) {
        return TS_STATUS_SUCCESS;
    } else {
        return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
    }
}

int thingset_write_cbor(ts_buffer_t *req, ts_buffer_t *resp, ts_data_t *data)
{
    unsigned int pos = 1;       // ignore first byte for function code in request
    uint16_t num_elements, element = 0;

    pos += cbor_num_elements(&req->data.bin[1], &num_elements);
    if ((req->data.bin[1] & CBOR_TYPE_MASK) != CBOR_MAP) {
        return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
    }

    //printf("write request, elements: %d, hex data: %x %x %x %x %x %x %x %x\n", num_elements, 
    //    req->data.bin[pos], req->data.bin[pos+1], req->data.bin[pos+2], req->data.bin[pos+3], 
    //    req->data.bin[pos+4], req->data.bin[pos+5], req->data.bin[pos+6], req->data.bin[pos+6]);

    // loop through all elements to check if request is valid
    while (pos < req->pos && element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        uint16_t id;
        num_bytes = cbor_deserialize_uint16(&req->data.bin[pos], &id);
        if (num_bytes == 0) {
            return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        const data_object_t* data_obj = thingset_data_object_by_id(data, id);
        if (data_obj == NULL) {
            return _status_msg(resp, TS_STATUS_UNKNOWN_DATA_OBJ);
        }
        if (!(data_obj->access & TS_ACCESS_WRITE)) {
            return _status_msg(resp, TS_STATUS_UNAUTHORIZED);
        }

        num_bytes = _deserialize_data_object(&req->data.bin[pos], data_obj);

        if (num_bytes == 0) {
            return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
        }
        pos += num_bytes;

        element++;
    }

    if (element == num_elements) {
        return _status_msg(resp, TS_STATUS_SUCCESS);
    } else {
        return _status_msg(resp, TS_STATUS_WRONG_FORMAT);
    }
}

int thingset_pub_msg_cbor(ts_buffer_t *resp, ts_data_t *data, uint16_t pub_list[], size_t num_elements)
{
    size_t element = 0;

    resp->data.bin[0] = TS_FUNCTION_PUBMSG;
    resp->pos = 1;

    if (num_elements > 1) {
        resp->pos += cbor_serialize_map(&resp->data.bin[resp->pos], num_elements, resp->size - resp->pos);
    }
    
    while (element < num_elements) {

        size_t num_bytes = 0;       // temporary storage of cbor data length (req and resp)

        const data_object_t* data_obj = thingset_data_object_by_id(data, pub_list[element]);
        if (data_obj == NULL) {
            return _status_msg(resp, TS_STATUS_UNKNOWN_DATA_OBJ);
        }
        if (!(data_obj->access & TS_ACCESS_READ)) {
            return _status_msg(resp, TS_STATUS_UNAUTHORIZED);
        }

        resp->pos += cbor_serialize_uint(&resp->data.bin[resp->pos], pub_list[element], resp->size - resp->pos);
        num_bytes = _cbor_serialize_data_object(resp, data_obj);

        if (num_bytes == 0) {
            return _status_msg(resp, TS_STATUS_RESPONSE_TOO_LONG);
        } else {
            resp->pos += num_bytes;
        }
        element++;
    }

    return TS_STATUS_SUCCESS;
}


/*
int thingset_get_data_object_name(void)
{
    _resp->data.bin[0] = TS_OBJ_NAME + 0x80;    // Function ID
    int data_obj_id = _req[1] + ((int)_req[2] << 8);

    for (unsigned int i = 0; i < sizeof(dataObjects)/sizeof(data_object_t); i++) {
        if (dataObjects[i].id == data_obj_id) {
            if (dataObjects[i].access & ACCESS_READ) {
                _resp->data.bin[1] = T_STRING;
                int len = strlen(dataObjects[i].name);
                for (int j = 0; j < len; j++) {
                    _resp->data.bin[j+2] = *(dataObjects[i].name + j);
                }
                #if DEBUG
                serial.printf("Get Data Object Name: %s (id = %d)\n", dataObjects[i].name, data_obj_id);
                #endif
                return len + 2;
            }
            else {
                _resp->data.bin[1] = TS_STATUS_UNAUTHORIZED;
                return 2;   // length of response
            }
        }
    }

    // data object not found --> send error message
    _resp->data.bin[1] = TS_STATUS_DATA_UNKNOWN;
    return 2;   // length of response
}


int thingset_list_cbor(ts_buffer_t *req, ts_buffer_t *resp, ts_data_t *data)
{
    _resp->data.bin[0] = TS_LIST + 0x80;    // Function ID
    //_resp->data.bin[1] = T_UINT16;
    int category = _req[1];
    int num_ids = 0;

    for (unsigned int i = 0; i < sizeof(dataObjects)/sizeof(data_object_t); i++) {
        if (dataObjects[i].access & ACCESS_READ
            && (dataObjects[i].category == category || category == TS_C_ALL))
        {
            if (num_ids * 2 < _resp_max_len - 4) {  // enough space left in buffer
                _resp->data.bin[num_ids*2 + 2] = dataObjects[i].id & 0x00FF;
                _resp->data.bin[num_ids*2 + 3] = dataObjects[i].id >> 8;
                num_ids++;
            }
            else {
                // not enough space in buffer
                _resp->data.bin[1] = TS_STATUS_RESPONSE_TOO_LONG;
                return 2;
            }
        }
    }

    return num_ids*2 + 2;
}
*/
