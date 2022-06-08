/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_H_
#define THINGSET_H_

#include "ts_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "jsmn.h"
#include "cbor.h"

/*
 * Protocol function codes (same as CoAP)
 */
#define TS_GET       0x01 /**< GET request */
#define TS_POST      0x02 /**< POST request */
#define TS_DELETE    0x04 /**< DELETE request */
#define TS_FETCH     0x05 /**< FETCH request */
#define TS_PATCH     0x07 /**< PATCH request (actually iPATCH equivalent in CBOR) */
#define TS_STATEMENT 0x1F /**< STATEMENT message */

/*
 * Status codes (same as CoAP)
 */

// success
#define TS_STATUS_CREATED               0x81 /**< Success status 0x81: Created */
#define TS_STATUS_DELETED               0x82 /**< Success status 0x82: Deleted */
#define TS_STATUS_VALID                 0x83 /**< Success status 0x83: Valid */
#define TS_STATUS_CHANGED               0x84 /**< Success status 0x84: Changed */
#define TS_STATUS_CONTENT               0x85 /**< Success status 0x85: Content */

// client errors
#define TS_STATUS_BAD_REQUEST           0xA0 /**< Client error 0xA0: Bad Request */
#define TS_STATUS_UNAUTHORIZED          0xA1 /**< Client error 0xA1: Authentication required */
#define TS_STATUS_FORBIDDEN             0xA3 /**< Client error 0xA3: Forbidden to write read-only value */
#define TS_STATUS_NOT_FOUND             0xA4 /**< Client error 0xA4: Not found */
#define TS_STATUS_METHOD_NOT_ALLOWED    0xA5 /**< Client error 0xA5: Method not allowed */
#define TS_STATUS_REQUEST_INCOMPLETE    0xA8 /**< Client error 0xA8: Request incomplete */
#define TS_STATUS_CONFLICT              0xA9 /**< Client error 0xA9: Conflict */
#define TS_STATUS_REQUEST_TOO_LARGE     0xAD /**< Client error 0xAD: Request too large */
#define TS_STATUS_UNSUPPORTED_FORMAT    0xAF /**< Client error 0xAF: Unsupported format */

// server errors
#define TS_STATUS_INTERNAL_SERVER_ERR   0xC0 /**< Server error 0xC0: Internal server error */
#define TS_STATUS_NOT_IMPLEMENTED       0xC1 /**< Server error 0xC1: Not implemented */

// ThingSet specific errors
#define TS_STATUS_RESPONSE_TOO_LARGE    0xE1 /**< ThingSet error 0xE1: Response too large */

/*
 * Reserved data object IDs
 */
#define TS_ID_ROOT              0x00 /**< Data Object ID for root element */
#define TS_ID_TIME              0x10 /**< Data Object ID for timestamp (t_s) */
#define TS_ID_IDS               0x16 /**< Data Object ID to determine IDs from paths (_ids) */
#define TS_ID_PATHS             0x17 /**< Data Object ID to determine paths from IDs (_paths) */
#define TS_ID_METADATAURL       0x18 /**< Data Object ID for Metadata URL (cMetadataURL) */
#define TS_ID_NODEID            0x1D /**< Data Object ID for node ID (cNodeID) */

/*
 * ThingSet addressing in 29-bit CAN ID similar to SAE J1939
 *
 * In order to avoid collisions with SAE J1939 and NMEA 2000, bit 25 (EDP) is always set to 1 and
 * bit 24 (DP) is used to distinguish between request/response and pub/sub messages.
 *
 * Request/response messages using ISO-TP:
 *
 *   --------------------------------------------------------------------
 *   | 28 .. 26 | 25 | 24 |    23 .. 16     |   15 .. 8   |   7 .. 0    |
 *   --------------------------------------------------------------------
 *   | Priority | 1  | 0  | reserved (0xDA) | target addr | source addr |
 *   --------------------------------------------------------------------
 *
 *   Bits 23..16 are set to 218 (0xDA) as suggested by ISO-TP standard (ISO 15765-2) for normal
 *   fixed addressing with N_TAtype = physical
 *
 * Control and pub/sub messages (always single-frame):
 *
 *   --------------------------------------------------------------------
 *   | 28 .. 26 | 25 | 24 |   23 .. 16    |   15 .. 8     |   7 .. 0    |
 *   --------------------------------------------------------------------
 *   | Priority | 1  | 1  | data ID (MSB) | data ID (LSB) | source addr |
 *   --------------------------------------------------------------------
 *
 *   Priority:
 *     0 .. 3: High-priority control frames
 *     4 .. 7: Normal pub/sub frames for monitoring
 */

