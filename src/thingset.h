/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
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
// function + data node parent_id
#define TS_INFO     0x01       // read-only device information (e.g. manufacturer, device ID)
#define TS_CONF     0x02       // configurable settings
#define TS_INPUT    0x03       // input data (e.g. set-points)
#define TS_OUTPUT   0x04       // output data (e.g. measurement values)
#define TS_REC      0x05       // recorded data (history-dependent)
#define TS_CAL      0x06       // calibration
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

// success
#define TS_STATUS_CREATED               0x81
#define TS_STATUS_DELETED               0x82
#define TS_STATUS_VALID                 0x83
#define TS_STATUS_CHANGED               0x84
#define TS_STATUS_CONTENT               0x85

// client errors
#define TS_STATUS_BAD_REQUEST           0xA0
#define TS_STATUS_UNAUTHORIZED          0xA1        // need to authenticate
#define TS_STATUS_FORBIDDEN             0xA3        // trying to write read-only value
#define TS_STATUS_NOT_FOUND             0xA4
#define TS_STATUS_METHOD_NOT_ALLOWED    0xA5
#define TS_STATUS_REQUEST_INCOMPLETE    0xA8
#define TS_STATUS_CONFLICT              0xA9
#define TS_STATUS_REQUEST_TOO_LARGE     0xAD
#define TS_STATUS_UNSUPPORTED_FORMAT    0xAF

// server errors
#define TS_STATUS_INTERNAL_SERVER_ERR   0xC0
#define TS_STATUS_NOT_IMPLEMENTED       0xC1

// ThingSet specific errors
#define TS_STATUS_RESPONSE_TOO_LARGE    0xE1

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
    TS_T_DECFRAC,       // CBOR decimal fraction
    TS_T_PATH,          // internal node to describe URI path
    TS_T_NODE_ID,       // internally equal to uint16_t
    TS_T_FUNCTION       // for exec data objects
};

/**
 * Data structure to store the array pointer, length, and its type
 */
typedef struct {
    void *ptr;          ///< Pointer to the array
    int num_elements;   ///< Number of elements in the array. For eg, sizeof(data)/sizeof(data[0]);
    uint8_t type;       ///< Type of the array
    int max_elements;   ///< The maximum number of elements in the array
} ArrayInfo;

/*
 * Functions to generate data_node map and make compiler complain if wrong
 * type is passed
 */

#define TS_DATA_NODE_ID_CHECK(_id) TS_DATA_NODE_ID_TEMP_##_id

