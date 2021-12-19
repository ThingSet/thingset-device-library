/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet local context (private interface)
 */

/**
 * @brief Abstraction over ThingSet local context.
 *
 * @defgroup ts_ctx_api_priv ThingSet local context (private interface)
 * @{
 */

#ifndef TS_COM_H_
#define TS_COM_H_

#include "thingset_ctx.h"
#include "ts_port.h"
#include "ts_obj.h"
#include "ts_msg.h"
#include "ts_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ThingSet context handling
 * -------------------------
 */

/**
 * @def TS_CTX_IS_CORE
 *
 * @brief Is given local context of core variant type.
 *
 * @param locid Local context identifier.
 */
#if TS_CONFIG_CORE && TS_CONFIG_COM
#define TS_CTX_IS_CORE(locid)   (ts_ctxs[locid]->ctx_type == TS_CTX_TYPE_CORE)
#elif TS_CONFIG_CORE
#define TS_CTX_IS_CORE(locid)   (true)
#elif TS_CONFIG_COM
#define TS_CTX_IS_CORE(locid)   (false)
#endif

/**
 * @def TS_CTX_IS_COM
 *
 * @brief Is given local context of communication variant type.
 *
 * @param locid Local context identifier.
 */
#if TS_CONFIG_CORE && TS_CONFIG_COM
#define TS_CTX_IS_COM(locid)    (ts_ctxs[locid]->ctx_type == TS_CTX_TYPE_COM)
#elif TS_CONFIG_CORE
#define TS_CTX_IS_COM(locid)    (false)
#elif TS_CONFIG_COM
#define TS_CTX_IS_COM(locid)    (true)
#endif

/**
 *@brief Local contexts by context id.
 *
 * Array of pointers to contexts. Array index == context id.
 */
extern const struct ts_ctx *const ts_ctxs[TS_CONFIG_LOCAL_COUNT];

/**
 * @brief Get core variant local context given by local context identifier.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Pointer to core variant local context.
 */
static inline const struct ts_ctx_core *ts_ctx_core_context(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
#if TS_CONFIG_CORE && TS_CONFIG_COM
    TS_ASSERT(TS_CTX_IS_CORE(locid), "CTX: %s on wrong context type", __func__);
    return (const struct ts_ctx_core *)(ts_ctxs[locid]->variant);
#elif TS_CONFIG_CORE
    return (const struct ts_ctx_core *)(ts_ctxs[locid]->variant);
#elif TS_CONFIG_COM
    TS_ASSERT(false, "CTX: %s on wrong context type", __func__);
    return NULL;
#endif
}

/**
 * @brief Get core variant local context data given by local context identifier.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Pointer to core variant local context data.
 */
static inline struct ts_ctx_core_data *ts_ctx_core_data(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
#if TS_CONFIG_CORE && TS_CONFIG_COM
    TS_ASSERT(TS_CTX_IS_CORE(locid), "CTX: %s on wrong context type", __func__);
    return (struct ts_ctx_core_data *)(ts_ctxs[locid]->data);
#elif TS_CONFIG_CORE
    return (struct ts_ctx_core_data *)(ts_ctxs[locid]->data);
#elif TS_CONFIG_COM
    TS_ASSERT(false, "CTX: %s on wrong context type", __func__);
    return NULL;
#endif
}

/**
 * @brief Get communication variant local context given by local context identifier.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Pointer to communication variant local context.
 */
static inline const struct ts_ctx_com *ts_ctx_com_context(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
#if TS_CONFIG_CORE && TS_CONFIG_COM
    TS_ASSERT(TS_CTX_IS_COM(locid), "CTX: %s on wrong context type", __func__);
    return (const struct ts_ctx_com *)(ts_ctxs[locid]->variant);
#elif TS_CONFIG_CORE
    TS_ASSERT(false, "CTX: %s on wrong context type", __func__);
    return NULL;
#elif TS_CONFIG_COM
    return (const struct ts_ctx_com *)(ts_ctxs[locid]->variant);
#endif
}

/**
 * @brief Get communication variant local context data given by local context identifier.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Pointer to communication variant local context data.
 */