#define TS_CAN_SOURCE_POS               (0U)
#define TS_CAN_SOURCE_MASK              (0xFF << TS_CAN_SOURCE_POS)
#define TS_CAN_SOURCE_SET(addr)         ((uint32_t)addr << TS_CAN_SOURCE_POS)

#define TS_CAN_TARGET_POS               (8U)
#define TS_CAN_TARGET_MASK              (0xFF << TS_CAN_TARGET_POS)
#define TS_CAN_TARGET_SET(addr)         ((uint32_t)addr << TS_CAN_TARGET_POS)

#define TS_CAN_DATA_ID_POS              (8U)
#define TS_CAN_DATA_ID_MASK             (0xFFFF << TS_CAN_DATA_ID_POS)
#define TS_CAN_DATA_ID_SET(id)          ((uint32_t)id << TS_CAN_DATA_ID_POS)

#define TS_CAN_J1939_DP                 (0x1 << 24U)
#define TS_CAN_J1939_EDP                (0x1 << 25U)

#define TS_CAN_TYPE_REQRESP             (TS_CAN_J1939_EDP)
#define TS_CAN_TYPE_PUBSUB              (TS_CAN_J1939_EDP | TS_CAN_J1939_DP)
#define TS_CAN_TYPE_MASK                (TS_CAN_J1939_EDP | TS_CAN_J1939_DP)

#define TS_CAN_PRIO_POS                 (26U)
#define TS_CAN_PRIO_MASK                (0x7 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_SET(prio)           ((uint32_t)prio << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_GET(id)             (((uint32_t)id & TS_CAN_PRIO_MASK) >> TS_CAN_PRIO_POS)

// some pre-defined priorities
#define TS_CAN_PRIO_CONTROL_EMERGENCY   (0x0 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_CONTROL_HIGH        (0x2 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_CONTROL_LOW         (0x3 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_PUBSUB_HIGH         (0x5 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_REQRESP             (0x6 << TS_CAN_PRIO_POS)
#define TS_CAN_PRIO_PUBSUB_LOW          (0x7 << TS_CAN_PRIO_POS)

// base configuration to create valid CAN IDs
#define TS_CAN_BASE_CONTROL             TS_CAN_TYPE_PUBSUB
#define TS_CAN_BASE_PUBSUB              TS_CAN_TYPE_PUBSUB
#define TS_CAN_BASE_REQRESP             (TS_CAN_TYPE_REQRESP | 218U << 16)  // N_TAtype = physical

// below macros return true if the CAN ID matches the specified message type
#define TS_CAN_CONTROL(id)              (((id & TS_CAN_TYPE_MASK) == TS_CAN_TYPE_PUBSUB) && \
                                        TS_CAN_PRIO_GET(id) < 4)
#define TS_CAN_PUBSUB(id)               (((id & TS_CAN_TYPE_MASK) == TS_CAN_TYPE_PUBSUB) && \
                                        TS_CAN_PRIO_GET(id) >= 4)
#define TS_CAN_REQRESP(id)              ((id & TS_CAN_TYPE_MASK) == TS_CAN_TYPE_REQRESP)

/**
 * Internal C data types (used to cast void* pointers)
 */
enum TsType {
    TS_T_BOOL,      /**< bool */
    TS_T_UINT64,    /**< uint64_t */
    TS_T_INT64,     /**< int64_t */
    TS_T_UINT32,    /**< uint32_t */
    TS_T_INT32,     /**< int32_t */
    TS_T_UINT16,    /**< uint16_t */
    TS_T_INT16,     /**< int16_t */
    TS_T_FLOAT32,   /**< float */
    TS_T_STRING,    /**< String buffer (UTF-8 text) */
    TS_T_BYTES,     /**< Byte buffer (binary data) */
    TS_T_ARRAY,     /**< Array */
    TS_T_DECFRAC,   /**< CBOR decimal fraction */
    TS_T_GROUP,     /**< Internal object to describe data hierarchy */
    TS_T_EXEC,      /**< Executable functions */
    TS_T_SUBSET,    /**< Subset of data items */
};

/**
 * Data structure to specify a binary data buffer
 */
struct ts_bytes_buffer {
    uint8_t *bytes;             /**< Pointer to the buffer */
    uint16_t num_bytes;         /**< Actual number of bytes in the buffer */
};

/**
 * Data structure to specify an array data object
 */
