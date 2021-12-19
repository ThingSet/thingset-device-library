/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet data objects (public interface)
 */

#ifndef THINGSET_OBJ_H_
#define THINGSET_OBJ_H_

/**
 * @brief ThingSet data objects.
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_obj_api_pub ThingSet data objects (public interface)
 * @{
 */

#include "thingset_env.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Reserved data object IDs
 */
#define TS_ID_ROOT              0x0000U
#define TS_ID_TIME              0x0010U
#define TS_ID_NAME              0x0017U
#define TS_ID_METADATAURL       0x0018U
#define TS_ID_DEVICEID          0x001DU
#define TS_ID_INVALID           0xFFFFU

/**
 * @brief ThingSet data objects C types (used to cast void* pointers)
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
    TS_T_GROUP,         // internal object to describe data hierarchy
    TS_T_EXEC,          // functions
    TS_T_SUBSET,
    TS_T_INVALID
};

/**
 * Data structure to specify a binary data buffer
 */
struct ts_bytes_buffer {
    uint8_t *bytes;             ///< Pointer to the buffer
    uint16_t num_bytes;         ///< Actual number of bytes in the buffer
};

/**
 * Data structure to specify an array data object
 */
struct ts_array_info {
    void *ptr;                  ///< Pointer to the array
    uint16_t max_elements;      ///< Maximum number of elements in the array
    uint16_t num_elements;      ///< Actual number of elements in the array
    uint8_t type;               ///< Type of the array elements
};

/**
 * @brief ThingSet exec function pointer type.
 */
typedef void(*ts_obj_exec_t)(void);

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
static inline void *ts_array_to_void(struct ts_array_info *ptr) { return (void *) ptr; }

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

/**
 * @def TS_OBJ
 *
 * Initialise ThingSet database data object.
 */
#if !defined(__cplusplus) || (__cplusplus == 202002)
/* Designated initializer are a feature of C++20 and C in general */
#define TS_OBJ(obj_id, obj_parent_id, obj_name, obj_data_ptr, obj_type, obj_detail, obj_access, \
               obj_subsets)                                                                     \
    {   .id = obj_id,                                                                           \
        .type = obj_type,                                                                       \
        .name = obj_name,                                                                       \
        .parent = obj_parent_id,                                                                \
        .data = obj_data_ptr,                                                                   \
        .meta_default.db_id = 0,                                                                \
        .meta_default.subsets = obj_subsets,                                                    \
        .meta_default.detail = obj_detail,                                                      \
        .meta_default.access = obj_access                                                       \
    }
#else
#define TS_OBJ(obj_id, obj_parent_id, obj_name, obj_data_ptr, obj_type, obj_detail, obj_access, \
               obj_subsets)                                                                     \
    {   obj_id,                                                                                 \
        obj_type,                                                                               \
        obj_name,                                                                               \
        obj_parent_id,                                                                          \
        obj_data_ptr,                                                                           \
        0,                                                                                      \
        obj_subsets,                                                                            \
        obj_detail,                                                                             \
        obj_access                                                                              \
    }
#endif

#define TS_ITEM_BOOL(id, name, bool_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_bool_to_void(bool_ptr), TS_T_BOOL, 0, access, subsets)

#define TS_ITEM_UINT64(id, name, uint64_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_uint64_to_void(uint64_ptr), TS_T_UINT64, 0, access, subsets)

#define TS_ITEM_INT64(id, name, int64_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_int64_to_void(int64_ptr), TS_T_INT64, 0, access, subsets)

#define TS_ITEM_UINT32(id, name, uint32_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_uint32_to_void(uint32_ptr), TS_T_UINT32, 0, access, subsets)

#define TS_ITEM_INT32(id, name, int32_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_int32_to_void(int32_ptr), TS_T_INT32, 0, access, subsets)

#define TS_ITEM_UINT16(id, name, uint16_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_uint16_to_void(uint16_ptr), TS_T_UINT16, 0, access, subsets)

#define TS_ITEM_INT16(id, name, int16_ptr, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_int16_to_void(int16_ptr), TS_T_INT16, 0, access, subsets)

#define TS_ITEM_FLOAT(id, name, float_ptr, digits, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_float_to_void(float_ptr), TS_T_FLOAT32, digits, access, subsets)

#define TS_ITEM_DECFRAC(id, name, mantissa_ptr, exponent, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_int32_to_void(mantissa_ptr), TS_T_DECFRAC, exponent, access, subsets)

#define TS_ITEM_STRING(id, name, char_ptr, buf_size, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_string_to_void(char_ptr), TS_T_STRING, buf_size, access, subsets)

#define TS_ITEM_BYTES(id, name, bytes_buffer_ptr, buf_size, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_bytes_to_void(bytes_buffer_ptr), TS_T_BYTES, buf_size, access, subsets)

