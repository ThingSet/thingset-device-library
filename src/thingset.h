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

#ifndef __THINGSET_H_
#define __THINGSET_H_

#include "ts_config.h"
#include "jsmn.h"

#include <stdint.h>
#include <stdbool.h>

/*
 * Protocol functions / categories
 */
// function + data object category
#define TS_INFO     0x01       // read-only device information (e.g. manufacturer, device ID)
#define TS_CONF     0x02       // configurable settings
#define TS_INPUT    0x03       // input data (e.g. set-points)
#define TS_OUTPUT   0x04       // output data (e.g. measurement values)
#define TS_REC      0x05       // recorded data (history-dependent)
#define TS_CAL      0x06       // calibration
#define TS_ANY      0x09       // any of above non-exec categories 0x01-0x08
#define TS_EXEC     0x0B       // RPC / function call
// function only
#define TS_NAME     0x0E
#define TS_AUTH     0x10
#define TS_LOG      0x11       // access log data
#define TS_PUB      0x12       // publication request
#define TS_PUBMSG   0x1F       // actual publication message

/*
 * Status codes
 */
#define TS_STATUS_SUCCESS            0
#define TS_STATUS_ERROR             32
#define TS_STATUS_UNKNOWN_FUNCTION  33    // Function ID unknown
#define TS_STATUS_UNKNOWN_DATA_OBJ  34    // Data Object ID unknown
#define TS_STATUS_WRONG_FORMAT      35
#define TS_STATUS_WRONG_TYPE        36    // Data type not supported
#define TS_STATUS_DEVICE_BUSY       37    // Device busy
#define TS_STATUS_UNAUTHORIZED      38
#define TS_STATUS_REQUEST_TOO_LONG  39
#define TS_STATUS_RESPONSE_TOO_LONG 40
#define TS_STATUS_INVALID_VALUE     41     // value out of allowed range
#define TS_STATUS_WRONG_CATEGORY    42
#define TS_STATUS_WRONG_PASSWORD    43
#define TS_STATUS_UNSUPPORTED       44    // type of request not (yet) supported

/**
 * Internal C data types (used to cast void* pointers)
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
    TS_T_ARRAY,
    TS_T_DECFRAC       // CBOR decimal fraction
};

/**
 * Data structure to store the array pointer, length, and its type
 */
typedef struct {
    void *ptr;          ///< Pointer to the array
    int num_elements;            ///< Number of elements in the array. For eg, sizeof(data)/sizeof(data[0]);
    uint8_t type;       ///< Type of the array
    int max_elements;   ///< The maximum number of elements in the array
} ArrayInfo;

/*
 * Functions to generate data_object map and make compiler complain if wrong
 * type is passed
 */

#define TS_DATA_OBJ_ID_CHECK(_id) TS_DATA_OBJ_ID_TEMP_##_id