struct ts_array {
    void *elements;             /**< Pointer to the first element of the array */
    uint16_t max_elements;      /**< Maximum number of elements in the array */
    uint16_t num_elements;      /**< Actual number of elements in the array */
    uint8_t type;               /**< Type of the array elements */
};

#define ts_array_info ts_array __attribute__((deprecated))

/** @cond INTERNAL_HIDDEN */
/*
 * Functions to generate data_objects map and make compiler complain if wrong
 * type is passed
 */

#ifdef __cplusplus
static inline void *ts_bool_to_void(bool *ptr) { return (void*) ptr; }
static inline void *ts_uint64_to_void(uint64_t *ptr) { return (void*) ptr; }
static inline void *ts_int64_to_void(int64_t *ptr) { return (void*) ptr; }
static inline void *ts_uint32_to_void(uint32_t *ptr) { return (void*) ptr; }
static inline void *ts_int32_to_void(int32_t *ptr) { return (void*) ptr; }
static inline void *ts_uint16_to_void(uint16_t *ptr) { return (void*) ptr; }
static inline void *ts_int16_to_void(int16_t *ptr) { return (void*) ptr; }
static inline void *ts_float_to_void(float *ptr) { return (void*) ptr; }
static inline void *ts_string_to_void(const char *ptr) { return (void*) ptr; }
static inline void *ts_bytes_to_void(struct ts_bytes_buffer *ptr) { return (void *) ptr; }
static inline void *ts_function_to_void(void (*fnptr)()) { return (void*) fnptr; }
static inline void *ts_array_to_void(struct ts_array *ptr) { return (void *) ptr; }
#else
#define ts_bool_to_void(ptr) ((void*)ptr)
#define ts_uint64_to_void(ptr) ((void*)ptr)
#define ts_int64_to_void(ptr) ((void*)ptr)
#define ts_uint32_to_void(ptr) ((void*)ptr)
#define ts_int32_to_void(ptr) ((void*)ptr)
#define ts_uint16_to_void(ptr) ((void*)ptr)
#define ts_int16_to_void(ptr) ((void*)ptr)
#define ts_float_to_void(ptr) ((void*)ptr)
#define ts_string_to_void(ptr) ((void*)ptr)
#define ts_bytes_to_void(ptr) ((void*)ptr)
#define ts_function_to_void(ptr) ((void*)ptr)
#define ts_array_to_void(ptr) ((void*)ptr)
#endif

/** @endcond */

/** Create data item for bool variable. */
#define TS_ITEM_BOOL(id, name, bool_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_bool_to_void(bool_ptr), TS_T_BOOL, 0, access, subsets}

/** Create data item for uint64_t variable. */
#define TS_ITEM_UINT64(id, name, uint64_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_uint64_to_void(uint64_ptr), TS_T_UINT64, 0, access, subsets}

/** Create data item for int64_t variable. */
#define TS_ITEM_INT64(id, name, int64_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_int64_to_void(int64_ptr), TS_T_INT64, 0, access, subsets}

/** Create data item for uint32_t variable. */
#define TS_ITEM_UINT32(id, name, uint32_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_uint32_to_void(uint32_ptr), TS_T_UINT32, 0, access, subsets}

/** Create data item for int32_t variable. */
#define TS_ITEM_INT32(id, name, int32_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_int32_to_void(int32_ptr), TS_T_INT32, 0, access, subsets}

/** Create data item for uint16_t variable. */
#define TS_ITEM_UINT16(id, name, uint16_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_uint16_to_void(uint16_ptr), TS_T_UINT16, 0, access, subsets}

/** Create data item for int16_t variable. */
#define TS_ITEM_INT16(id, name, int16_ptr, parent_id, access, subsets) \
    {id, parent_id, name, ts_int16_to_void(int16_ptr), TS_T_INT16, 0, access, subsets}

/** Create data item for float variable. */
#define TS_ITEM_FLOAT(id, name, float_ptr, digits, parent_id, access, subsets) \
    {id, parent_id, name, ts_float_to_void(float_ptr), TS_T_FLOAT32, digits, access, subsets}

/**
 * Create data item for decimal fraction variable. The mantissa is internally stored as int32_t.
 * The value is cnverted into a float (JSON) or decimal fraction type (CBOR) for the protocol,
 * based on the specified (fixed) exponent.
 */
#define TS_ITEM_DECFRAC(id, name, mantissa_ptr, exponent, parent_id, access, subsets) \
    {id, parent_id, name, ts_int32_to_void(mantissa_ptr), TS_T_DECFRAC, exponent, access, subsets}

