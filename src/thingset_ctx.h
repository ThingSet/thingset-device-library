/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet local context (public interface)
 */

#ifndef THINGSET_CTX_H_
#define THINGSET_CTX_H_

/**
 * @brief Abstraction over ThingSet local context.
 *
 * The abstraction of a ThingSet local context provides a 'generic' local context that is
 * implemented by a specific variant of local context - core or communication.
 * There is the generic local context identifier that provides access to a local context without
 * actually knowing the exact variant of a context that is currently used. The local context
 * identifier is an unsigned integer value of type @ref thingset_locid_t.
 *
 * Access to the different variants of context is hiden behind the set of `generic` functions with
 * prefix `thingset_` that use the generic local context identifier and handle all variants of
 * contexts.
 *
 * Local contexts are defined with the help of special macros - @ref THINGSET_CORE_DEFINE and
 * @ref THINGSET_COM_DEFINE.
 *
 * - @ref THINGSET_CORE_DEFINE expects that just one core context exists per device. The local
 *        context identifier is given by @ref TS_CONFIG_CORE_LOCID, which by default is 0.
 * - @ref THINGSET_COM_DEFINE takes the local context identifier as a macro parameter that is
 *        expected to expand to an unsigned number.
 *
 * @defgroup ts_ctx_api_pub ThingSet local context (public interface)
 * @{
 */