#define TS_FUNCTION(id, name, void_function_ptr, parent_id, access) \
    TS_OBJ(id, parent_id, name, ts_function_to_void(void_function_ptr), TS_T_EXEC, 0, access, 0)

#define TS_ITEM_ARRAY(id, name, array_info_ptr, digits, parent_id, access, subsets) \
    TS_OBJ(id, parent_id, name, ts_array_to_void(array_info_ptr), TS_T_ARRAY, digits, access, subsets)

#define TS_SUBSET(id, name, subset, parent_id, access) \
    TS_OBJ(id, parent_id, name, NULL, TS_T_SUBSET, subset, access, 0)

#define TS_GROUP(id, name, void_function_cb_ptr, parent_id) \
    TS_OBJ(id, parent_id, name, ts_function_to_void(void_function_cb_ptr), TS_T_GROUP, 0, TS_ANY_R, 0)

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

/*
 * Defines to make data object definitions more explicit
 */
#define TS_NO_CALLBACK  NULL

/*
 * Access rights macros for data objects
 */
#define TS_ROLE_USR     (1U << 0)       // normal user
#define TS_ROLE_EXP     (1U << 1)       // expert user
#define TS_ROLE_MKR     (1U << 2)       // maker

#define TS_USR_MASK     (TS_ROLE_USR << 8 | TS_ROLE_USR)
#define TS_EXP_MASK     (TS_ROLE_EXP << 8 | TS_ROLE_EXP)
#define TS_MKR_MASK     (TS_ROLE_MKR << 8 | TS_ROLE_MKR)

/**
 * @brief Read access flags mask.
 *
 * Read flags are stored in the 8 least-significant bits.
 */
#define TS_READ_MASK    (0x00FF & (TS_USR_MASK | TS_EXP_MASK | TS_MKR_MASK))

/**
 * @brief Write access flags mask.
 *
 * Write flags are stored in the 8 most-significant bits.
 */
#define TS_WRITE_MASK   (0xFF00 & (TS_USR_MASK | TS_EXP_MASK | TS_MKR_MASK))

#define TS_USR_R        (TS_READ_MASK & TS_USR_MASK)
#define TS_EXP_R        (TS_READ_MASK & TS_EXP_MASK)
#define TS_MKR_R        (TS_READ_MASK & TS_MKR_MASK)
#define TS_ANY_R        (TS_READ_MASK)

#define TS_USR_W        (TS_WRITE_MASK & TS_USR_MASK)
#define TS_EXP_W        (TS_WRITE_MASK & TS_EXP_MASK)
#define TS_MKR_W        (TS_WRITE_MASK & TS_MKR_MASK)
#define TS_ANY_W        (TS_WRITE_MASK)

#define TS_USR_RW       (TS_USR_R | TS_USR_W)
#define TS_EXP_RW       (TS_EXP_R | TS_EXP_W)
#define TS_MKR_RW       (TS_MKR_R | TS_MKR_W)
#define TS_ANY_RW       (TS_ANY_R | TS_ANY_W)

/** @brief ThingSet data objects database ID type */
typedef uint16_t ts_obj_db_id_t;

/** @brief ThingSet data object ID type */
typedef uint16_t ts_obj_id_t;

/* support for legacy code with old nomenclature */
typedef ts_obj_id_t ts_ctx_node_id_t __attribute__((deprecated));

/**
 * @brief Struct for the mutable meta data of a ThingSet data object.
 */
struct ts_obj_meta {
    /** @brief Data objects database ID of database the object belongs to. */
    ts_obj_db_id_t db_id;

    /**
     * @brief Flags to assign data item to different data item subsets.
     *
     * May be used for e.g. for publication messages.
     */
    uint16_t subsets;

    /**
     * @brief Object type dependent details.
     *
     * - Exponent (10^exponent = factor to convert to SI unit) for decimal fraction type,
     * - Decimal digits to use for printing of floats in JSON strings or
     * - Length of string buffer for string type
     */
    int16_t detail;

    /** @brief Flags to define read/write access */
    uint16_t access;
};

/**
 * @brief ThingSet data object struct.
 */
struct ts_obj {
    /** @brief ThingSet data object ID of data object. */
    ts_obj_id_t id;

    /**
     * @brief Type of data object.
     *
     * One of TS_TYPE_INT32, _FLOAT, ...
     */
    uint8_t type;

    /** @brief Name of data object */
    const char *name;

    /** @brief ThingSet data object ID of parent data object. */
    ts_obj_id_t parent;

    /**
     * @brief Pointer to the variable containing the data.
     *
     * The variable type must match the type as specified.
     */
    void *data;