/**
 * Create data item for a string buffer. The string must be null-terminated and buf_size contains
 * the maximum length of the buffer including the null-termination character.
 */
#define TS_ITEM_STRING(id, name, char_ptr, buf_size, parent_id, access, subsets) \
    {id, parent_id, name, ts_string_to_void(char_ptr), TS_T_STRING, buf_size, access, subsets}

/**
 * Create data item for a byte buffer containing arbitrary binary data. In contrast to string
 * buffers, no null-termination is used.
 */
#define TS_ITEM_BYTES(id, name, bytes_buffer_ptr, buf_size, parent_id, access, subsets) \
    {id, parent_id, name, ts_bytes_to_void(bytes_buffer_ptr), TS_T_BYTES, buf_size, access, subsets}

/** Create an executable data object for function calls. */
#define TS_FUNCTION(id, name, void_function_ptr, parent_id, access) \
    {id, parent_id, name, ts_function_to_void(void_function_ptr), TS_T_EXEC, 0, access, 0}

/** Create a data object pointing to a struct ts_array. */
#define TS_ITEM_ARRAY(id, name, array_info_ptr, digits, parent_id, access, subsets) \
    {id, parent_id, name, ts_array_to_void(array_info_ptr), TS_T_ARRAY, digits, access, subsets}

/** Create a subset data object for the provided subset flag. */
#define TS_SUBSET(id, name, subset, parent_id, access) \
    {id, parent_id, name, NULL, TS_T_SUBSET, subset, access, 0}

/** Create a group for hierarchical structuring of the data. */
#define TS_GROUP(id, name, void_function_cb_ptr, parent_id) \
    {id, parent_id, name, ts_function_to_void(void_function_cb_ptr), TS_T_GROUP, 0, TS_READ_MASK, 0}

/** @cond INTERNAL_HIDDEN */
/*
 * Deprecated defines for spec v0.3 to maintain compatibility
 */

#define TS_NODE_BOOL(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_BOOL(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_BOOL' macro is deprecated, use 'TS_ITEM_BOOL'\"")

#define TS_NODE_UINT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_UINT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_UINT64' macro is deprecated, use 'TS_ITEM_UINT64'\"")

#define TS_NODE_INT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_INT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_INT64' macro is deprecated, use 'TS_ITEM_INT64'\"")

#define TS_NODE_UINT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_UINT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_UINT32' macro is deprecated, use 'TS_ITEM_UINT32'\"")

#define TS_NODE_INT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_INT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_INT32' macro is deprecated, use 'TS_ITEM_INT32'\"")

#define TS_NODE_UINT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_UINT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_UINT16' macro is deprecated, use 'TS_ITEM_UINT16'\"")

#define TS_NODE_INT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    TS_ITEM_INT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_INT16' macro is deprecated, use 'TS_ITEM_INT16'\"")

#define TS_NODE_FLOAT(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    TS_ITEM_FLOAT(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_FLOAT' macro is deprecated, use 'TS_ITEM_FLOAT'\"")

#define TS_NODE_STRING(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    TS_ITEM_STRING(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_STRING' macro is deprecated, use 'TS_ITEM_STRING'\"")

#define TS_NODE_BYTES(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    TS_ITEM_BYTES(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_BYTES' macro is deprecated, use 'TS_ITEM_BYTES'\"")

#define TS_NODE_EXEC(_id, _name, _function_ptr, _parent, _acc) \
    TS_FUNCTION(_id, _name, _function_ptr, _parent, _acc) \
    _Pragma ("GCC warning \"'TS_NODE_EXEC' macro is deprecated, use 'TS_FUNCTION'\"")

#define TS_NODE_ARRAY(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    TS_ITEM_ARRAY(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    _Pragma ("GCC warning \"'TS_NODE_ARRAY' macro is deprecated, use 'TS_ITEM_ARRAY'\"")

#define TS_NODE_PUBSUB(_id, _name, _pubsub_channel, _parent, _acc, _pubsub) \
    TS_SUBSET(_id, _name, _pubsub_channel, _parent, _acc) \
    _Pragma ("GCC warning \"'TS_NODE_PUBSUB' macro is deprecated, use 'TS_SUBSET'\"")

#define TS_NODE_PATH(_id, _name, _parent, _callback) \
    TS_GROUP(_id, _name, _callback, _parent) \
    _Pragma ("GCC warning \"'TS_NODE_PATH' macro is deprecated, use 'TS_GROUP'\"")