static inline void *_bool_to_void(bool *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_BOOL(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_BOOL, 0, _bool_to_void(_data_ptr), _name}

static inline void *_uint64_to_void(uint64_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_UINT64(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_UINT64, 0, _uint64_to_void(_data_ptr), _name}

static inline void *_int64_to_void(int64_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_INT64(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_INT64, 0, _int64_to_void(_data_ptr), _name}

static inline void *_uint32_to_void(uint32_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_UINT32(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_UINT32, 0, _uint32_to_void(_data_ptr), _name}

static inline void *_int32_to_void(int32_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_INT32(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_INT32, 0, _int32_to_void(_data_ptr), _name}

static inline void *_uint16_to_void(uint16_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_UINT16(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_UINT16, 0, _uint16_to_void(_data_ptr), _name}

static inline void *_int16_to_void(int16_t *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_INT16(_id, _name, _data_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_INT16, 0, _int16_to_void(_data_ptr), _name}

static inline void *_float_to_void(float *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_FLOAT(_id, _name, _data_ptr, _digits, _parent, _acc) \
    {_id, _parent, _acc, TS_T_FLOAT32, _digits, _float_to_void(_data_ptr), _name}

static inline void *_string_to_void(const char *ptr) { return (void*) ptr; }
#define TS_DATA_NODE_STRING(_id, _name, _data_ptr, _buf_size, _parent, _acc) \
    {_id, _parent, _acc, TS_T_STRING, _buf_size, _string_to_void(_data_ptr), _name}

static inline void *_function_to_void(void (*fnptr)()) { return (void*) fnptr; }
#define TS_DATA_NODE_EXEC(_id, _name, _function_ptr, _parent, _acc) \
    {_id, _parent, _acc, TS_T_FUNCTION, 0, _function_to_void(_function_ptr), _name}

static inline void *_array_to_void(ArrayInfo *ptr) { return (void *) ptr; }
#define TS_DATA_NODE_ARRAY(_id, _name, _data_ptr, _digits, _parent, _acc) \
    {_id, _parent, _acc, TS_T_ARRAY, _digits, _array_to_void(_data_ptr), _name}

#define TS_DATA_NODE_PATH(_id, _name, _parent, _callback) \
    {_id, _parent, TS_READ_ALL, TS_T_PATH, 0, _function_to_void(_callback), _name}

/*
 * This macro should be called first in the data_nodes array
 */
#define TS_CATEGORIES_DATA_NODES \
    TS_DATA_NODE_PATH(TS_INFO,   "info",   0, NULL),  \
    TS_DATA_NODE_PATH(TS_CONF,   "conf",   0, NULL),  \
    TS_DATA_NODE_PATH(TS_INPUT,  "input",  0, NULL),  \
    TS_DATA_NODE_PATH(TS_OUTPUT, "output", 0, NULL),  \
    TS_DATA_NODE_PATH(TS_REC,    "rec",    0, NULL),  \
    TS_DATA_NODE_PATH(TS_CAL,    "cal",    0, NULL),  \
    TS_DATA_NODE_PATH(TS_EXEC,   "exec",   0, NULL),  \
    TS_DATA_NODE_PATH(TS_NAME,   "name",   0, NULL),  \
    TS_DATA_NODE_PATH(TS_AUTH,   "auth",   0, NULL),  \
    TS_DATA_NODE_PATH(TS_LOG,    "log",    0, NULL),  \
    TS_DATA_NODE_PATH(TS_PUB,    "pub",    0, NULL)

/*
 * Deprecate old macros
 */
#define TS_DATA_OBJ_BOOL _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_BOOL

#define TS_DATA_OBJ_UINT64 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_UINT64

#define TS_DATA_OBJ_INT64 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_INT64

#define TS_DATA_OBJ_UINT32 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_UINT32

#define TS_DATA_OBJ_INT32 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_INT32

#define TS_DATA_OBJ_UINT16 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_UINT16

#define TS_DATA_OBJ_INT16 _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_INT16

#define TS_DATA_OBJ_FLOAT _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_FLOAT

#define TS_DATA_OBJ_STRING _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_STRING

#define TS_DATA_OBJ_EXEC _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_EXEC

#define TS_DATA_OBJ_ARRAY _Pragma \
    ("GCC warning \"TS_DATA_NODE_... macros are deprecated. Use TS_DATA_NODE_... instead!\"") \
    TS_DATA_NODE_ARRAY

/*
 * Internal access rights to data nodes
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


typedef uint16_t ts_node_id_t;

/**
 * ThingSet data node struct
 */
typedef struct DataNode {
    /**
     * Data node ID
     */
    const ts_node_id_t id;

    /**
     * ID of parent node
     */
    const ts_node_id_t parent;

    /**
     * One of TS_ACCESS_READ, _WRITE, _EXECUTE, ...
     */
    const uint8_t access;

    /**
     * One of TS_TYPE_INT32, _FLOAT, ...
     */
    const uint8_t type;

    /**
     * Exponent (10^exponent = factor to convert to SI unit) for decimal fraction type,
     * decimal digits to use for printing of floats in JSON strings or
     * lenght of string buffer for string type
     */
    const int16_t detail;

    /**
     * Pointer to the variable containing the data. The variable type must match the type as
     * specified
     */
    void *data;

    /**
     * Data Nnode name
     */
    const char *name;
} DataNode;

/* old naming convention of structs with _t is deprecated and will be removed in the future */
__attribute__((deprecated)) typedef DataNode data_object_t;


class ThingSet
{
public:
    ThingSet(const DataNode *data, size_t num);

    ~ThingSet();

    /**
     * Process ThingSet request
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

    /**
     * Plot all data nodes as a structured JSON text to stdout
     *
     * WARNING: This is a recursive function and might cause stack overflows if run in constrained
     *          devices with large data node tree. Use with care and for testing only!
     *
     * @param node_id Root node ID where to start with plotting
     * @param level Indentation level (=depth inside the data node tree)
     */
    void dump_json(uint16_t node_id = 0, int level = 0);

    /**
     * Sets password for users
     */
    void set_user_password(const char *password);

    /**
     * Sets password for maker/manufacturer
     */
    void set_maker_password(const char *password);

    /**
     * Generate publication message in JSON format for supplied list of data node IDs
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param node_ids Array of data node IDs to be published
     * @param num_ids Number of elements in the nodes array
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int pub_msg_json(char *buf, size_t buf_size, const ts_node_id_t node_ids[], size_t num_ids);

    /**
     * Generate publication message in CBOR format for supplied list of data node IDs
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param node_ids Array of data node IDs to be published
     * @param num_ids Number of elements in the nodes array
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int pub_msg_cbor(uint8_t *buf, size_t buf_size, const ts_node_id_t node_ids[], size_t num_ids);

    /**
     * Encodes a publication message in CAN message format for supplied data node
     *
     * @param can_node_id id of the can node
     * @param msg_id reference to can message id storage
     * @param data_node reference to data node to be published
     * @param msg_data reference to the buffer where the publication message should be stored
     *
     * @returns Actual length of the message_data, or -1 if not encodable / in case of error,
     *          message length otherwise. msg_len 0 is valid, just the id is transmitted
     */
    int encode_msg_can(const DataNode& object, uint8_t can_node_id, unsigned int& msg_id,
        uint8_t (&msg_data)[8]);

    /**
     * Initialize data nodes based on values stored in EEPROM
     *
     * @param cbor_data Buffer containing key/value map that should be written to the ThingSet data
     *                  objects
     * @param len Length of the data in the buffer
     *
     * @returns ThingSet status code
     */
    int init_cbor(uint8_t *cbor_data, size_t len);

    /**
     * Get data node by ID
     */
    const DataNode *get_data_node(uint16_t id);

    /**
     * Get data node by name
     *
     * As the names are not necessarily unique in the entire data tree, the parent is needed
     *
     * @param name node name
     * @param len length of the node name
     * @param parent node ID of the parent or -1 for global search
     *
     * @returns Pointer to data node or NULL if node is not found
     */
    const DataNode *get_data_node(const char *name, size_t len, int32_t parent = -1);

    /**
     * Get the endpoint node of a provided path, e.g. "conf"
     */
    const DataNode *get_endpoint_node(const char *path, size_t len);

private:
    /**
     * Parser preparation and calling of the different data node access functions read/write/list
     */
    int process_json();

    /**
     * GET request (text mode)
     *
     * List child data nodes (function called without content / parameters)
     */
    int get_json(const DataNode *parent_node, bool include_values = false);

    /**
     * GET request (binary mode)
     *
     * List child data nodes (function called without content / parameters)
     */
    int get_cbor(uint16_t parent_id, bool values = false, bool ids_only = true);

    /**
     * FETCH request (text mode)
     *
     * Read data node values (function called with an array as argument)
     */
    int fetch_json(uint16_t parent_id);

    /**
     * FETCH request (binary mode)
     *
     * Read data node values (function called with an array as argument)
     */
    int fetch_cbor(uint16_t parent_id);

    /**
     * PATCH request (text mode)
     *
     * Write data node values in text mode (function called with a map as argument)
     */
    int patch_json(uint16_t parent_id);

    /**
     * PATCH request (binary mode)
     *
     * Write data node values in binary mode (function called with a map as argument)
     */
    int patch_cbor(uint16_t parent_id, bool ignore_access);

    /**
     * POST request to append data
     */
    int create_json(const DataNode *node);

    /**
     * DELETE request to delete data from node
     */
    int delete_json(const DataNode *node);

    /**
     * Execute command in text mode (function called with a single data node name as argument)
     */
    int exec_json(const DataNode *node);

    /**
     * Execute command in binary mode (function called with a single data node name/id as argument)
     */
    int exec_cbor();

    /**
     * Authentication command in text mode (user or root level)
     */
    int auth_json();

    /**
     * Fill the resp buffer with a JSON response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int status_message_json(int code);

    int json_serialize_value(char *resp, size_t size, const DataNode *data_node);

    int json_serialize_name_value(char *resp, size_t size, const DataNode* data_node);

    int json_deserialize_value(char *buf, size_t len, int tok, const DataNode *data_node);

    /**
     * Fill the resp buffer with a CBOR response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int status_message_cbor(uint8_t code);

    const DataNode *data_nodes;
    size_t num_nodes;

    uint8_t *req;               ///< Request buffer
    size_t req_len;             ///< Length of request

    uint8_t *resp;              ///< Response buffer
    size_t resp_size;           ///< Size of response buffer (i.e. maximum length)

    /**
     * Pointer to the start of JSON payload in the request
     */
    char *json_str;

    /**
     * JSON tokes in json_str parsed by JSMN
     */
    jsmntok_t tokens[TS_NUM_JSON_TOKENS];

    /**
     * Number of JSON tokens parsed by JSMN
     */
    int tok_count;

    const char *user_pass = NULL;
    const char *maker_pass = NULL;

    bool user_authorized = false;
    bool maker_authorized = false;
};

#endif
