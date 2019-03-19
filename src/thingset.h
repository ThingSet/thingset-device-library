/* LibreSolar MPPT charge controller firmware
 * Copyright (c) 2016-2018 Martin Jäger (www.libre.solar)
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

#ifndef __THINGSET_H_
#define __THINGSET_H_

#include "ts_config.h"
#include "jsmn.h"

#include <stdint.h>
#include <stdbool.h>

/* Protocol functions
 */
#define TS_FUNCTION_INFO     0x01       // read device info
#define TS_FUNCTION_CONF     0x02       // read/write configuration
#define TS_FUNCTION_INPUT    0x03       // input
#define TS_FUNCTION_OUTPUT   0x04       // output
#define TS_FUNCTION_REC      0x05       // RPC / exec
#define TS_FUNCTION_CAL      0x06       // RPC / exec
#define TS_FUNCTION_EXEC     0x0B       // RPC / function call
#define TS_FUNCTION_NAME     0x0E
#define TS_FUNCTION_AUTH     0x10
#define TS_FUNCTION_LOG      0x11       // access log data
#define TS_FUNCTION_PUB      0x12       // publication request
#define TS_FUNCTION_PUBMSG   0x1F       // actual publication message


/* Status codes
 */
#define TS_STATUS_SUCCESS            0
#define TS_STATUS_ERROR             30
#define TS_STATUS_UNKNOWN_FUNCTION  31    // Function ID unknown
#define TS_STATUS_UNKNOWN_DATA_OBJ  32    // Data Object ID unknown
#define TS_STATUS_WRONG_FORMAT      33
#define TS_STATUS_WRONG_TYPE        34    // Data type not supported
#define TS_STATUS_DEVICE_BUSY       35    // Device busy
#define TS_STATUS_UNAUTHORIZED      36
#define TS_STATUS_REQUEST_TOO_LONG  37
#define TS_STATUS_RESPONSE_TOO_LONG 38
#define TS_STATUS_INVALID_VALUE     39     // value out of allowed range
#define TS_STATUS_WRONG_CATEGORY    40

/* Internal C data types (used to cast void* pointers)
 */
enum ts_type {
    TS_T_BOOL,
    TS_T_UINT64,
    TS_T_INT64,
    TS_T_UINT32,
    TS_T_INT32,
    TS_T_UINT16,
    TS_T_INT16,
    TS_T_FLOAT32,
    TS_T_STRING,
    TS_T_DECFRAC       // CBOR decimal fraction
};

/* Internal access rights to data objects
 */
#define TS_ACCESS_READ          (0x1U)
#define TS_ACCESS_WRITE         (0x1U << 1)
#define TS_ACCESS_READ_AUTH     (0x1U << 2)     // read after authentication
#define TS_ACCESS_WRITE_AUTH    (0x1U << 3)     // write after authentication
#define TS_ACCESS_EXEC          (0x1U << 4)     // execute (for RPC only)
#define TS_ACCESS_EXEC_AUTH     (0x1U << 5)     // execute after authentication

/* ThingSet data object categories
 */
#define TS_CAT_INFO    0x01   // read-only device information (e.g. manufacturer, device ID)
#define TS_CAT_CONF    0x02   // configurable settings
#define TS_CAT_INPUT   0x03   // input data (e.g. set-points)
#define TS_CAT_OUTPUT  0x04   // output data (e.g. measurement values)
#define TS_CAT_REC     0x05   // recorded data (histor-dependent)
#define TS_CAT_CAL     0x06   // calibration
#define TS_CAT_EXEC    0x0B   // Remote Procedure Call


/* for CAN only...
 */
#define PUB_MULTIFRAME_EN (0x1U << 7)
#define PUB_TIMESTAMP_EN (0x1U << 6)

/** ThingSet data object struct
 *
 * id = Data object ID
 * access = one of TS_ACCESS_READ, _WRITE, _EXECUTE, ...
 * type = one of TS_TYPE_INT32, _FLOAT, ...
 * detail = exponent (10^exponent = factor to convert to SI unit) for UINT / INT
 *          decimal digits to use for plotting of floats in JSON strings
 *          lenght of string buffer for string type
 */