#include "thingset_env.h"
#include "thingset_time.h"
#include "thingset_obj.h"
#include "thingset_msg.h"
#include "thingset_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @} <!-- ts_ctx_api_pub -->
 * @addtogroup ts_ctx_api_priv
 * @{
 */

/**
 * @brief ThingSet invalid context type identifier.
 */
#define TS_CTX_TYPE_INVALID 0

/**
 * @brief ThingSet core context type identifier.
 */
#define TS_CTX_TYPE_CORE    1

/**
 * @brief ThingSet communication context type identifier.
 */
#define TS_CTX_TYPE_COM     2

/**
 * @} <!-- ts_ctx_api_priv -->
 * @addtogroup ts_ctx_api_pub
 * @{
 */

/**
 * @def THINGSET_LOCID_DEFAULT
 *
 * @brief Default ThingSet local context identifier.
 */
#define THINGSET_LOCID_DEFAULT  0

/**
 * @brief ThingSet local context identifier.
 *
 * A local context identifier identifies a ThingSet context that is local to a device.
 *
 * The default local context identifier is 0.
 */
typedef uint8_t thingset_locid_t;

/**
 * @brief ThingSet unique context identifier.
 *
 * A unique context identifier identifies a specific ThingSet context. It shall be unique within all
 * contexts known to a device - local and remote ones.
 *
 * The unique context identifier may be equivalent to a device ID as exposed through the application
 * layer protocol. It may be translated to a communication port specific addressing (e.g. CAN ID).
 */
typedef uint64_t thingset_uid_t;

/**
 * @} <!-- ts_ctx_api_pub -->
 * @addtogroup ts_ctx_api_priv
 * @{
 */

/**
 * @brief Context node table element.
 *
 * Node information of other node known to this ThingSet context.
 */
struct ts_ctx_node {
    /** @brief Unique context identifier of this node. */
    thingset_uid_t ctx_uid;

    /**
     * @brief Reference to port (by port table entry) this node was seen last.
     */
    thingset_portid_t port_id;

    /**
     * @brief Time the node was last seen on the port.
     */
    thingset_time_ms_t last_seen_time;

    /**
     * @brief Reference to port (by port table entry) that responses from this node shall
     *        be routed to.
     */
    thingset_portid_t response_port_id;
};

/**
 * @brief Context node table.
 */
struct ts_ctx_node_table {
    /**
     * @brief Context node table elements.
     */
    struct ts_ctx_node nodes[TS_CONFIG_NODETABLE_SIZE];
};

/**
 * @brief Private data of a ThingSet context object.
 */
struct ts_ctx_data {
    /**
     * @brief Mutex for message processing.
     */
    pthread_mutex_t process_mutex;

    /**
     * @brief Communication protocol to use for application initiated communication.
     */
    bool app_protocol_use_bin;

    /**
     * @brief Current authorisation status (authorisation as "normal" user as default)
     */
    uint16_t _auth_flags;
};

/**
 * @brief Private data of a ThingSet core context object.
 */
struct ts_ctx_core_data {
    /**
     * @brief Data common to all context variants.
     *
     * Must be first to make context data types interchangeable
     */
    struct ts_ctx_data common;

    /**
     * @brief Pointer to response message buffer.
     *
     * Data is provided in thingset_process_buf().
     */
    uint8_t *resp_buf;

    /**
     * @brief Size of response message buffer.
     *
     * Data is provided in thingset_process_buf().
     */
    uint16_t resp_buf_size;
};

/**
 * @brief Private data of a ThingSet communication context object.
 */
struct ts_ctx_com_data {
    /**
     * @brief Data common to all context variants.
     *
     * Must be first to make context data types interchangeable
     */
    struct ts_ctx_data common;

    /**
     * @brief Message queue for message processing by context.
     */
    struct ts_impl_bufq process_bufq;

    /**
     * @brief Table of nodes known by this context.
     */
    struct ts_ctx_node_table node_table;
};

/**
 * @brief ThingSet generic local context.
 *
 * Runtime generic local context structure (in ROM) per context object.
 */
struct ts_ctx {
    /**
     * @brief Type of this local context.
     *
     * One of @p TS_CTX_TYPE_CORE or @p TS_CTX_TYPE_COM
     */
    uint16_t ctx_type;

    /** @brief Data objects database ID of this local context. */
    ts_obj_db_id_t db_id;

    /**
     * @brief Pointer to generic local context data.
     *
     * The pointer may be casted to the specific context data structure. That is why the specific
     * context data always have the common context data as it's first element.
     */
    struct ts_ctx_data *data;

    /** @brief Pointer to specific variant context. */
    const void *variant;
};

/**
 * @brief Core variant ThingSet local context.
 *
 * Runtime core variant local context structure (in ROM) per context object.
 */
struct ts_ctx_core {
    /* This is a placeholder for future core context extensions */
};

/**
 * @brief Communication variant ThingSet local context.
 *
 * Runtime communication variant local context structure (in ROM) per context object.
 */
struct ts_ctx_com {
    /* This is a placeholder for future communication context extensions */
};

/*
 * ThingSet generic context support
 * --------------------------------
 */

/** @cond IGNORE */

#if TS_CONFIG_LOCAL_COUNT > 0
extern const struct ts_ctx ts_ctx_0;
#endif
#if TS_CONFIG_LOCAL_COUNT > 1
extern const struct ts_ctx ts_ctx_1;
#endif
#if TS_CONFIG_LOCAL_COUNT > 2
extern const struct ts_ctx ts_ctx_2;
#endif
#if TS_CONFIG_LOCAL_COUNT > 3
extern const struct ts_ctx ts_ctx_3;
#endif
#if TS_CONFIG_LOCAL_COUNT > 4
extern const struct ts_ctx ts_ctx_4;
#endif
#if TS_CONFIG_LOCAL_COUNT > 5
#error "Local contexts limited to 5"
#endif

/* forward declarations for public interface */
extern thingset_oref_t ts_obj_db_oref_root(ts_obj_db_id_t did);
extern thingset_oref_t ts_obj_db_oref_any(ts_obj_db_id_t did);
extern thingset_oref_t ts_obj_db_oref_by_object(const struct ts_obj *object);
extern int ts_obj_db_oref_by_id(ts_obj_db_id_t did, ts_obj_id_t obj_id, thingset_oref_t *oref);
extern int ts_obj_by_path(thingset_oref_t parent, const char *path, size_t len,
                          thingset_oref_t *oref);
extern ts_obj_db_id_t ts_ctx_obj_db(thingset_locid_t locid);

/** @endcond <!-- IGNORE --> */

/**
 * @} <!-- ts_ctx_api_priv -->
 * @addtogroup ts_ctx_api_pub
 * @{
 */

/**
 * @brief Initialize a ThingSet local context.
 *
 * @param[in] locid ThingSet local context identifier.
 * @returns 0 on success, <0 otherwise.
 */
int thingset_init(thingset_locid_t locid);

/**
 * @brief Run a ThingSet local context.
 *
 * @param[in] locid ThingSet local context identifier.
 * @returns 0 on success, <0 otherwise.
 */
int thingset_run(thingset_locid_t locid);

/**
 * @brief Use text mode protocol for application initiated communication.
 *
 * @param[in] locid ThingSet local context identifier.
 */
void thingset_protocol_set_txt(thingset_locid_t locid);

/**
 * @brief Use binary mode protocol for application initiated communication.
 *
 * @param[in] locid ThingSet local context identifier.
 */
void thingset_protocol_set_bin(thingset_locid_t locid);

/**
 * @brief Get current authorisation level.
 *
 * The authorisation flags must match with access flags to allow read/write access to a data object.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Flags that define the authorisation level (1 = access allowed)
 */
uint16_t thingset_authorisation(thingset_locid_t locid);

/**
 * @brief Set current authorisation level.
 *
 * The authorisation flags must match with access flags to allow read/write access to a data object.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] auth Flags to define authorisation level (1 = access allowed)
 */
void thingset_authorisation_set(thingset_locid_t locid, uint16_t auth);

/*
 * ThingSet context (receive) message processing
 * ---------------------------------------------
 */

/**
 * @brief Process a ThingSet message and transfer resulting message.
 *
 * If the message processing results in an output message, this output message is transfered to
 * the appropriate sink. The sink depends on the context.
 *
 * This function expects the process type scratchpad of the message to be set.
 *
 * The function consumes the message buffer (decreases the reference count).
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] msg Pointer to message buffer.
 * @returns 0 on success, <0 otherwise.
 */
int thingset_process(thingset_locid_t locid, struct thingset_msg *msg);

/**
 * @brief Process a ThingSet request buffer and create response in response buffer.
 *
 * This function also detects if JSON or CBOR format is used
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] request Pointer to the ThingSet request buffer
 * @param[in] request_len Length of the data in the request buffer
 * @param[in,out] response Pointer to the buffer where the ThingSet response should be stored
 * @param[in] response_size Size of the response buffer, i.e. maximum allowed length of the response
 * @returns Actual length of the response written to the buffer or <= 0 in case of error or if no
 *          response message has been generated (e.g. because a statement was processed)
 */
int thingset_process_buf(thingset_locid_t locid, const uint8_t *request, size_t request_len,
                         uint8_t *response, size_t response_size);

/*
 * ThingSet context database import/ export support
 * ------------------------------------------------
 */

/**
 * @brief Export data for given subset(s) of data objects.
 *
 * This function can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] subsets Flags to select which subset(s) of data items should be exported.
 * @param[in] buf Pointer to the buffer where the data should be stored
 * @param[in] buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @returns Actual length of the data written to the buffer or <= 0 in case of error
 */
int thingset_export_buf(thingset_locid_t locid, uint16_t subsets, uint8_t *buf, uint16_t buf_size);

/**
 * @brief Import data from buffer into data objects.
 *
 * This function can be used to initialize data objects from previously exported data (using
 * thingset_export_buf() function) and stored in the EEPROM or other non-volatile memory.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] buf Buffer containing ID/value map that should be written to the data objects
 * @param[in] buf_len Length of the data in the buffer
 * @param[in] auth_flags Authentication flags to be used in this function (to override _auth_flags)
 * @param[in] subsets Flags to select which subset(s) of data items should be imported
 * @returns 0 on success, <0 otherwise.
 */
int thingset_import_buf(thingset_locid_t locid, uint8_t *buf, uint16_t buf_len,
                        uint16_t auth_flags, uint16_t subsets);

/*
 * ThingSet context message support
 * --------------------------------
 */

/**
 * @brief Generate statement message based on object reference to group or subset.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] oref Object reference to group or subset object specifying the items to be published
 * @param[out] buf Pointer to the buffer where the publication message should be stored
 * @param[in] buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
int thingset_add_statement_buf(thingset_locid_t locid, thingset_oref_t oref, uint8_t *buf,
                               uint16_t buf_size);

/**
 * @brief ThingSet communication context.
 *
 * Defines and functions that are specific to the communication context only.
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_ctx_com_api_pub ThingSet communication context public interface
 * @{
 */

/**
 * @def THINGSET_COM_DEFINE
 *
 * @brief Define a communication variant ThingSet local context object.
 *
 * @param locid Local context identifier. Paramter must expand to a positiv number.
 *              Usually 0. Limit defined by @ref TS_CONFIG_LOCAL_COUNT.
 */
#if TS_CONFIG_COM
#define THINGSET_COM_DEFINE(locid)                                                      \
    struct ts_ctx_com_data TS_CAT(ts_ctx_com_data_, locid);                             \
    const struct ts_ctx_com TS_CAT(ts_ctx_com_, locid) = {                              \
    };                                                                                  \
    const struct ts_ctx TS_CAT(ts_ctx_, locid) = {                                      \
        .ctx_type = TS_CTX_TYPE_COM,                                                    \
        .db_id = locid,                                                                 \
        .data = &TS_CAT(ts_ctx_com_data_, locid).common,                                \
        .variant = &TS_CAT(ts_ctx_com_, locid)                                          \
    }
#else
#define THINGSET_COM_DEFINE(locid)                                                      \
    const struct ts_ctx TS_CAT(ts_ctx_, locid) = {                                      \
        .ctx_type = TS_CTX_TYPE_INVALID                                                 \
    }
#endif

/**
 * @brief Get communication port identifier by port name.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] port_name Name of port.
 * @param[out] port_id Identifier of port.
 * @returns 0 on success, <0 otherwise.
 */
int thingset_port(thingset_locid_t locid, const char *port_name,
                  thingset_portid_t *port_id);

/**
 * @} ts_ctx_com_api_pub
 */

/**
 * @brief ThingSet core context.
 *
 * Defines and functions that are specific to the core context only.
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_ctx_core_api_pub ThingSet core context public interface
 * @{
 */

/**
 * @def THINGSET_CORE_DEFINE
 *
 * @brief Define a core variant ThingSet local context object.
 */
#if TS_CONFIG_CORE
#define THINGSET_CORE_DEFINE()                                                      \
    struct ts_ctx_core_data TS_CAT(ts_ctx_core_data_, TS_CONFIG_CORE_LOCID);        \
    const struct ts_ctx_core TS_CAT(ts_ctx_core_, TS_CONFIG_CORE_LOCID) = {         \
    };                                                                              \
    const struct ts_ctx TS_CAT(ts_ctx_, TS_CONFIG_CORE_LOCID) = {                   \
        .ctx_type = TS_CTX_TYPE_CORE,                                               \
        .db_id = TS_CONFIG_CORE_LOCID,                                              \
        .data = &TS_CAT(ts_ctx_core_data_, TS_CONFIG_CORE_LOCID).common,            \
        .variant = &TS_CAT(ts_ctx_core_, TS_CONFIG_CORE_LOCID)                      \
    }
#else
#define THINGSET_CORE_DEFINE()                                                      \
    const struct ts_ctx TS_CAT(ts_ctx_, TS_CONFIG_CORE_LOCID) = {                   \
        .ctx_type = TS_CTX_TYPE_INVALID                                             \
    }
#endif

/**
 * @def THINGSET_CORE_DATABASE_DEFINE
 *
 * @brief Define a ThingSet data objects database for core variant local context data objects.
 *
 * Use like this:
 *
 *     THINGSET_CORE_DATABASE_DEFINE(0x123456789012345ULL,
 *         TS_GROUP(ID_INFO, "info", TS_NO_CALLBACK, ID_ROOT),
 *         TS_ITEM_STRING(0x19, "Manufacturer", manufacturer, 0, I D_INFO, TS_ANY_R, 0)
 *     );
 *
 *
 * @note @ref THINGSET_CORE_DATABASE_DEFINE works on the single core variant ThingSet local context.
 *
 * @param ctx_uuid Unique identifier for the context the database belongs to.
 * @param ... Initialisation for the data objects.
 */
#define THINGSET_CORE_DATABASE_DEFINE(ctx_uuid, ...)                                \
    THINGSET_DATABASE_DEFINE(TS_CONFIG_CORE_LOCID, ctx_uuid, __VA_ARGS__)

/**
 * @brief Initialize the ThingSet core context.
 *
 * @note @ref thingset_core_init works on the single core variant ThingSet local context.
 */
static inline int thingset_core_init(void)
{
    return thingset_init(TS_CONFIG_CORE_LOCID);
}

/**
 * @brief Process a ThingSet request buffer and create response in response buffer.
 *
 * This function also detects if JSON or CBOR format is used
 *
 * @note @ref thingset_core_process works on the single core variant ThingSet local context.
 *
 * @param[in] request Pointer to the ThingSet request buffer
 * @param[in] request_len Length of the data in the request buffer
 * @param[in,out] response Pointer to the buffer where the ThingSet response should be stored
 * @param[in] response_size Size of the response buffer, i.e. maximum allowed length of the response
 * @returns Actual length of the response written to the buffer or <= 0 in case of error or if no
 *          response message has been generated (e.g. because a statement was processed)
 */
static inline int thingset_core_process(const uint8_t *request, size_t request_len,
                                        uint8_t *response, size_t response_size)
{
    return thingset_process_buf(TS_CONFIG_CORE_LOCID, request, request_len, response,
                                response_size);
}

/**
 * @brief Log all data objects as a structured JSON text.
 *
 * @warning This function uses a huge buffer for dumping the text before handing it to logging.
 *          It might cause stack overflows if run in constrained devices with a large data object
 *          tree. Use with care and for testing only!
 *
 * @note @ref thingset_dump_json works on the single core variant ThingSet local context.
 *
 * @param obj_id Root object ID where to start with printing
 * @param level Indentation level (=depth inside the data object tree)
 */
void thingset_dump_json(ts_obj_id_t obj_id, int level);

/**
 * @brief Retrieve data in JSON format for given subset(s).
 *
 * This function does not return a complete ThingSet message, but only the payload data as a
 * name/value map. It can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * @note @ref thingset_txt_export works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param subsets Flags to select which subset(s) of data items should be exported
 * @returns Actual length of the data written to the buffer or 0 in case of error
 */
static inline int thingset_txt_export(char *buf, size_t buf_size, uint16_t subsets)
{
    thingset_protocol_set_txt(TS_CONFIG_CORE_LOCID);
    return thingset_export_buf(TS_CONFIG_CORE_LOCID, subsets, (uint8_t *)buf, buf_size);
}

/**
 * @brief Generate statement message in JSON format based on pointer to group or subset.
 *
 * This is the fastest method to generate a statement as it does not require to search through the
 * entire data objects array.
 *
 * @note @ref thingset_txt_statement works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param object Group or subset object specifying the items to be published
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_txt_statement(char *buf, size_t buf_size, const struct ts_obj *object)
{
    thingset_protocol_set_txt(TS_CONFIG_CORE_LOCID);
    return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, ts_obj_db_oref_by_object(object),
                                      (uint8_t *)buf, buf_size);
}

/**
 * @brief Generate statement message in JSON format based on path.
 *
 * @note @ref thingset_txt_statement_by_path works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path Path to group or subset object specifying the items to be published
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_txt_statement_by_path(char *buf, size_t buf_size, const char *path)
{
    thingset_oref_t oref;
    int ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TS_CONFIG_CORE_LOCID)),
                             path, strlen(path), &oref);
    if (ret == 0) {
        thingset_protocol_set_txt(TS_CONFIG_CORE_LOCID);
        return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, oref, (uint8_t *)buf, buf_size);
    }
    return 0;
}

/**
 * @brief Generate statement message in JSON format based on data object ID.
 *
 * @note @ref thingset_txt_statement_by_id works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path ID of group or subset object specifying the items to be published
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_txt_statement_by_id(char *buf, size_t buf_size, ts_obj_id_t id)
{
    thingset_oref_t oref;
    int ret = ts_obj_db_oref_by_id(TS_CONFIG_CORE_LOCID, id, &oref);
    if (ret == 0) {
        thingset_protocol_set_txt(TS_CONFIG_CORE_LOCID);
        return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, oref, (uint8_t *)buf, buf_size);
    }
    return 0;
}

/**
 * @brief Retrieve data in CBOR format for given subset(s).
 *
 * This function does not return a complete ThingSet message, but only the payload data as an
 * ID/value map. It can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * @note @ref thingset_bin_export works on the single core variant ThingSet local context.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param subsets Flags to select which subset(s) of data items should be exported
 * @returns Actual length of the data written to the buffer or 0 in case of error
 */
static inline int thingset_bin_export(uint8_t *buf, size_t buf_size, uint16_t subsets)
{
    thingset_protocol_set_bin(TS_CONFIG_CORE_LOCID);
    return thingset_export_buf(TS_CONFIG_CORE_LOCID, subsets, buf, buf_size);
}

/**
 * @brief Generate statement message in CBOR format based on pointer to group or subset.
 *
 * This is the fastest method to generate a statement as it does not require to search through the
 * entire date nodes array.
 *
 * @note @ref thingset_bin_statement works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param object Group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_bin_statement(uint8_t *buf, size_t buf_size, const struct ts_obj *object)
{
    thingset_protocol_set_bin(TS_CONFIG_CORE_LOCID);
    return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, ts_obj_db_oref_by_object(object), buf,
                                      buf_size);
}

/**
 * @brief Generate statement message in CBOR format based on path.
 *
 * @note @ref thingset_bin_statement_by_path works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path Path to group or subset object specifying the items to be published
 *
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_bin_statement_by_path(uint8_t *buf, size_t buf_size, const char *path)
{
    thingset_oref_t oref;
    int ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TS_CONFIG_CORE_LOCID)), path,
                             strlen(path), &oref);
    if (ret == 0) {
        thingset_protocol_set_bin(TS_CONFIG_CORE_LOCID);
        return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, oref, buf, buf_size);
    }
    return 0;
}

/**
 * @brief Generate statement message in CBOR format based on data object ID.
 *
 * @note @ref thingset_bin_statement_by_id works on the single core variant ThingSet local context.
 *
 * @param buf Pointer to the buffer where the publication message should be stored
 * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
 * @param path ID of group or subset object specifying the items to be published
 * @returns Actual length of the message written to the buffer or 0 in case of error
 */
static inline int thingset_bin_statement_by_id(uint8_t *buf, size_t buf_size, ts_obj_id_t id)
{
    thingset_oref_t oref;
    int ret = ts_obj_db_oref_by_id(TS_CONFIG_CORE_LOCID, id, &oref);
    if (ret == 0) {
        thingset_protocol_set_bin(TS_CONFIG_CORE_LOCID);
        return thingset_add_statement_buf(TS_CONFIG_CORE_LOCID, oref, buf, buf_size);
    }
    return 0;
}

/**
 * @brief Import data in CBOR format into data objects.
 *
 * This function can be used to initialize data objects from previously exported data (using
 * thingset_bin_export function) and stored in the EEPROM or other non-volatile memory.
 *
 * @note @ref thingset_bin_import works on the single core variant ThingSet local context.
 *
 * @param buf Buffer containing ID/value map that should be written to the data objects
 * @param buf_len Length of the data in the buffer
 * @param auth_flags Authentication flags to be used in this function (to override _auth_flags)
 * @param subsets Flags to select which subset(s) of data items should be imported
 * @returns TS_STATUS_CHANGED on success.
 */
static inline int thingset_bin_import(uint8_t *buf, uint16_t buf_len, uint16_t auth_flags,
                                      uint16_t subsets)
{
    int ret = thingset_import_buf(TS_CONFIG_CORE_LOCID, buf, buf_len, auth_flags, subsets);
    return ret == 0 ? TS_STATUS_CHANGED : TS_STATUS_BAD_REQUEST;
}

/**
 * @brief Get data object by ID.
 *
 * @note @ref thingset_object_by_id works on the single core variant ThingSet local context.
 *
 * @param id Data object ID
 * @returns Pointer to data object or NULL if object is not found
 */
const struct ts_obj *thingset_object_by_id(ts_obj_id_t id);

/**
 * @brief Get data object by name.
 *
 * As the names are not necessarily unique in the entire data tree, the parent is needed
 *
 * @note @ref thingset_object_by_name works on the single core variant ThingSet local context.
 *
 * @param name Data object name
 * @param len Length of the object name
 * @param parent Data object ID of the parent or -1 for global search
 * @returns Pointer to data object or NULL if object is not found
 */
const struct ts_obj *thingset_object_by_name(const char *name, size_t len, int32_t parent);

/**
 * @brief Get data object by path.
 *
 * Get the endpoint object of a provided path.
 *
 * @note @ref thingset_object_by_path works on the single core variant ThingSet local context.
 *
 * @param path Path with multiple object names separated by forward slash.
 * @param len Length of the entire path
 *
 * @returns Pointer to data object or NULL if object is not found
 */
const struct ts_obj *thingset_object_by_path(const char *path, size_t len);

/**
 * @} <!-- ts_ctx_core_api_pub -->
 */

#ifdef __cplusplus
}
#endif

/**
 * @} <!-- ts_ctx_api_pub -->
 */

/** @page ts_topic_ctx Contexts

TBD

*/

#endif /* THINGSET_CTX_H_ */