/** @endcond */

/*
 * Defines to make data object definitions more explicit
 */
#define TS_NO_CALLBACK  NULL    /**< No callback assigned to group */

/*
 * Access right macros for data objects
 */

/** @cond INTERNAL_HIDDEN */

#define TS_ROLE_USR     (1U << 0)       /**< Normal user */
#define TS_ROLE_EXP     (1U << 1)       /**< Expert user */
#define TS_ROLE_MKR     (1U << 2)       /**< Maker */

#define TS_READ_MASK    0x0F          /**< Read flags stored in 4 least-significant bits */
#define TS_WRITE_MASK   0xF0          /**< Write flags stored in 4 most-significant bits */

#define TS_USR_MASK     (TS_ROLE_USR << 4 | TS_ROLE_USR) /**< Mask for normal user role */
#define TS_EXP_MASK     (TS_ROLE_EXP << 4 | TS_ROLE_EXP) /**< Mask for expert user role */
#define TS_MKR_MASK     (TS_ROLE_MKR << 4 | TS_ROLE_MKR) /**< Mask for maker role */

#define TS_READ(roles)          ((roles) & TS_READ_MASK)
#define TS_WRITE(roles)         (((roles) << 4) & TS_WRITE_MASK)
#define TS_READ_WRITE(roles)    (TS_READ(roles) | TS_WRITE(roles))

/** @endcond */

#define TS_USR_R        TS_READ(TS_ROLE_USR) /**< Read-only access for normal user */
#define TS_EXP_R        TS_READ(TS_ROLE_EXP) /**< Read-only access for expert user */
#define TS_MKR_R        TS_READ(TS_ROLE_MKR) /**< Read-only access for maker */
#define TS_ANY_R        (TS_USR_R | TS_EXP_R | TS_MKR_R) /**< Read-only access for any user */

#define TS_USR_W        TS_WRITE(TS_ROLE_USR) /**< Write-only access for normal user */
#define TS_EXP_W        TS_WRITE(TS_ROLE_EXP) /**< Write-only access for expert user */
#define TS_MKR_W        TS_WRITE(TS_ROLE_MKR) /**< Write-only access for maker */
#define TS_ANY_W        (TS_USR_W | TS_EXP_W | TS_MKR_W) /**< Write-only access for any user */

#define TS_USR_RW       TS_READ_WRITE(TS_ROLE_USR) /**< Read/write access for normal user */
#define TS_EXP_RW       TS_READ_WRITE(TS_ROLE_EXP) /**< Read/write access for expert user */
#define TS_MKR_RW       TS_READ_WRITE(TS_ROLE_MKR) /**< Read/write access for maker */
#define TS_ANY_RW       (TS_USR_RW | TS_EXP_RW | TS_MKR_RW) /**< Read/write access for any user */

/** ThingSet data object ID (16-bit) */
typedef uint16_t ts_object_id_t;

/** @cond INTERNAL_HIDDEN */

/** Support for legacy code with old nomenclature. Node ID is now called Object ID */
typedef ts_object_id_t ts_node_id_t __attribute__((deprecated));

/** @endcond */

/**
 * ThingSet data object struct.
 */
struct ts_data_object {
    /**
     * Data object ID
     */
    const ts_object_id_t id;

    /**
     * ID of parent object
     */
    const ts_object_id_t parent;

    /**
     * Data object name
     */
    const char *name;

    /**
     * Pointer to the variable containing the data. The variable type must match the type as
     * specified
     */
    void *const data;

    /**
     * One of TS_TYPE_INT32, _FLOAT, ...
     */
    const uint32_t type : 4;

    /**
     * Variable storing different detail information depending on the data type
     *
     * - FLOAT32: Decimal digits (precision) to use during serialization to JSON.
     *
     * - DECFRAC: Exponent (10^exponent = factor to convert to internal unit). Example: If
     *   a voltage measurement is internally stored as an integer in mV, use exponent -3 to
     *   convert to the SI base unit V as exposed via ThingSet.
     *
     * - STRING or BYTES: Size of the internal buffer in bytes.
     */
    const int32_t detail : 12;

    /**
     * Flags to define read/write access
     */
    const uint32_t access : 8;

    /**
     * Flags to assign data item to different data item subsets (e.g. for publication messages)
     */
    uint32_t subsets : 8;

};

/** @cond INTERNAL_HIDDEN */

/* support for legacy code with old nomenclature */
typedef struct ts_data_object ts_data_node __attribute__((deprecated));