static inline struct ts_ctx_com_data *ts_ctx_com_data(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
#if TS_CONFIG_CORE && TS_CONFIG_COM
    TS_ASSERT(TS_CTX_IS_COM(locid), "CTX: %s on wrong context type", __func__);
    return (struct ts_ctx_com_data *)(ts_ctxs[locid]->data);
#elif TS_CONFIG_CORE
    TS_ASSERT(false, "CTX: %s on wrong context type", __func__);
    return NULL;
#elif TS_CONFIG_COM
    return (struct ts_ctx_com_data *)(ts_ctxs[locid]->data);
#endif
}

/**
 * @brief Get the data objects database of this local context.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Data objects database ID.
 */
ts_obj_db_id_t ts_ctx_obj_db(thingset_locid_t locid);

/**
 * @brief Get the unique identifier of this context.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return Pointer to unique identifier
 */
const thingset_uid_t *ts_ctx_uid(thingset_locid_t locid);

/**
 * @brief Is binary mode protocol selected for application initiated communication.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return True in case the binary protocol is selected, false otherwise.
 */
static inline bool ts_ctx_protocol_is_bin(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    return ts_ctxs[locid]->data->app_protocol_use_bin;
}

/*
 * ThingSet context (receive) message processing
 * ---------------------------------------------
 */

/**
 * @brief Lock local context for message processing.
 *
 * The local context is locked to the calling thread. Recursive relock is possible.
 *
 * @param[in] locid ThingSet local context identifier.
 * @returns 0 on success, <0 otherwise.
 * @retval -EOWNERDEAD if locked by another thread.
 */
int ts_ctx_process_lock(thingset_locid_t locid);

/**
 * @brief Unock local context from message processing.
 *
 * Release local context from calling thread.
 *
 * @param[in] locid ThingSet local context identifier.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_process_unlock(thingset_locid_t locid);

/**
 * @brief Process a ThingSet message and create out message.
 *
 * This function expects the process type scratchpad of the message to be set.
 *
 * The function consumes the input message buffer (decreases the reference count).
 *
 * The functions locks the local context to the calling thread during message processing.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] msg_in Pointer to input message buffer.
 * @param[out] msg_out Pointer to output message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_process_msg(thingset_locid_t locid, struct thingset_msg *msg_in,
                       struct thingset_msg **msg_out);

/**
 * @brief Process a ThingSet request.
 *
 * This function also detects if JSON or CBOR format is used.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] request Pointer to request message buffer.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_process_request(thingset_locid_t locid, struct thingset_msg *request,
                           struct thingset_msg **response);


/**
 * @brief Process a ThingSet request error.
 *
 * Allocate and create a response message for a request error.
 *
 * @param[in] response_msg_proto Message protocol to use for response message.
 * @param[in] response_msg_code Code to report in response message.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_process_request_error(enum ts_msg_proto response_msg_proto,
                                 enum ts_msg_code response_msg_code,
                                 struct thingset_msg **response);

/**
 * @brief Process a ThingSet CREATE request in CBOR format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_create_cbor(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response);

/**
 * @brief Process a ThingSet CREATE request in JSON format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_create_json(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response);

/**
 * @brief Process a ThingSet DELETE request in CBOR format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_delete_cbor(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response);

/**
 * @brief Process a ThingSet DELETE request in JSON format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_delete_json(struct thingset_msg *request, thingset_oref_t oref,
                               struct thingset_msg **response);

/**
 * @brief Process a ThingSet EXEC request in CBOR format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_exec_cbor(struct thingset_msg *request, thingset_oref_t oref,
                             struct thingset_msg **response);

/**
 * @brief Process a ThingSet EXEC request in JSON format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_exec_json(struct thingset_msg *request, thingset_oref_t oref,
                             struct thingset_msg **response);

/**
 * @brief Process a ThingSet PATCH request in CBOR format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_patch_cbor(struct thingset_msg *request, thingset_oref_t oref,
                              struct thingset_msg **response);

/**
 * @brief Process a ThingSet PATCH request in JSON format.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_patch_json(struct thingset_msg *request, thingset_oref_t oref,
                              struct thingset_msg **response);

/**
 * @brief Process the part of a ThingSet request in CBOR format to set object data.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_set_cbor(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response);

/**
 * @brief Process the part of a ThingSet request in JSON format to set object data.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @param[in] request Pointer to request message buffer.
 * @param[in] oref Database object reference to object the request is for.
 * @param[out] response Pointer to response message buffer. Null if there is none.
 * @returns 0 on success, <0 otherwise. Even in case of success no response message may have been
 *          generated (e.g. because a statement was processed).
 */
