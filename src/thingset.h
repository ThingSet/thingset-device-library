/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte
 */

#ifndef THINGSET_H_
#define THINGSET_H_

#if !CONFIG_THINGSET_CPP && !CONFIG_THINGSET_C
/* Default library implementation */
#define CONFIG_THINGSET_CPP 1
#endif

#include <stdint.h>
#include <stdbool.h>

#include "ts_config.h"

/*
 * Protocol function codes (same as CoAP)
 */
#define TS_GET      0x01
#define TS_POST     0x02
#define TS_DELETE   0x04
#define TS_FETCH    0x05
#define TS_PATCH    0x07        // it's actually iPATCH

#define TS_PUBMSG   0x1F

/*
 * Status codes (same as CoAP)
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
    TS_T_BOOL,
    TS_T_UINT64,
    TS_T_INT64,
    TS_T_UINT32,
    TS_T_INT32,
    TS_T_UINT16,
    TS_T_INT16,
    TS_T_FLOAT32,
    TS_T_STRING,
    TS_T_BYTES,
    TS_T_ARRAY,
    TS_T_DECFRAC,       // CBOR decimal fraction
    TS_T_PATH,          // internal node to describe URI path
    TS_T_NODE_ID,       // internally equal to uint16_t
    TS_T_EXEC,          // for exec data objects
    TS_T_PUBSUB
};

/**
 * Data structure to specify a binary data buffer
 */
typedef struct {
    uint8_t *bytes;             ///< Pointer to the buffer
    uint16_t num_bytes;         ///< Actual number of bytes in the buffer
} TsBytesBuffer;

/**
 * Data structure to specify an array data node
 */
typedef struct {
    void *ptr;                  ///< Pointer to the array
    uint16_t max_elements;      ///< Maximum number of elements in the array
    uint16_t num_elements;      ///< Actual number of elements in the array
    uint8_t type;               ///< Type of the array elements
} ArrayInfo;

/**
 * If TS_AUTODETECT_ARRLEN is assigned to num_elements, the number of elements in the array is
 * detected in the constructor by counting downwards till the first non-zero element is found.
 */
#define TS_AUTODETECT_ARRLEN    UINT16_MAX

/*
 * Functions to generate data_node map and make compiler complain if wrong
 * type is passed
 */

static inline void *_bool_to_void(bool *ptr) { return (void*) ptr; }
#define TS_NODE_BOOL(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _bool_to_void(_data_ptr), TS_T_BOOL, 0, _acc, _pubsub}

static inline void *_uint64_to_void(uint64_t *ptr) { return (void*) ptr; }
#define TS_NODE_UINT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _uint64_to_void(_data_ptr), TS_T_UINT64, 0, _acc, _pubsub}

static inline void *_int64_to_void(int64_t *ptr) { return (void*) ptr; }
#define TS_NODE_INT64(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _int64_to_void(_data_ptr), TS_T_INT64, 0, _acc, _pubsub}

static inline void *_uint32_to_void(uint32_t *ptr) { return (void*) ptr; }
#define TS_NODE_UINT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _uint32_to_void(_data_ptr), TS_T_UINT32, 0, _acc, _pubsub}

static inline void *_int32_to_void(int32_t *ptr) { return (void*) ptr; }
#define TS_NODE_INT32(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _int32_to_void(_data_ptr), TS_T_INT32, 0, _acc, _pubsub}

static inline void *_uint16_to_void(uint16_t *ptr) { return (void*) ptr; }
#define TS_NODE_UINT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _uint16_to_void(_data_ptr), TS_T_UINT16, 0, _acc, _pubsub}

static inline void *_int16_to_void(int16_t *ptr) { return (void*) ptr; }
#define TS_NODE_INT16(_id, _name, _data_ptr, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _int16_to_void(_data_ptr), TS_T_INT16, 0, _acc, _pubsub}

static inline void *_float_to_void(float *ptr) { return (void*) ptr; }
#define TS_NODE_FLOAT(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _float_to_void(_data_ptr), TS_T_FLOAT32, _digits, _acc, _pubsub}

static inline void *_string_to_void(const char *ptr) { return (void*) ptr; }
#define TS_NODE_STRING(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _string_to_void(_data_ptr), TS_T_STRING, _buf_size, _acc, _pubsub}