/** @endcond */

/**
 * ThingSet context.
 *
 * Stores and handles all data objects exposed to different communication interfaces.
 */
struct ts_context {
    /**
     * Array of objects database provided during initialization
     */
    struct ts_data_object *data_objects;

    /**
     * Number of objects in the data_objects array
     */
    size_t num_objects;

    /**
     * Pointer to request buffer (provided in process function)
     */
    const uint8_t *req;

    /**
     * Length of the request
     */
    size_t req_len;

    /**
     * Pointer to response buffer (provided in process function)
     */
    uint8_t *resp;

    /**
     * Size of response buffer (i.e. maximum length)
     */
    size_t resp_size;

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

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t _auth_flags;

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t _update_subsets;

    /**
     * Callback to be called from patch function if a value belonging to _update_subsets
     * was changed
     */
    void (*update_cb)(void);
};

/**
 * Initialize a ThingSet context.
 *
 * @param ts Pointer to ThingSet context.
 * @param data Pointer to array of ThingSetDataObject type containing the entire object database
 * @param num Number of elements in that array
 */
int ts_init(struct ts_context *ts, struct ts_data_object *data, size_t num);

/**
 * Process ThingSet request.
 *
 * This function also detects if JSON or CBOR format is used
 *
 * @param ts Pointer to ThingSet context.
 * @param request Pointer to the ThingSet request buffer
 * @param request_len Length of the data in the request buffer
 * @param response Pointer to the buffer where the ThingSet response should be stored
 * @param response_size Size of the response buffer, i.e. maximum allowed length of the response
 *
 * @returns Actual length of the response written to the buffer or 0 in case of error or if no
 *          response message has been generated (e.g. because a statement was processed)
 */
int ts_process(struct ts_context *ts, const uint8_t *request, size_t request_len,
               uint8_t *response, size_t response_size);

/**
 * Print all data objects as a structured JSON text to stdout.
 *
 * WARNING: This is a recursive function and might cause stack overflows if run in constrained
 *          devices with large data object tree. Use with care and for testing only!
 *
 * @param ts Pointer to ThingSet context.
 * @param obj_id Root object ID where to start with printing
 * @param level Indentation level (=depth inside the data object tree)
 */
void ts_dump_json(struct ts_context *ts, ts_object_id_t obj_id, int level);

/**
 * Sets current authentication level.
 *
 * The authentication flags must match with access flags specified in ThingSetDataObject to allow
 * read/write access to a data object.
 *
 * @param ts Pointer to ThingSet context.
 * @param flags Flags to define authentication level (1 = access allowed)
 */
void ts_set_authentication(struct ts_context *ts, uint8_t flags);

/**
 * Configures a callback for notification if data belonging to specified subset(s) was updated.
 *
 * @param ts Pointer to ThingSet context.
 * @param subsets Flags to select which subset(s) of data items should be considered
 * @param update_cb Callback to be called after an update.
 */
void ts_set_update_callback(struct ts_context *ts, const uint16_t subsets, void (*update_cb)(void));

/**
 * Retrieve data in JSON format for given subset(s).
 *
 * This function does not return a complete ThingSet message, but only the payload data as a
 * name/value map. It can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param subsets Flags to select which subset(s) of data items should be exported
 *
 * @returns Actual length of the data written to the buffer or 0 in case of error
 */
int ts_txt_export(struct ts_context *ts, char *buf, size_t buf_size, uint16_t subsets);

/**
 * Generate statement message in JSON format based on pointer to group or subset.
 *
 * This is the fastest method to generate a statement as it does not require to search through the
 * entire data objects array.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param object Group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_txt_statement(struct ts_context *ts, char *buf, size_t buf_size,
                     struct ts_data_object *object);

/**
 * Generate statement message in JSON format based on path.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path Path to group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_txt_statement_by_path(struct ts_context *ts, char *buf, size_t buf_size, const char *path);

/**
 * Generate statement message in JSON format based on data object ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param id ID of group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_txt_statement_by_id(struct ts_context *ts, char *buf, size_t buf_size, ts_object_id_t id);

/**
 * Retrieve data in CBOR format for given subset(s).
 *
 * This function does not return a complete ThingSet message, but only the payload data as an
 * ID/value map. It can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param subsets Flags to select which subset(s) of data items should be exported
 *
 * @returns Actual length of the data written to the buffer or 0 in case of error
 */
int ts_bin_export(struct ts_context *ts, uint8_t *buf, size_t buf_size, uint16_t subsets);