static inline void *_bool_to_void(bool *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_BOOL(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_BOOL, 0, _bool_to_void(_data_ptr), _name}

static inline void *_uint64_to_void(uint64_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_UINT64(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_UINT64, 0, _uint64_to_void(_data_ptr), _name}

static inline void *_int64_to_void(int64_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_INT64(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_INT64, 0, _int64_to_void(_data_ptr), _name}

static inline void *_uint32_to_void(uint32_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_UINT32(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_UINT32, 0, _uint32_to_void(_data_ptr), _name}

static inline void *_int32_to_void(int32_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_INT32(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_INT32, 0, _int32_to_void(_data_ptr), _name}

static inline void *_uint16_to_void(uint16_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_UINT16(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_UINT16, 0, _uint16_to_void(_data_ptr), _name}

static inline void *_int16_to_void(int16_t *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_INT16(_id, _name, _data_ptr, _cat, _acc) \
    {_id, _cat, _acc, TS_T_INT16, 0, _int16_to_void(_data_ptr), _name}

static inline void *_float_to_void(float *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_FLOAT(_id, _name, _data_ptr, _digits, _cat, _acc) \
    {_id, _cat, _acc, TS_T_FLOAT32, _digits, _float_to_void(_data_ptr), _name}

static inline void *_string_to_void(const char *ptr) { return (void*) ptr; }
#define TS_DATA_OBJ_STRING(_id, _name, _data_ptr, _buf_size, _cat, _acc) \
    {_id, _cat, _acc, TS_T_STRING, _buf_size, _string_to_void(_data_ptr), _name}

static inline void *_function_to_void(void (*fnptr)()) { return (void*) fnptr; }
#define TS_DATA_OBJ_EXEC(_id, _name, _data_ptr, _acc) \
    {_id, TS_EXEC, _acc, TS_T_BOOL, 0, _function_to_void(_data_ptr), _name}

static inline void *_array_to_void(ArrayInfo *ptr) { return (void *) ptr; }
#define TS_DATA_OBJ_ARRAY(_id, _name, _data_ptr, _digits, _cat, _acc) \
    {_id, _cat, _acc, TS_T_ARRAY, _digits, _array_to_void(_data_ptr), _name}

/*
 * Internal access rights to data objects
 */
#define TS_READ_ALL     (0x1U << 0)     // read access for all
#define TS_READ_USER    (0x3U << 0)     // read after authentication as normal user
#define TS_READ_MAKER   (0x7U << 0)     // read after authentication as manufacturer, e.g. for factory calibration
#define TS_READ_MASK    TS_READ_MAKER   // maker has all 3 bits set

#define TS_WRITE_ALL    (0x1U << 3)     // write access for all
#define TS_WRITE_USER   (0x3U << 3)     // write after authentication as normal user
#define TS_WRITE_MAKER  (0x7U << 3)     // write after authentication as manufacturer, e.g. for factory calibration
#define TS_WRITE_MASK   TS_WRITE_MAKER

#define TS_EXEC_ALL     (0x1U << 6)     // execute access for all (only for RPC)
#define TS_EXEC_USER    (0x3U << 6)     // execute after authentication as normal user
#define TS_EXEC_MAKER   (0x7U << 6)     // execute after authentication as manufacturer, e.g. for factory calibration
#define TS_EXEC_MASK    TS_EXEC_MAKER

// legacy names
#define TS_ACCESS_READ          TS_READ_ALL
#define TS_ACCESS_READ_AUTH     TS_READ_USER
#define TS_ACCESS_WRITE         TS_WRITE_ALL
#define TS_ACCESS_WRITE_AUTH    TS_WRITE_USER
#define TS_ACCESS_EXEC          TS_EXEC_ALL
#define TS_ACCESS_EXEC_AUTH     TS_EXEC_USER

/** ThingSet data object struct
 */
typedef struct data_object_t {
    /** Data Object ID
     */
    const uint16_t id;

    /** One of TS_INFO, TS_CONF, ...
     */
    const uint16_t category;

    /** One of TS_ACCESS_READ, _WRITE, _EXECUTE, ...
     */
    const uint8_t access;

    /** One of TS_TYPE_INT32, _FLOAT, ...
     */
    const uint8_t type;

    /** Exponent (10^exponent = factor to convert to SI unit) for decimal fraction type,
     * decimal digits to use for printing of floats in JSON strings or
     * lenght of string buffer for string type
     */
    const int16_t detail;

    /** Pointer to the variable containing the data. The variable type must match the type as specified
     */
    void *data;

    /** Data Object name
     */
    const char *name;
} data_object_t;

/** Container for data object publication channel
 */
typedef struct {
    const char *name;           ///< Publication channel name
    const uint16_t *object_ids; ///< Array of data object IDs
    const size_t num;           ///< Number of objects
    bool enabled;               ///< Enabled/disabled status
} ts_pub_channel_t;


class ThingSet
{
public:
    ThingSet(const data_object_t *data, size_t num);
    ThingSet(const data_object_t *data, size_t num_obj, ts_pub_channel_t *channels, size_t num_ch);
    ~ThingSet();

    /** Process ThingSet request
     *
     * This function also detects if JSON or CBOR format is used
     *
     * @param request Pointer to the ThingSet request buffer
     * @param req_len Length of the data in the request buffer
     * @param response Pointer to the buffer where the ThingSet response should be stored
     * @param resp_size Size of the response buffer, i.e. maximum allowed length of the response

     * @returns Actual length of the response written to the buffer or 0 in case of error
     */
    int process(uint8_t *request, size_t req_len, uint8_t *response, size_t resp_size);

    void set_pub_channels(ts_pub_channel_t *channels, size_t num);

    /** Sets password for users
     */
    void set_user_password(const char *password);

    /** Sets password for maker/manufacturer
     */
    void set_maker_password(const char *password);

    /** Generates a publication message in JSON format for a defined channel
     *
     * @param msg_buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param channel Number/ID of the publication channel
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int pub_msg_json(char *msg_buf, size_t size, unsigned int channel);

    /** Generates a publication message in CBOR format for a defined channel
     *
     * @param msg_buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param channel Number/ID of the publication channel
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int pub_msg_cbor(uint8_t *msg_buf, size_t size, unsigned int channel);

    /** Generates a publication message in CBOR format for supplied list of data object IDs
     *
     * @param msg_buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param pub_list Array of data object IDs to be published
     * @param num_elements Number of elements in the pub_list array
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int pub_msg_cbor(uint8_t *resp, size_t size, const uint16_t pub_list[], size_t num_elements);

    /** Encodes a publication message in CAN message format for supplied data object
     *
     * @param can_node_id id of the can node
     * @param msg_id reference to can message id storage
     * @param data_object reference to data object to be published
     * @param msg_data reference to the buffer where the publication message should be stored
     *
     * @returns Actual length of the message_data, or -1 if not encodable / in case of error
     */
    int encode_msg_can(const data_object_t& object, uint8_t can_node_id, unsigned int& msg_id, uint8_t (&msg_data)[8]);

    /** Initialize data objects based on values stored in EEPROM
     *
     * @param cbor_data Buffer containing key/value map that should be written to the ThingSet data objects
     * @param len Length of the data in the buffer
     *
     * @returns ThingSet status code
     */
    int init_cbor(uint8_t *cbor_data, size_t len);

    /** Set function to be called when data objects of conf category were changed
     */
    void set_conf_callback(void (*callback)(void));

    /** Get data object by ID
     */
    const data_object_t *get_data_object(uint16_t id);

    /** Get data object by name
     */
    const data_object_t *get_data_object(char *name, size_t len);

    /** Get pub channel by name
     */
    ts_pub_channel_t *get_pub_channel(char *name, size_t len);

    /** Get pub channel by id
     */
    inline ts_pub_channel_t *get_pub_channel(unsigned int id)
    {
        return id < num_channels ? &pub_channels[id] : NULL;
    }

private:
    /**
     * Parser preparation and calling of the different data object access functions read/write/list
     */
    int access_json(int category, size_t pos);

    /**
     * List data objects in text mode (function called with empty argument)
     */
    int list_json(int category, bool values = false);

    /**
     * List data objects in binary mode (function called with empty argument)
     */
    int list_cbor(int category, bool values = false, bool ids_only = true);

    /**
     * Read data object values in text mode (function called with an array as argument)
     */
    int read_json(int category);

    /**
     * Read data object values in binary mode (function called with an array as argument)
     */
    int read_cbor(int category);

    /**
     * Write data object values in text mode (function called with a map as argument)
     */
    int write_json(int category);

    /**
     * Write data object values in binary mode (function called with a map as argument)
     */
    int write_cbor(int category, bool ignore_access);

    /**
     * Execute command in text mode (function called with a single data object name as argument)
     */
    int exec_json();

    /**
     * Execute command in binary mode (function called with a single data object name/id as argument)
     */
    int exec_cbor();

    /** Publication control function in text mode
     */
    int pub_json();

    /** Authentication command in text mode (user or root level)
     */
    int auth_json();

    //int auth_cbor();

    /**
     * Fill the resp buffer with a JSON response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int status_message_json(int code);

    /**
     * Fill the resp buffer with a CBOR response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int status_message_cbor(uint8_t code);

    const data_object_t *data_objects;
    size_t num_objects;

    ts_pub_channel_t *pub_channels;
    size_t num_channels;

    uint8_t *req;               ///< Request buffer
    size_t req_len;             ///< Length of request

    uint8_t *resp;              ///< Response buffer
    size_t resp_size;           ///< Size of response buffer (i.e. maximum length)

    // parsed JSON data of request
    jsmntok_t tokens[TS_NUM_JSON_TOKENS];
    char *json_str;
    int tok_count;

    const char *user_pass = NULL;
    const char *maker_pass = NULL;

    bool user_authorized = false;
    bool maker_authorized = false;

    void (*conf_callback)(void);
};

#endif