int ts_ctx_process_set_json(struct thingset_msg *request, thingset_oref_t oref,
                            struct thingset_msg **response);

/*
 * ThingSet context port handling
 * ------------------------------
 */

/**
 * @brief Transmit message on port.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] port_id Port identifier of port to send at.
 * @param[in] msg Pointer to message buffer to be send.
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @return 0 on success, <0 otherwise.
 */
int ts_ctx_transmit(thingset_locid_t locid, thingset_portid_t port_id,
                    struct thingset_msg *msg, thingset_time_ms_t timeout_ms);


/*
 * ThingSet context node handling
 * ------------------------------
 */

/**
 * @brief Init node table.
 *
 * @param[in] locid ThingSet local context identifier.
 */
void ts_ctx_node_table_init(thingset_locid_t locid);

/**
 * @brief Init node table entry to phantom node.
 *
 * Also no port info shall be attached to the node table entry.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] node_idx Index of node table element.
 * @param[in] ctx_uid Ponter to unique context identifier of neighbour/ originator node.
 */
void ts_ctx_node_init_phantom(thingset_locid_t locid, uint16_t node_idx,
                              const thingset_uid_t *ctx_uid);

/**
 * @brief Get a node table entry.
 *
 * If the node table entry already exists return the existing one.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] ctx_uid Ponter to unique context identifier of neighbour/ originator node.
 * @param[out] node_idx Index of node table element.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_node_get(thingset_locid_t locid, const thingset_uid_t* ctx_uid, uint16_t* node_idx);

/**
 * @brief Search best candidate node for eviction from node table.
 *
 * Best candidate is a node that was not seen for a long time.
 *
 * @param[in] locid ThingSet local context identifier.
 * @return node_idx of best candidate node for eviction.
 */
uint16_t ts_ctx_node_evict(thingset_locid_t locid);

/**
 * @brief Free a node table entry.
 *
 * Mark node information unused.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] node_idx Index of node table element.
 */
void ts_ctx_node_free(thingset_locid_t locid, uint16_t node_idx);

/**
 * @brief Lookup the node table element of a node.
 *
 * @param[in] locid ThingSet local context identifier.
 * @param[in] ctx_uid Ponter to unique context identifier of neighbour/ originator node.
 * @param[out] node_idx Index of node table element.
 * @returns 0 on success, <0 otherwise.
 */
int ts_ctx_node_lookup(thingset_locid_t locid, const thingset_uid_t* ctx_uid, uint16_t* node_idx);

/*
 * ThingSet context application remote object support
 * --------------------------------------------------
 */

int ts_ctx_obj_connect(thingset_locid_t locid, thingset_uid_t *ctx_uid,
                       ts_obj_id_t obj_id, const char *obj_path, thingset_time_ms_t timeout_ms,
                       thingset_oref_t *oref);

int ts_ctx_obj_disconnect(thingset_locid_t locid, thingset_oref_t oref);

/*
 * ThingSet context application command support
 * --------------------------------------------
 */

int ts_ctx_cmd_publish(thingset_locid_t locid, thingset_portid_t *app_port_id,
                       thingset_oref_t oref, thingset_time_ms_t period_ms);

int ts_ctx_cmd_subscribe(thingset_locid_t locid, thingset_portid_t *app_port_id,
                         thingset_oref_t oref);

int ts_ctx_cmd_change(thingset_locid_t locid, thingset_portid_t *app_port_id,
                      thingset_oref_t oref, thingset_time_ms_t timeout_ms);

int ts_ctx_cmd_fetch(thingset_locid_t locid, thingset_portid_t *app_port_id,
                     thingset_oref_t oref, uint16_t subsets, thingset_time_ms_t timeout_ms);

int ts_ctx_cmd_exec(thingset_locid_t locid, thingset_portid_t *app_port_id,
                    thingset_oref_t oref, thingset_time_ms_t timeout_ms);

/*
 * Helpers
 * -------
 */

/**
 * @brief Check unique context identifiers to be equal.
 *
 * @param uuid_a Pointer to uuid of a context.
 * @param uuid_b Pointer to uuid of a context.
 * @returns true on equal, false otherwise.
 */
bool ts_ctx_uid_equal(const thingset_uid_t *uuid_a, const thingset_uid_t *uuid_b);

#ifdef __cplusplus
}
#endif

/**
 * @} ts_ctx_api_priv
 */

#endif /* TS_COM_H_ */