/**
 * Generate statement message in CBOR format based on pointer to group or subset.
 *
 * This is the fastest method to generate a statement as it does not require to search through the
 * entire date nodes array.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param object Group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_bin_statement(struct ts_context *ts, uint8_t *buf, size_t buf_size,
                     struct ts_data_object *object);

/**
 * Generate statement message in CBOR format based on path.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path Path to group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_bin_statement_by_path(struct ts_context *ts, uint8_t *buf, size_t buf_size,
                             const char *path);

/**
 * Generate statement message in CBOR format based on data object ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param id ID of group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int ts_bin_statement_by_id(struct ts_context *ts, uint8_t *buf, size_t buf_size, ts_object_id_t id);

/**
 * Encode a publication message in CAN message format for supplied data object.
 *
 * The data may only be 8 bytes long. If the actual length of a object exceeds the available
 * length, the object is silently ignored and the function continues with the next one.
 *
 * @param ts Pointer to ThingSet context.
 * @param start_pos Position in data_objects array to start searching
 *                  This value is updated with the next object found to allow iterating over all
 *                  objects for this channel. It should be set to 0 to start from the beginning.
 * @param subset Flag to select which subset of data items should be published
 * @param can_dev_id Device ID on the CAN bus
 * @param msg_id reference to can message id storage
 * @param msg_data reference to the buffer where the publication message should be stored
 *
 * @returns Actual length of the message_data or -1 if not encodable / in case of error
 */
int ts_bin_pub_can(struct ts_context *ts, int *start_pos, uint16_t subset, uint8_t can_dev_id,
                   uint32_t *msg_id, uint8_t *msg_data);

/**
 * Import data in CBOR format into data objects.
 *
 * This function can be used to initialize data objects from previously exported data (using
 * ts_bin_export function) and stored in the EEPROM or other non-volatile memory.
 *
 * @param ts Pointer to ThingSet context.
 * @param data Buffer containing ID/value map that should be written to the data objects
 * @param len Length of the data in the buffer
 * @param auth_flags Authentication flags to be used in this function (to override _auth_flags)
 * @param subsets Flags to select which subset(s) of data items should be imported
 *
 * @returns ThingSet status code
 */
int ts_bin_import(struct ts_context *ts, uint8_t *data, size_t len, uint8_t auth_flags,
                  uint16_t subsets);

/**
 * Get data object by ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param id Data object ID
 *
 * @returns Pointer to data object or NULL if object is not found
 */
struct ts_data_object *ts_get_object_by_id(struct ts_context *ts, ts_object_id_t id);

/**
 * Get data object by name.
 *
 * As the names are not necessarily unique in the entire data tree, the parent is needed
 *
 * @param ts Pointer to ThingSet context.
 * @param name Data object name
 * @param len Length of the object name
 * @param parent Data object ID of the parent or -1 for global search
 *
 * @returns Pointer to data object or NULL if object is not found
 */
struct ts_data_object *ts_get_object_by_name(struct ts_context *ts, const char *name, size_t len,
                                             int32_t parent);

/**
 * Get data object by path.
 *
 * Get the endpoint object of a provided path.
 *
 * @param ts Pointer to ThingSet context.
 * @param path Path with multiple object names separated by forward slash.
 * @param len Length of the entire path
 *
 * @returns Pointer to data object or NULL if object is not found
 */
struct ts_data_object *ts_get_object_by_path(struct ts_context *ts, const char *path, size_t len);

#ifdef __cplusplus

/* Provide C++ naming for C constructs. */
typedef ts_object_id_t ThingSetObjId;
typedef struct ts_bytes_buffer ThingSetBytesBuffer;
typedef struct ts_array ThingSetArrayInfo;
typedef struct ts_data_object ThingSetDataObject;
typedef struct ts_context ThingSetContext;

#if CONFIG_THINGSET_CPP_LEGACY
/* compatibility to legacy CPP interface */
typedef ThingSetBytesBuffer TsBytesBuffer __attribute__((deprecated));
typedef ThingSetArrayInfo ArrayInfo __attribute__((deprecated));
typedef ThingSetDataObject DataNode __attribute__((deprecated));
#endif

} /* extern 'C' */

/**
 * Main ThingSet class.
 *
 * Class Thingset is a C++ shim for the C implementation of ThingSet.
 * See the respective C functions for a detailed description.
 */
class ThingSet
{
public:

    inline ThingSet(ThingSetDataObject *data, size_t num)
    {
        (void)ts_init(&ts, data, num);
    };