typedef struct data_object_t {
    const uint16_t id;
    const uint16_t category;
    const uint8_t access;
    const uint8_t type;
    const int16_t detail;
    void *data;    int pub_msg_cbor(uint8_t *buf, size_t size, int channel);

    const char *name;
} data_object_t;

/** Buffer for string-type and binary data
 *
 * Remark: char type data union necessary to use string functions without casts
 */
typedef struct {
    union {
        char *str;          // pointer to ASCII data
        uint8_t *bin;       // pointer to binary data
    } data;
    size_t size;            // size of the array
    size_t pos;             // index of the next free byte    int pub_msg_cbor(uint8_t *buf, size_t size, int channel);

} ts_buffer_t;

/** ThingSet Data Object container including size
 */
typedef struct ts_data_t {
    const data_object_t *objects;
    size_t size;
} ts_data_t;

/** Parser container for JSON data
 */
typedef struct {
    char *str;
    jsmn_parser parser;
    jsmntok_t tokens[TS_NUM_JSON_TOKENS];
    int tok_count;
} ts_parser_t;


/** Container for data object publication channel
 */
typedef struct {
    const char *name;
    const uint16_t *object_ids;  // array of data object IDs
    const size_t num;             // number of objects
} ts_pub_channel_t;


class ThingSet
{
public:
    ThingSet(const data_object_t *data, size_t num);
    ThingSet(const data_object_t *data, size_t num_obj, const ts_pub_channel_t *channels, size_t num_ch);
    ~ThingSet();

    /** Process ThingSet request
    *   - receives a request and stores the pointer
    *   - performs an action (i.e. thingset function)
    *   - saves the response in resp->data
    *   - returns ThingSet status code
    *
    * This function also detects if JSON or CBOR format is used
    */
    int process(uint8_t *request, size_t req_len, uint8_t *response, size_t resp_size);

    void set_pub_channels(const ts_pub_channel_t *channels, size_t num);

    int set_user_password(char *password);
    int set_manufacturer_password(char *password);

    int pub_msg_json(char *resp, size_t size, unsigned int channel);
    int pub_msg_cbor(uint8_t *resp, size_t size, unsigned int channel);

    // manually supplied list of elements
    int pub_msg_cbor(uint8_t *resp, size_t size, const uint16_t pub_list[], size_t num_elements);

    // function to initialize data objects based on values stored in EEPROM
    // returns ThingSet status code
    int init_cbor(uint8_t *cbor_data, size_t len);

    void set_conf_callback(void (*callback)(void));

    const data_object_t *get_data_object(uint16_t id);
    const data_object_t *get_data_object(char *str, size_t len);

private:

    void set_request(uint8_t *buf, size_t length);
    int get_response(uint8_t *buf, size_t size);


    /* ThingSet JSON functions
    *   - append requested data to resp buffer
    *   - return ThingSet status code
    */

    // call with empty function
    int list_json(char *buf, size_t size, int category, bool values = false);
    int list_cbor(uint8_t *buf, size_t size, int category, bool values = false, bool ids_only = true);

    // function call with array
    int read_json(char *buf, size_t size, int category);
    int read_cbor(uint8_t *buf, size_t size, int category);

    // function call with map
    int write_json(char *buf, size_t size, int category);
    int write_cbor(uint8_t *buf, size_t size, int category, bool ignore_access);

    // function call with single value
    int exec_json(char *buf, size_t size);
    int exec_cbor(uint8_t *buf, size_t size);

    // authentication
    int auth_json(char *buf, size_t size);
    int auth_cbor(uint8_t *buf, size_t size);

    int json_serialize_value(char *buf, size_t size, const data_object_t* data_obj);
    int json_serialize_name_value(char *buf, size_t size, const data_object_t* data_obj);

    int status_message_json(char *buf, size_t size, int code);

    const data_object_t *data_objects;
    size_t num_objects;

    const ts_pub_channel_t *pub_channels;
    size_t num_channels;

    // request raw data
    uint8_t *req;
    size_t req_len;             // length of request

    // parsed JSON data of request
    jsmntok_t tokens[TS_NUM_JSON_TOKENS];
    char *json_str;
    int tok_count;

    const char *user_pass;
    const char *manufacturer_pass;

    void (*conf_callback)(void);
};

#endif