    /** @brief Default value for mutable meta data of data object. */
    struct ts_obj_meta meta_default;
};

/**
 * @brief ThingSet data objects database structure.
 */
struct ts_obj_db {
    /**
     * @brief ID of data objects database.
     *
     * Identifier for the database on this device.
     */
    ts_obj_db_id_t id;

    /** @brief Unique context identifier of the context the database belongs to. */
    const uint64_t *uuid;

    /** @brief Array of data objects. */
    const struct ts_obj *objects;

    /** @brief Array of mutable meta data of data objects. */
    struct ts_obj_meta *meta;

    /** @brief Number of objects in the data_objects array */
    size_t num;

    /**
     * @brief Additional data for remote context objects database.
     *
     * This is NULL for a local context objects database.
     */
    struct ts_obj_rdb *rdb;
};

/* support for legacy code with old nomenclature */
typedef struct ts_obj ts_data_node __attribute__((deprecated));

/**
 * @def THINGSET_DATABASE_DEFINE
 *
 * @brief Define a ThingSet data objects database for local context data objects.
 *
 * Use like this:
 *
 *     THINGSET_DATABASE_DEFINE(THINGSET_LOCID_DEFAULT, 0x123456789012345ULL,
 *         TS_GROUP(ID_INFO, "info", TS_NO_CALLBACK, ID_ROOT),
 *         TS_ITEM_STRING(0x19, "Manufacturer", manufacturer, 0, I D_INFO, TS_ANY_R, 0)
 *     );
 *
 * @param locid Local context identifier. Paramter must expand to a positiv number.
 *              Usually 0. Limit defined by @ref TS_CONFIG_LOCAL_COUNT.
 * @param ctx_uuid Unique identifier for the context the database belongs to.
 * @param ... Initialisation for the data objects.
 */
#if !defined(__cplusplus) || (__cplusplus == 202002)
/* Designated initializer are a feature of C++20 and C in general */
#define THINGSET_DATABASE_DEFINE(locid, ctx_uuid, ...)                              \
    const struct ts_obj TS_CAT(ts_obj_db_objects_, locid)[] = { __VA_ARGS__ };      \
    struct ts_obj_meta TS_CAT(ts_obj_db_meta_, locid)                               \
                            [TS_ARRAY_SIZE(TS_CAT(ts_obj_db_objects_, locid))];     \
    const struct ts_obj_db TS_CAT(ts_obj_db_, locid) = {                            \
        .id = locid,                                                                \
        .uuid = &ctx_uuid,                                                          \
        .objects = &TS_CAT(ts_obj_db_objects_, locid)[0],                           \
        .meta = &TS_CAT(ts_obj_db_meta_, locid)[0],                                 \
        .num = TS_ARRAY_SIZE(TS_CAT(ts_obj_db_objects_, locid)),                    \
        .rdb = NULL                                                                 \
    }
#else
#define THINGSET_DATABASE_DEFINE(locid, ctx_uuid, ...)                              \
    const struct ts_obj TS_CAT(ts_obj_db_objects_, locid)[] = { __VA_ARGS__ };      \
    struct ts_obj_meta TS_CAT(ts_obj_db_meta_, locid)                               \
                            [TS_ARRAY_SIZE(TS_CAT(ts_obj_db_objects_, locid))];     \
    const struct ts_obj_db TS_CAT(ts_obj_db_, locid) = {                            \
        locid,                                                                      \
        &ctx_uuid,                                                                  \
        &TS_CAT(ts_obj_db_objects_, locid)[0],                                      \
        &TS_CAT(ts_obj_db_meta_, locid)[0],                                         \
        TS_ARRAY_SIZE(TS_CAT(ts_obj_db_objects_, locid)),                           \
        NULL                                                                        \
    }
#endif

/**
 * @brief ThingSet data objects database object identifier.
 */
typedef uint16_t ts_obj_db_oid_t;

/**
 * @brief ThingSet data objects database object reference.
 */
typedef struct ts_obj_db_oref {
    /** @brief Data objects database ID. */
    ts_obj_id_t db_id;
    /** @brief Data objects database object ID. */
    ts_obj_db_oid_t db_oid;
} thingset_oref_t;

/**
 * @brief Print all data objects as a structured JSON text to log buffer.
 *
 * @warning This is a recursive function and might cause stack overflows if run in constrained
 *          devices with large data object tree. Use with care and for testing only!
 *
 * @param[in] oref Database object reference to root object where to start with printing.
 * @param[in] log Pointer to the log buffer.
 * @param[in] len Length of the log buffer.
 */
void thingset_obj_log(thingset_oref_t oref, char *log, size_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

/**
 * @} <!-- ts_obj_api_pub -->
 */

#endif /* THINGSET_OBJ_H_ */