    inline int process(uint8_t *request, size_t req_len, uint8_t *response, size_t resp_size)
    {
        return ts_process(&ts, request, req_len, response, resp_size);
    };

    inline void dump_json(ts_object_id_t obj_id = 0, int level = 0)
    {
        ts_dump_json(&ts, obj_id, level);
    };

    inline void set_authentication(uint8_t flags)
    {
        ts_set_authentication(&ts, flags);
    };

    inline void set_update_callback(const uint16_t subsets, void (*update_cb)(void))
    {
        ts_set_update_callback(&ts, subsets, update_cb);
    };

    inline int txt_export(char *buf, size_t size, const uint16_t subsets)
    {
        return ts_txt_export(&ts, buf, size, subsets);
    };

    inline int txt_statement(char *buf, size_t size, ThingSetDataObject *object)
    {
        return ts_txt_statement(&ts, buf, size, object);
    };

    inline int txt_statement(char *buf, size_t size, const char *path)
    {
        return ts_txt_statement_by_path(&ts, buf, size, path);
    };

    inline int txt_statement(char *buf, size_t size, ThingSetObjId id)
    {
        return ts_txt_statement_by_id(&ts, buf, size, id);
    };

    inline int bin_export(uint8_t *buf, size_t size, const uint16_t subsets)
    {
        return ts_bin_export(&ts, buf, size, subsets);
    };

    inline int bin_import(uint8_t *buf, size_t size, uint8_t auth_flags, const uint16_t subsets)
    {
        return ts_bin_import(&ts, buf, size, auth_flags, subsets);
    };

    inline int bin_statement(uint8_t *buf, size_t size, ThingSetDataObject *object)
    {
        return ts_bin_statement(&ts, buf, size, object);
    };

    inline int bin_statement(uint8_t *buf, size_t size, const char *path)
    {
        return ts_bin_statement_by_path(&ts, buf, size, path);
    };

    inline int bin_statement(uint8_t *buf, size_t size, ThingSetObjId id)
    {
        return ts_bin_statement_by_id(&ts, buf, size, id);
    };

    inline int bin_pub_can(int &start_pos, uint16_t subset, uint8_t can_dev_id, uint32_t &msg_id,
                           uint8_t (&msg_data)[8])
    {
        return ts_bin_pub_can(&ts, &start_pos, subset, can_dev_id, &msg_id, &msg_data[0]);
    };

    inline ThingSetDataObject *get_object(ThingSetObjId id)
    {
        return ts_get_object_by_id(&ts, id);
    };

    inline ThingSetDataObject *get_object(const char *name, size_t len, int32_t parent = -1)
    {
        return ts_get_object_by_name(&ts, name, len, parent);
    };

    inline ThingSetDataObject *get_endpoint(const char *path, size_t len)
    {
        return ts_get_object_by_path(&ts, path, len);
    };

    /*
     * Deprecated functions from ThingSet v0.3 interface
     */

    /**
     * Generate statement (previously known as publication message) in JSON format.
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param subset Flag to select which subset of data items should be published
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    inline int txt_pub(char *buf, size_t size, const uint16_t subset)
        __attribute__((deprecated))
    {
        buf[0] = '#';
        buf[1] = ' ';
        int ret = ts_txt_export(&ts, &buf[2], size - 2, subset);
        return (ret > 0) ? 2 + ret : 0;
    };

    /**
     * Generate statement (previously known as publication message) in CBOR format.
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param subset Flag to select which subset of data items should be published
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    inline int bin_pub(uint8_t *buf, size_t size, const uint16_t subset)
        __attribute__((deprecated))
    {
        buf[0] = TS_STATEMENT;
        int ret = ts_bin_export(&ts, &buf[1], size - 1, subset);
        return (ret > 0) ? 1 + ret : 0;
    };

    /**
     * Update data objects based on values provided by from other pub msg.
     *
     * @param buf Buffer containing pub message and data that should be written to the data objects
     * @param len Length of the data in the buffer
     * @param auth_flags Authentication flags to be used in this function (to override _auth_flags)
     * @param subsets Subscribe channel (as bitfield)
     *
     * @returns ThingSet status code
     */
    inline int bin_sub(uint8_t *buf, size_t len, uint8_t auth_flags, uint16_t subsets)
        __attribute__((deprecated))
    {
        return ts_bin_import(&ts, buf + 1, len - 1, auth_flags, subsets);
    };

private:

    ThingSetContext ts;
};

#endif /* __cplusplus */

#endif /* THINGSET_H_ */