static inline void *_bytes_to_void(TsBytesBuffer *ptr) { return (void *) ptr; }
#define TS_NODE_BYTES(_id, _name, _data_ptr, _buf_size, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _bytes_to_void(_data_ptr), TS_T_BYTES, _buf_size, _acc, _pubsub}

static inline void *_function_to_void(void (*fnptr)()) { return (void*) fnptr; }
#define TS_NODE_EXEC(_id, _name, _function_ptr, _parent, _acc) \
    {_id, _parent, _name, _function_to_void(_function_ptr), TS_T_EXEC, 0, _acc, 0}

static inline void *_array_to_void(ArrayInfo *ptr) { return (void *) ptr; }
#define TS_NODE_ARRAY(_id, _name, _data_ptr, _digits, _parent, _acc, _pubsub) \
    {_id, _parent, _name, _array_to_void(_data_ptr), TS_T_ARRAY, _digits, _acc, _pubsub}

#define TS_NODE_PUBSUB(_id, _name, _pubsub_channel, _parent, _acc, _pubsub) \
    {_id, _parent, _name, NULL, TS_T_PUBSUB, _pubsub_channel, _acc, _pubsub}

#define TS_NODE_PATH(_id, _name, _parent, _callback) \
    {_id, _parent, _name, _function_to_void(_callback), TS_T_PATH, 0, TS_READ_MASK, 0}

/*
 * Access right macros for data nodes
 */
#define TS_ROLE_USR     (1U << 0)       // normal user
#define TS_ROLE_EXP     (1U << 1)       // expert user
#define TS_ROLE_MKR     (1U << 2)       // maker

#define TS_READ_MASK    0x00FF          // read flags stored in 4 least-significant bits
#define TS_WRITE_MASK   0xFF00          // write flags stored in 4 most-significant bits

#define TS_USR_MASK     (TS_ROLE_USR << 8 | TS_ROLE_USR)
#define TS_EXP_MASK     (TS_ROLE_EXP << 8 | TS_ROLE_EXP)
#define TS_MKR_MASK     (TS_ROLE_MKR << 8 | TS_ROLE_MKR)

#define TS_READ(roles)          ((roles) & TS_READ_MASK)
#define TS_WRITE(roles)         (((roles) << 8) & TS_WRITE_MASK)
#define TS_READ_WRITE(roles)    (TS_READ(roles) | TS_WRITE(roles))

#define TS_USR_R        TS_READ(TS_ROLE_USR)
#define TS_EXP_R        TS_READ(TS_ROLE_EXP)
#define TS_MKR_R        TS_READ(TS_ROLE_MKR)
#define TS_ANY_R        (TS_USR_R | TS_EXP_R | TS_MKR_R)

#define TS_USR_W        TS_WRITE(TS_ROLE_USR)
#define TS_EXP_W        TS_WRITE(TS_ROLE_EXP)
#define TS_MKR_W        TS_WRITE(TS_ROLE_MKR)
#define TS_ANY_W        (TS_USR_W | TS_EXP_W | TS_MKR_W)

#define TS_USR_RW       TS_READ_WRITE(TS_ROLE_USR)
#define TS_EXP_RW       TS_READ_WRITE(TS_ROLE_EXP)
#define TS_MKR_RW       TS_READ_WRITE(TS_ROLE_MKR)
#define TS_ANY_RW       (TS_USR_RW | TS_EXP_RW | TS_MKR_RW)


typedef uint16_t node_id_t;

/**
 * ThingSet data node struct
 */
typedef struct DataNode {
    /**
     * Data node ID
     */
    const node_id_t id;

    /**
     * ID of parent node
     */
    const node_id_t parent;

    /**
     * Data Node name
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
    const uint8_t type;

    /**
     * Exponent (10^exponent = factor to convert to SI unit) for decimal fraction type,
     * decimal digits to use for printing of floats in JSON strings or
     * lenght of string buffer for string type
     */
    const int16_t detail;

    /**
     * Flags to define read/write access
     */
    const uint16_t access;

    /**
     * Flags to add this node to different pub/sub channels
     */
    uint16_t pubsub;

} DataNode;


#if CONFIG_THINGSET_CPP
#include "thingset_cpp.h"
#endif

#endif /* THINGSET_H_ */
