/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet message (private interface)
 */

#ifndef TS_MSG_H_
#define TS_MSG_H_

/**
 * @brief Abstraction over ThingSet messages.
 *
 * Messages are organized in a special way in message buffers:
 * - scratchroom - Room always occupied by the scratchpad
 * - headroom    - data already processed
 * - data        - the payload
 * - tailroom    - space to fill additional payload
 * - scratchroom - additional room occupied by the scratchpad for specific tasks
 *
 * @defgroup ts_msg_api_priv ThingSet message (private interface)
 * @{
 */

#include "thingset_env.h"
#include "thingset_msg.h"

#include "ts_macro.h"
#include "ts_log.h"
#include "ts_cobs.h"
#include "ts_jsmn.h"
#include "ts_buf.h"
#include "ts_obj.h"
#include "ts_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message status
 *
 * @defgroup ts_msg_api_status_priv ThingSet message status (private interface)
 *
 * @{
 */

#define TS_MSG_STATUS_TYPE(code)        (code >> 4)
#define TS_MSG_STATUS_ID(code)          (code & 0x0F)

/**
 * @brief Message validity.
 */
enum ts_msg_valid {
    TS_MSG_VALID_UNSET              = 0,
    TS_MSG_VALID_OK                 = 1,
    TS_MSG_VALID_ERROR              = 2,
};

/**
 * @brief Message protocol.
 */
enum ts_msg_proto {
    TS_MSG_PROTO_BIN                = 1,
    TS_MSG_PROTO_TXT                = 2
};


/**
 * @brief Message type.
 */
enum ts_msg_type {
    TS_MSG_TYPE_REQUEST             = 0,
    TS_MSG_TYPE_RESPONSE            = 1,
    TS_MSG_TYPE_STATEMENT           = 2
};

/**
 * @brief Message encoding.
 */
enum ts_msg_encoding {
    TS_MSG_ENCODING_NONE            = 0,
    TS_MSG_ENCODING_COBS            = 1,
    TS_MSG_ENCODING_CAN             = 2
};


/**
 * @brief Message codes (status codes same as CoAP).
 */
enum ts_msg_code {
// request message code - TS_MSG_VALID_OK
    TS_MSG_CODE_BIN_GET                     = 0x01,
    TS_MSG_CODE_BIN_POST                    = 0x02,
    TS_MSG_CODE_BIN_DELETE                  = 0x04,
    TS_MSG_CODE_BIN_FETCH                   = 0x05,
    TS_MSG_CODE_BIN_PATCH                   = 0x07,
    TS_MSG_CODE_BIN_STATEMENT               = 0x1F,
    TS_MSG_CODE_TXT_GET                     = '?',
    TS_MSG_CODE_TXT_PATCH                   = '=',
    TS_MSG_CODE_TXT_CREATE                  = '+',
    TS_MSG_CODE_TXT_DELETE                  = '-',
    TS_MSG_CODE_TXT_EXEC                    = '!',
    TS_MSG_CODE_TXT_STATEMENT               = '#',

// response message code - general - TS_MSG_VALID_OK
    TS_MSG_CODE_TXT_RESPONSE                = ':',

// response message code - success - TS_MSG_VALID_OK
    TS_MSG_CODE_CREATED                     = 0x81,
    TS_MSG_CODE_DELETED                     = 0x82,
    TS_MSG_CODE_VALID                       = 0x83,
    TS_MSG_CODE_CHANGED                     = 0x84,
    TS_MSG_CODE_CONTENT                     = 0x85,
    TS_MSG_CODE_EXPORT                      = 0x86,

// request status code/ response message code - client errors - TS_MSG_VALID_ERROR/ TS_MSG_VALID_OK
    TS_MSG_CODE_BAD_REQUEST                 = 0xA0,
    TS_MSG_CODE_UNAUTHORIZED                = 0xA1,       // need to authenticate
    TS_MSG_CODE_FORBIDDEN                   = 0xA3,       // trying to write read-only value
    TS_MSG_CODE_NOT_FOUND                   = 0xA4,
    TS_MSG_CODE_METHOD_NOT_ALLOWED          = 0xA5,
    TS_MSG_CODE_REQUEST_INCOMPLETE          = 0xA8,
    TS_MSG_CODE_CONFLICT                    = 0xA9,
    TS_MSG_CODE_REQUEST_TOO_LARGE           = 0xAD,
    TS_MSG_CODE_UNSUPPORTED_FORMAT          = 0xAF,
// request status code/ response message code - server errors - TS_MSG_VALID_ERROR/ TS_MSG_VALID_OK
    TS_MSG_CODE_INTERNAL_SERVER_ERR         = 0xC0,
    TS_MSG_CODE_NOT_IMPLEMENTED             = 0xC1,
// request status code/ response message code - specific errors - TS_MSG_VALID_ERROR/ TS_MSG_VALID_OK
    TS_MSG_CODE_RESPONSE_TOO_LARGE          = 0xE1,

// request status code - TS_MSG_VALID_OK
    TS_MSG_CODE_REQUEST_CREATE              = 0x01U,
    TS_MSG_CODE_REQUEST_DELETE              = 0x02U,
    TS_MSG_CODE_REQUEST_EXEC                = 0x03U,
    TS_MSG_CODE_REQUEST_FETCH_IDS           = 0x04U,
    TS_MSG_CODE_REQUEST_FETCH_NAMES         = 0x05U,
    TS_MSG_CODE_REQUEST_FETCH_SINGLE        = 0x06U,
    TS_MSG_CODE_REQUEST_FETCH_VALUES        = 0x07U,
    TS_MSG_CODE_REQUEST_GET_IDS             = 0x08U,
    TS_MSG_CODE_REQUEST_GET_IDS_VALUES      = 0x09U,
    TS_MSG_CODE_REQUEST_GET_NAMES           = 0x0AU,
    TS_MSG_CODE_REQUEST_GET_NAMES_VALUES    = 0x0BU,
    TS_MSG_CODE_REQUEST_GET_VALUES          = 0x0CU,
    TS_MSG_CODE_REQUEST_PATCH               = 0x0DU
};

/**
 * @brief ThingSet message status code type.
 */
typedef uint8_t ts_msg_status_code_t;

/**
 * @brief Internal message status.
 *
 * A message status describes the message content:
 * - validity
 * - protocol
 * - type
 * - encoding
 *
 * and for each type of message the specific characteristic of the message given by the message
 * code.
 */
struct ts_msg_stat {
    /**
     * @brief Message validity.
     */
    uint8_t valid : 2;

    /**
     * @brief Message protocol.
     */
    uint8_t proto : 2;

    /**
     * @brief Message type.
     */
    uint8_t type : 2;

    /**
     * @brief Message encoding.
     */
    uint8_t encoding : 2;

    /**
     * @brief Message status code.
     */
    ts_msg_status_code_t code;
};

/**
 * @} ts_msg_api_status_priv
 */

/**
 * @brief ThingSet message buffer support
 *
 * @defgroup ts_msg_api_buf_priv ThingSet message buffer (private interface)
 * @{
 */

#define TS_MSG_THINGSET         0x7U

/** @brief Standard scratchpad that is always available to a message buffer */
#define TS_MSG_SCRATCHPAD_STD   0x0U

/**
 * @brief Data structure for standard scratchroom at start of message buffer.
 */
struct ts_msg_buf_scratchroom {
    /** @brief Message scratchpad type */
    struct {
        uint8_t thingset : 4;
        uint8_t id : 4;
    } scratchtype;

    /** @brief Message status. */
    struct ts_msg_stat status;

    /** @brief Authorisation rights at the time the message was received/ created */
    uint16_t auth;

    /**
     * @brief Message extra scratchroom definition.
     *
     * @note Must be last element in standard scratchrom as buf[2] may be used as
     *       extension to message.
     */
    union {
        /** @brief Size of extra scratchroom allocated at buffer tailroom. */
        uint16_t size;
        /** @brief Minimal extra scratchroom buffer in case no tailroom is allocated. */
        uint8_t buf[2];
    } scratchroom;
};

/**
 * @def TS_MSG_ASSERT
 *
 * @brief Assert message in message buffer is a ThingSet message.
 *
 * @note Macro asserts on wrong message type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] msg Pointer to the message.
 */
#if !TS_CONFIG_ASSERT
#define TS_MSG_ASSERT(msg)
#else
#define TS_MSG_ASSERT(msg)                                                                      \
    do {                                                                                        \
        struct ts_msg_buf_scratchroom *ts_assert_scratchpad =                                   \
            (struct ts_msg_buf_scratchroom *)(ts_buf_data((struct ts_buf *)msg)                 \
                                             - ts_buf_headroom((struct ts_buf *)msg));          \
        TS_ASSERT((ts_assert_scratchpad != NULL),                                               \
                  "Invalid ThingSet msg scratchpad (NULL).");                                   \
        TS_ASSERT((ts_assert_scratchpad->scratchtype.thingset == TS_MSG_THINGSET),              \
                  "Invalid ThingSet msg type (0x%x).",                                          \
                  (unsigned int)(ts_assert_scratchpad->scratchtype.thingset));                  \
    } while(0)
#endif

/**
 * @def TS_MSG_ASSERT_SCRATCHTYPE
 *
 * @brief Assert scratchpad of message in message buffer is of given scratchpad type.
 *
 * @note Macro asserts on wrong message type or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] type Scratchpad type.
 */
#if !TS_CONFIG_ASSERT
#define TS_MSG_ASSERT_SCRATCHTYPE(msg, type)
#else
#define TS_MSG_ASSERT_SCRATCHTYPE(msg, type)                                                    \
    do {                                                                                        \
        struct ts_msg_buf_scratchroom *ts_assert_scratchpad =                                   \
            (struct ts_msg_buf_scratchroom *)(ts_buf_data((struct ts_buf *)msg)                 \
                                             - ts_buf_headroom((struct ts_buf *)msg));          \
        TS_ASSERT((ts_assert_scratchpad != NULL),                                               \
                  "Invalid ThingSet msg scratchpad (NULL).");                                   \
        TS_ASSERT((ts_assert_scratchpad->scratchtype.thingset == TS_MSG_THINGSET),              \
                  "Invalid ThingSet msg type (0x%x).",                                          \
                  (unsigned int)(ts_assert_scratchpad->scratchtype.thingset));                  \
        TS_ASSERT((ts_assert_scratchpad->scratchtype.id == type),                               \
                  "Unexpected ThingSet msg scratchpad type (0x%x) - expected 0x%x.",            \
                  (unsigned int)(ts_assert_scratchpad->scratchtype.id), (unsigned int)type);    \
    } while(0)
#endif

/**
 * @def TS_MSG_BUF_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to standard scratchpad of start of message.
 *
 * @note Macro asserts on wrong message type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to standard scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_BUF_SCRATCHPAD_PTR_INIT(name, msg)                                               \
    TS_MSG_ASSERT(msg);                                                                         \
    struct ts_msg_buf_scratchroom *name =                                                       \
        (struct ts_msg_buf_scratchroom *)(ts_buf_data((struct ts_buf *)msg)                     \
                                          - ts_buf_headroom((struct ts_buf*)msg))

/**
 * @brief Allocate a ThingSet message buffer from the buffer pool.
 *
 * The message buffer is allocated with reference count set to 1.
 *
 * Allocation accounts for process scratchpad size only.
 *
 * @param[in] size The size of the message buffer
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_alloc_raw(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg);

/**
 * @brief Current data pointer of the message.
 *
 * Get a pointer to the current start of the message in the message buffer.
 *
 * @param[in] msg Pointer to the message.
 * @return Current start of the message in the message buffer.
 */
static inline uint8_t *ts_msg_data(struct thingset_msg *msg)
{
    TS_MSG_ASSERT(msg);

    return ts_buf_data((struct ts_buf *)msg);
}

/**
 * @brief Current tail pointer of the message.
 *
 * Get a pointer to the current end of the message in the message buffer.
 *
 * @param[in] msg Pointer to the message.
 * @return Current tail pointer of the message.
 */
static inline uint8_t *ts_msg_tail(struct thingset_msg *msg)
{
    TS_MSG_ASSERT(msg);

    return ts_buf_tail((struct ts_buf *)msg);
}

/**
 * @brief Current length of message in message buffer.
 *
 * @param[in] msg Pointer to the message.
 * @return Current length of message.
 */
static inline uint16_t ts_msg_len(struct thingset_msg *msg)
{
    TS_MSG_ASSERT(msg);

    return ts_buf_len((struct ts_buf *)msg);
}

/**
 * @brief Size of message buffer headroom.
 *
 * Free space at the beginning of the message buffer.
 *
 * @param[in] msg Pointer to the message.
 * @return Number of bytes available at the beginning of the message buffer.
 */
static inline uint16_t ts_msg_headroom(struct thingset_msg *msg)
{
    TS_MSG_ASSERT(msg);

    return ts_buf_headroom((struct ts_buf *)msg) - sizeof(struct ts_msg_buf_scratchroom);
}

/**
 * @brief Size of message buffer tailroom.
 *
 * Free space at the end of the message buffer.
 *
 * @param[in] msg Pointer to the message.
 * @return Number of bytes available at the end of the message buffer.
 */
static inline uint16_t ts_msg_tailroom(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return ts_buf_tailroom((struct ts_buf *)msg) - scratchpad->scratchroom.size;
}

/**
 * @brief Size of message buffer scratchroom.
 *
 * Scratchpad space.
 *
 * @param[in] msg Pointer to the message.
 * @return Number of bytes available.
 */
static inline uint16_t ts_msg_scratchroom(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->scratchroom.size;
}

/**
 * @brief Get authentification rights from message data.
 *
 * @param[in] msg Pointer to the message.
 * @return Authentification rights.
 */
static inline uint16_t ts_msg_auth(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->auth;
}

/**
 * @brief Set authentification rights in message data.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] auth Authentification rights.
 */
static inline void ts_msg_auth_set(struct thingset_msg *msg, uint16_t auth)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->auth = auth;
    TS_LOGD("MSG: %s set message 0x%" PRIXPTR " authorisation to 0x%04x", __func__,
            (uintptr_t)msg, (unsigned int)auth);
}

/**
 * @ingroup ts_msg_api_status_priv
 * @brief Set message status by message status code.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] valid Message validty.
 * @param[in] proto Message protocol.
 * @param[in] type Message type.
 * @param[in] code Message status code.
 */
static inline void ts_msg_status_set(struct thingset_msg *msg, enum ts_msg_valid valid,
                                     enum ts_msg_proto proto, enum ts_msg_type type,
                                     enum ts_msg_code code)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->status.valid = valid;
    scratchpad->status.proto = proto;
    scratchpad->status.type = type;
    scratchpad->status.code = code;

    TS_ASSERT((scratchpad->status.type == TS_MSG_TYPE_REQUEST) ||
              (scratchpad->status.type == TS_MSG_TYPE_RESPONSE) ||
              (scratchpad->status.type == TS_MSG_TYPE_STATEMENT),
              "MSG: %s sets message status to invalid type 0x%x", __func__,
              (unsigned int)scratchpad->status.type);

    TS_LOGD("MSG: %s set message 0x%" PRIXPTR " status valid to 0x%02x", __func__,
            (uintptr_t)msg, (unsigned int)valid);
    TS_LOGD("MSG: %s set message 0x%" PRIXPTR " status proto to 0x%02x", __func__,
            (uintptr_t)msg, (unsigned int)proto);
    TS_LOGD("MSG: %s set message 0x%" PRIXPTR " status type to 0x%02x", __func__,
            (uintptr_t)msg, (unsigned int)type);
    TS_LOGD("MSG: %s set message 0x%" PRIXPTR " status code to 0x%02x", __func__,
            (uintptr_t)msg, (unsigned int)code);
}

/**
 * @brief Get message protocol.
 *
 * @param[in] msg Pointer to the message.
 * @return Message validity.
 */
static inline enum ts_msg_valid ts_msg_status_valid(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return (enum ts_msg_valid)scratchpad->status.valid;
}

/**
 * @brief Get message protocol.
 *
 * @param[in] msg Pointer to the message.
 * @return Message protocol.
 */
static inline enum ts_msg_proto ts_msg_status_proto(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return (enum ts_msg_proto)scratchpad->status.proto;
}

/**
 * @brief Get message type.
 *
 * @param[in] msg Pointer to the message.
 * @return Message type.
 */
static inline enum ts_msg_type ts_msg_status_type(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return (enum ts_msg_type)scratchpad->status.type;
}


/**
 * @brief Get message encoding.
 *
 * @param[in] msg Pointer to the message.
 * @return Message encoding.
 */
static inline enum ts_msg_encoding ts_msg_status_encoding(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return (enum ts_msg_encoding)scratchpad->status.encoding;
}

/**
 * @brief Get message status as message status code.
 *
 * @param[in] msg Pointer to the message.
 * @return Message status code.
 */
static inline ts_msg_status_code_t ts_msg_status_code(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->status.code;
}

/**
 * @brief Set message status code.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] code Message status code.
 */
static inline void ts_msg_status_code_set(struct thingset_msg *msg, enum ts_msg_code code)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->status.code = code;
}

/**
 * @brief Set message status to error.
 *
 * Marks the message to be invalid due to error.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] error Message status error code.
 */
static inline void ts_msg_status_error_set(struct thingset_msg *msg, enum ts_msg_code error)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->status.valid = TS_MSG_VALID_ERROR;
    scratchpad->status.code = error;
}

/**
 * @brief Get message status.
 *
 * @param[in] msg Pointer to the message.
 * @return Message status.
 */
static inline struct ts_msg_stat ts_msg_status(struct thingset_msg *msg)
{
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->status;
}

/** @} ts_msg_api_buf_priv */

/*
 * ThingSet message parsing state support
 * --------------------------------------
 */

/**
 * @brief Structure for parsing state of a message buffer.
 *
 * This is used for temporarily storing the parsing state of a message buffer
 * while giving control of the parsing to a routine which we don't control.
 */
struct ts_msg_state {
    /** @brief Data pointer to the beginning of the remaining message */
    uint8_t *data;
    /** @brief Data pointer after the end of the message */
    uint8_t *tail;
};

/**
 * @brief Save the parsing state of a message buffer.
 *
 * Save the parsing state of a buffer so it can be restored later by ts_msg_state_restore().
 *
 * @note Does not save scratchpad state(s).
 *
 * @param[in] msg Pointer to the message.
 * @param[out] state Pointer to storage for the state.
 */
static inline void ts_msg_state_save(struct thingset_msg *msg, struct ts_msg_state *state)
{
    state->data = ts_msg_data(msg);
    state->tail = ts_msg_tail(msg);
}

/**
 * @brief Restore the parsing state of a message buffer.
 *
 * Restore the parsing state of a buffer from a state previously stored by ts_msg_state_save().
 *
 * @note Does not restore scratchpad state(s).
 *
 * @param[in] msg Pointer to the message.
 * @param[out] state Stored state.
 */
static inline void ts_msg_state_restore(struct thingset_msg *msg, struct ts_msg_state *state)
{
    ts_buf_push((struct ts_buf *)msg, ts_msg_data(msg) - state->data);
    ts_buf_add((struct ts_buf *)msg, state->tail - ts_msg_tail(msg));

    TS_ASSERT(ts_msg_data(msg) == state->data,
              "MSG: %s restores message 0x%" PRIXPTR " to invalid data 0x%" PRIXPTR
              " instead of 0x%" PRIXPTR, __func__, (uintptr_t)msg, (uintptr_t)ts_msg_data(msg),
              (uintptr_t)state->data);
    TS_ASSERT(ts_msg_tail(msg) == state->tail,
              "MSG: %s restores message 0x%" PRIXPTR " to invalid tail 0x%" PRIXPTR
              " instead of 0x%" PRIXPTR, __func__, (uintptr_t)msg, (uintptr_t)ts_msg_tail(msg),
              (uintptr_t)state->tail);
}

/*
 * ThingSet message (en/de-)coder support
 * --------------------------------------
 */

/**
 * @brief Message processing support.
 *
 * @defgroup ts_msg_api_proc_priv ThingSet message processing support (private interface)
 *
 * @{
 */

/** @brief Scratchpad for ThingSet context message processing */
#define TS_MSG_SCRATCHPAD_PROC  0x1U

/**
 * @brief Data structure for message processing scratchroom at end of message.
 */
struct ts_msg_proc_scratchroom {
    /** @brief Source port */
    thingset_portid_t port_src;

    /** @brief Destination port */
    thingset_portid_t port_dest;

    /** @brief Unique context identifier of node the message was sent from or shall be send to */
    thingset_uid_t ctx_uid;

    /** @brief Subset to use if message is for import/ export of ThingSet data objects. */
    uint16_t import_export_subset;

    /** @brief Size hint for response message if this message is a request */
    uint16_t response_size_hint;
};

/**
 * @def TS_MSG_PROC_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to message processing scratchpad of message.
 *
 * @note Macro asserts on wrong message or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to message processing scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_PROC_SCRATCHPAD_PTR_INIT(name, msg)                                              \
    TS_MSG_ASSERT_SCRATCHTYPE(msg, TS_MSG_SCRATCHPAD_PROC);                                     \
    /* ts_msg_tailroom() accounts for scratchroom */                                            \
    struct ts_msg_proc_scratchroom *name =                                                   \
        (struct ts_msg_proc_scratchroom *)(ts_msg_tail(msg) + ts_msg_tailroom(msg))

/**
 * @brief Setup message buffer for ThingSet context message processing.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_proc_setup(struct thingset_msg *msg);

/**
 * @brief Get unique context identifier from message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @returns Pointer to node identifier.
 */
static inline const thingset_uid_t *ts_msg_proc_get_ctx_uid(struct thingset_msg *msg)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return &scratchpad->ctx_uid;
}

/**
 * @brief Set unique context identifier in message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] ctx_uid Pointer to unique context identifier.
 */
static inline void ts_msg_proc_set_ctx_uid(struct thingset_msg *msg, const thingset_uid_t *ctx_uid)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->ctx_uid = *ctx_uid;
}

/**
 * @brief Get source port from message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @returns Source port identifier.
 */
static inline thingset_portid_t ts_msg_proc_get_port_src(struct thingset_msg *msg)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->port_src;
}

/**
 * @brief Set source port in message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] port_id Source port identifier.
 */
static inline void ts_msg_proc_set_port_src(struct thingset_msg *msg, thingset_portid_t port_id)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->port_src = port_id;
}

/**
 * @brief Get destination port from message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @returns Destination port identifier.
 */
static inline thingset_portid_t ts_msg_proc_get_port_dest(struct thingset_msg *msg)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->port_dest;
}

/**
 * @brief Set Destination port in message processing data.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] port_id Destination port identifier.
 */
static inline void ts_msg_proc_set_port_dest(struct thingset_msg *msg, thingset_portid_t port_id)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->port_dest = port_id;
}

/**
 * @brief Get reponse size hint from message processing data.
 *
 * A request message may contain a hint of the suitable size for a response message.
 *
 * @param[in] msg Pointer to the message.
 * @returns Response size. 0 indicates no hint.
 */
static inline uint16_t ts_msg_proc_get_resp_size(struct thingset_msg *msg)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->response_size_hint;
}

/**
 * @brief Set response size hint in message processing data.
 *
 * A request message may contain a hint of the suitable size for a response message.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] resp_size Response size. 0 indicates no hint.
 */
static inline void ts_msg_proc_set_resp_size(struct thingset_msg *msg, uint16_t resp_size)
{
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->response_size_hint = resp_size;
}

/**
 * @} <!-- ts_msg_api_proc_priv -->
 */

/**
 * @brief Message JSON encoder support.
 *
 * @defgroup ts_msg_api_json_enc_priv ThingSet message JSON encoder (private interface)
 * @{
 */

/** @brief Scratchpad for JSON encoding */
#define TS_MSG_SCRATCHPAD_JSON_ENC  0x2U

/**
 * @brief Data structure for JSON encoder.
 */
struct ts_msg_json_encoder {
    /** @brief Number of remaining elements for a map or array. */
    uint16_t remaining;
    struct {
        /** @brief Encoder is encoding a map. */
        uint16_t map : 1;
        /** @brief Encoder is encoding an unbounded map. */
        uint16_t map_unbounded : 1;
        /** @brief Encoder is encoding an array. */
        uint16_t array : 1;
        /** @brief Encoder is encoding an unbounded array. */
        uint16_t array_unbounded : 1;
        /** @brief Next value is a key. */
        uint16_t key : 1;
    } flags;
};

/**
 * @brief Data structure for JSON encoder scratchroom at end of message.
 */
struct ts_msg_json_enc_scratchroom {
    /** @brief String buffer for conversion. */
    char s[32];
    uint8_t current;
    struct ts_msg_json_encoder instance[3];
};

/**
 * @def TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to JSON encoder scratchpad of message.
 *
 * @note Macro asserts on wrong message or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to JSON encoder scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(name, msg)                                          \
    TS_MSG_ASSERT_SCRATCHTYPE(msg, TS_MSG_SCRATCHPAD_JSON_ENC);                                     \
    /* ts_msg_tailroom() accounts for scratchroom */                                            \
    struct ts_msg_json_enc_scratchroom *name =                                                  \
        (struct ts_msg_json_enc_scratchroom *)(ts_msg_tail(msg) + ts_msg_tailroom(msg))

/**
 * @brief Setup message buffer for JSON encoding.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_json_enc_setup(struct thingset_msg *msg);

/**
 * @brief Get current Encoder of JSON encoder scratchpad.
 *
 * @param[in] msg Pointer to the message.
 * @return current encoder
 */
static inline struct ts_msg_json_encoder *ts_msg_scratchpad_json_enc_current(struct thingset_msg *msg)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return &(scratchpad->instance[scratchpad->current]);
}

/**
 * @brief Update message data position based on JSON encoder result.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] func function that calls
 * @param[in] s JSON string to add to message.
 * @param[in] len Length of JSON string.
 * @returns 0 on success, <0 on encoder error.
 */
int ts_msg_scratchpad_json_enc_update(struct thingset_msg *msg, const char *func, const char *s, uint16_t len);

static inline bool ts_msg_scratchpad_json_enc_is_top(struct thingset_msg *msg)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->current == 0;
}

/**
 * @brief Add an array to the JSON message.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] num_elements Number of elements in the array. 0 indicates an unbounded array.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_array_json(struct thingset_msg* msg, uint16_t num_elements);

/**
 * @brief Add array end to the JSON message.
 *
 * @param[in] msg Pointer to the message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_array_end_json(struct thingset_msg* msg);

int ts_msg_add_bool_json(struct thingset_msg* msg, bool b);

int ts_msg_add_decfrac_json(struct thingset_msg* msg, int32_t mantissa, int16_t exponent);

int ts_msg_add_f32_json(struct thingset_msg* msg, float val, int precision);

int ts_msg_add_i16_json(struct thingset_msg* msg, int16_t val);

int ts_msg_add_i32_json(struct thingset_msg* msg, int32_t val);

int ts_msg_add_i64_json(struct thingset_msg* msg, int64_t val);

/**
 * @brief Add a map to the JSON message.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] num_elements Number of elements in the map. 0 indicates an unbounded map.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_map_json(struct thingset_msg* msg, uint16_t num_elements);

/**
 * @brief Add map end to the JSON message.
 *
 * @param[in] msg Pointer to the message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_map_end_json(struct thingset_msg* msg);

int ts_msg_add_string_json(struct thingset_msg* msg, const char *s);

int ts_msg_add_u16_json(struct thingset_msg* msg, uint16_t val);

int ts_msg_add_u32_json(struct thingset_msg* msg, uint32_t val);

int ts_msg_add_u64_json(struct thingset_msg* msg, uint64_t val);

/**
 * @} <!-- ts_msg_api_json_enc_priv -->
 */

/**
 * @brief Message JSON decoder support.
 *
 * @defgroup ts_msg_api_json_dec_priv ThingSet message JSON decoder (private interface)
 * @{
 */

/** @brief Scratchpad for JSMN JSON parser */
#define TS_MSG_SCRATCHPAD_JSON_DEC  0x3U

/**
 * @brief Data structure for JSON decoder (JSMN) scratchroom at end of message.
 */
struct ts_msg_json_dec_scratchroom {
    /** @brief Index of current token for pull operation */
    uint16_t token_idx;
    /** @brief JSMN (JSON decoder) context */
    struct ts_jsmn_context jsmn;
};

/**
 * @def TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to JSMN (JSON decoder) scratchpad of message.
 *
 * @note Macro asserts on wrong message or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to JSMN (JSON decoder) scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(name, msg)                                              \
    TS_MSG_ASSERT_SCRATCHTYPE(msg, TS_MSG_SCRATCHPAD_JSON_DEC);                                     \
    /* ts_msg_tailroom() accounts for scratchroom */                                            \
    struct ts_msg_json_dec_scratchroom *name = (struct ts_msg_json_dec_scratchroom *)(ts_msg_tail(msg)  \
                                            + ts_msg_tailroom(msg))

/**
 * @brief Setup message buffer for JSON decoding/ parsing.
 *
 * Current message data position is taken as start of JSON data string.
 * JSON data string is up to end of buffer.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_json_dec_setup(struct thingset_msg *msg);

/**
 * @brief Update message data position based on JSON decoder result.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] func function that calls
 * @param[in] start Start of JSON string in message.
 * @param[in] len Length of JSON string.
 * @returns 0 on success, <0 on decoder error.
 */
int ts_msg_json_dec_update(struct thingset_msg *msg, const char *func, const char *start,
                           uint16_t len);

int ts_msg_pull_type_json(struct thingset_msg* msg, uint16_t *jsmn_type);

int ts_msg_pull_array_json(struct thingset_msg* msg, uint16_t *num_elements);

int ts_msg_pull_bool_json(struct thingset_msg* msg, bool *b);

int ts_msg_pull_array_end_json(struct thingset_msg* msg);

int ts_msg_pull_decfrac_json(struct thingset_msg* msg, int32_t *mantissa, int16_t *exponent);

int ts_msg_pull_f32_json(struct thingset_msg* msg, float *val);

int ts_msg_pull_i16_json(struct thingset_msg* msg, int16_t *val);

int ts_msg_pull_i32_json(struct thingset_msg* msg, int32_t *val);

int ts_msg_pull_i64_json(struct thingset_msg* msg, int64_t *val);

int ts_msg_pull_map_json(struct thingset_msg* msg, uint16_t *num_elements);

int ts_msg_pull_map_end_json(struct thingset_msg* msg);

int ts_msg_pull_mem_json(struct thingset_msg* msg, const uint8_t **mem, uint16_t *mem_len);

/**
 * @brief Pull a JSON text string from the JSON message.
 *
 * @param[in] msg Pointer to the message.
 * @param[out] s Pointer to text string.
 * @param[out] s_len Pointer to length of text string.
 * @returns 0 on success, <0 otherwise.
 * @return -EINVAL JSON data element is not a text string
 * @return -ENOMEM No JSON data element.
 */
int ts_msg_pull_string_json(struct thingset_msg* msg, const char **s, uint16_t *s_len);

int ts_msg_pull_u16_json(struct thingset_msg* msg, uint16_t *val);

int ts_msg_pull_u32_json(struct thingset_msg* msg, uint32_t *val);

int ts_msg_pull_u64_json(struct thingset_msg* msg, uint64_t *val);

/**
 * @} <!-- ts_msg_api_json_dec_priv -->
 */

/**
 * @brief Message CBOR encoder support.
 *
 * @defgroup ts_msg_api_cbor_enc_priv ThingSet message CBOR encoder (private interface)
 * @{
 */

/** @brief Scratchpad for TinyCBOR encoder */
#define TS_MSG_SCRATCHPAD_CBOR_ENC  0x4U

/**
 * @brief Data structure for TinyCBOR encoder scratchroom at end of message.
 */
struct ts_msg_cbor_enc_scratchroom {
    uint8_t current;
    struct CborEncoder instance[3];
};

/**
 * @def TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to TinyCBOR encoder scratchpad of message.
 *
 * @note Macro asserts on wrong message or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to TinyCBOR encoder scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(name, msg)                                          \
    TS_MSG_ASSERT_SCRATCHTYPE(msg, TS_MSG_SCRATCHPAD_CBOR_ENC);                                     \
    /* ts_msg_tailroom() accounts for scratchroom */                                            \
    struct ts_msg_cbor_enc_scratchroom *name =                                              \
        (struct ts_msg_cbor_enc_scratchroom *)(ts_msg_tail(msg) + ts_msg_tailroom(msg))

/**
 * @brief Setup message buffer for CBOR encoding.
 *
 * Current message data position is taken as start of CBOR data.
 * Complete current tailroom is reserved to encoded CBOR data.
 *
 * @note If the default assumptions about the CBOR data start and size are invalid the encoder has
 *       to be initialized again.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_cbor_enc_setup(struct thingset_msg *msg);

/**
 * @brief Get current CborEncoder of TinyCBOR encoder scratchpad.
 *
 * @param[in] msg Pointer to the message.
 * @return current encoder
 */
static inline struct CborEncoder *ts_msg_scratchpad_tinycbor_enc_current(struct thingset_msg *msg)
{
    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return &(scratchpad->instance[scratchpad->current]);
}

/**
 * @brief Update message data position based on TnyCBOR encoder result.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] func function that calls
 * @param[in] error Enocder result.
 * @returns 0 on success, <0 on encoder error.
 */
int ts_msg_scratchpad_tinycbor_enc_update(struct thingset_msg *msg, const char *func, CborError error);

static inline bool ts_msg_scratchpad_tinycbor_enc_is_top(struct thingset_msg *msg)
{
    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->current == 0;
}

int ts_msg_add_array_cbor(struct thingset_msg* msg, uint16_t num_elements);

int ts_msg_add_array_end_cbor(struct thingset_msg* msg);

int ts_msg_add_bool_cbor(struct thingset_msg* msg, bool b);

int ts_msg_add_decfrac_cbor(struct thingset_msg* msg, int32_t mantissa, int16_t exponent);

int ts_msg_add_f32_cbor(struct thingset_msg* msg, float val, int precision);

int ts_msg_add_i64_cbor(struct thingset_msg* msg, int64_t val);

static inline int ts_msg_add_i16_cbor(struct thingset_msg* msg, int16_t val)
{
    return ts_msg_add_i64_cbor(msg, (int64_t)val);
}

static inline int ts_msg_add_i32_cbor(struct thingset_msg* msg, int32_t val)
{
    return ts_msg_add_i64_cbor(msg, (int64_t)val);
}

int ts_msg_add_map_cbor(struct thingset_msg* msg, uint16_t num_elements);

int ts_msg_add_map_end_cbor(struct thingset_msg* msg);

int ts_msg_add_mem_cbor(struct thingset_msg* msg, const uint8_t *mem, uint16_t len);

int ts_msg_add_string_cbor(struct thingset_msg* msg, const char *s);

int ts_msg_add_undefined_cbor(struct thingset_msg* msg);

int ts_msg_add_u64_cbor(struct thingset_msg* msg, uint64_t val);

static inline int ts_msg_add_u16_cbor(struct thingset_msg* msg, uint16_t val)
{
    return ts_msg_add_u64_cbor(msg, (uint64_t)val);
}

static inline int ts_msg_add_u32_cbor(struct thingset_msg* msg, uint32_t val)
{
    return ts_msg_add_u64_cbor(msg, (uint64_t)val);
}

/**
 * @} <!-- ts_msg_api_cbor_enc_priv -->
 */

/**
 * @brief Message CBOR decoder support.
 *
 * @defgroup ts_msg_api_cbor_dec_priv ThingSet message CBOR decoder (private interface)
 * @{
 */

/** @brief Scratchpad for TinyCBOR decoder/parser */
#define TS_MSG_SCRATCHPAD_CBOR_DEC  0x5U

/**
 * @brief Data structure for TinyCBOR decoder scratchroom at end of message.
 */
struct ts_msg_cbor_dec_scratchroom {
    uint8_t current;
    struct CborParser parser;
    struct CborValue value[3];
};

/**
 * @def TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT
 *
 * @brief Define and initialize pointer to TinyCBOR decoder scratchpad of message.
 *
 * @note Macro asserts on wrong message or scratchpad type if TS_CONFIG_ASSERT is enabled.
 *
 * @param[in] name Object name of pointer to TinyCBOR decoder scratchpad.
 * @param[in] msg Pointer to the message.
 */
#define TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(name, msg)                                          \
    TS_MSG_ASSERT_SCRATCHTYPE(msg, TS_MSG_SCRATCHPAD_CBOR_DEC);                                     \
    /* ts_msg_tailroom() accounts for scratchroom */                                            \
    struct ts_msg_cbor_dec_scratchroom *name =                                              \
        (struct ts_msg_cbor_dec_scratchroom *)(ts_msg_tail(msg) + ts_msg_tailroom(msg))

/**
 * @brief Setup message buffer for CBOR decoding.
 *
 * Current message data position is taken as start of CBOR data.
 * CBOR data size is up to end of buffer.
 *
 * @note If the default assumptions about the CBOR data start and size are invalid the parser has
 *       to be initialized again:
 *          TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(decoder, msg);
 *          cbor_parser_init(new_start, new_size, new_flags, &decoder->parser, &decoder->value);
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_cbor_dec_setup(struct thingset_msg *msg);

/**
 * @brief Get current CborValue iterator of TinyCBOR decoder scratchpad.
 *
 * @param[in] msg Pointer to the message.
 * @return current iterator
 */
static inline struct CborValue *ts_msg_scratchpad_tinycbor_dec_current(struct thingset_msg *msg)
{
    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return &(scratchpad->value[scratchpad->current]);
}

/**
 * @brief Update message data position based on decoder result.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] advance_value advance decoder to next CBOR value
 * @param[in] func function that calls
 * @param[in] error Deocder result.
 * @returns 0 on success, <0 on decoder error.
 */
int ts_msg_scratchpad_tinycbor_dec_update(struct thingset_msg *msg, bool advance_value,
                                          const char *func, CborError error);

static inline bool ts_msg_scratchpad_tinycbor_dec_is_top(struct thingset_msg *msg)
{
    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    return scratchpad->current == 0;
}


int ts_msg_pull_type_cbor(struct thingset_msg* msg, CborType *cbor_type);

int ts_msg_pull_array_cbor(struct thingset_msg* msg, uint16_t *num_elements);

int ts_msg_pull_array_end_cbor(struct thingset_msg* msg);

int ts_msg_pull_bool_cbor(struct thingset_msg* msg, bool *b);

int ts_msg_pull_decfrac_cbor(struct thingset_msg* msg, int32_t *mantissa, int16_t *exponent);

int ts_msg_pull_f32_cbor(struct thingset_msg* msg, float *val);

int ts_msg_pull_i16_cbor(struct thingset_msg* msg, int16_t *val);

int ts_msg_pull_i32_cbor(struct thingset_msg* msg, int32_t *val);

int ts_msg_pull_i64_cbor(struct thingset_msg* msg, int64_t *val);

int ts_msg_pull_map_cbor(struct thingset_msg* msg, uint16_t *num_elements);

int ts_msg_pull_map_end_cbor(struct thingset_msg* msg);

int ts_msg_pull_mem_cbor(struct thingset_msg* msg, const uint8_t **mem, uint16_t *len);

/**
 * @brief Pull a CBOR text string from the message.
 *
 * Pull definitive length text string. Return error if the string is not of definitive length
 * or the data element is not a text string.
 *
 * @param[in] msg Pointer to the message.
 * @param[out] s Pointer to text string.
 * @param[out] len Pointer to length of text string.
 * @returns 0 on success, <0 otherwise.
 * @return -EINVAL CBOR data element is not a definitive length test string
 * @return -ENOMEM No CBOR data element.
 */
int ts_msg_pull_string_cbor(struct thingset_msg* msg, const char **s, uint16_t *len);

int ts_msg_pull_u16_cbor(struct thingset_msg* msg, uint16_t *val);

int ts_msg_pull_u32_cbor(struct thingset_msg* msg, uint32_t *val);

int ts_msg_pull_u64_cbor(struct thingset_msg* msg, uint64_t *val);

/**
 * @} <!-- ts_msg_api_cbor_dec_priv -->
 */

/**
 * @brief Message COBS encoder support.
 *
 * @defgroup ts_msg_api_cobs_enc_priv ThingSet message COBS encoder (private interface)
 * @{
 */

/** @brief Scratchpad for COBS encoder */
#define TS_MSG_SCRATCHPAD_COBS_ENC  0x6U

/**
 * @brief Setup message buffer for COBS encoding.
 *
 * Current message data position is taken as start of (decoded) data to be COBS encoded.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_cobs_enc_setup(struct thingset_msg *msg);

/**
 * @} <!-- ts_msg_api_cobs_enc_priv -->
 */

/**
 * @brief Message COBS decoder support.
 *
 * @defgroup ts_msg_api_cobs_dec_priv ThingSet message COBS decoder (private interface)
 * @{
 */

/** @brief Scratchpad for COBS decoder */
#define TS_MSG_SCRATCHPAD_COBS_DEC  0x7U

/**
 * @brief Setup message buffer for COBS decoding.
 *
 * Current message data position is taken as start of COBS encoded data to be decoded.
 *
 * @param[in] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int ts_msg_cobs_dec_setup(struct thingset_msg *msg);

/**
 * @} <!-- ts_msg_api_cobs_enc_priv -->
 */

/*
 * ThingSet message primitive value support
 * ----------------------------------------
 */

static inline uint8_t *ts_msg_add(struct thingset_msg* msg, uint16_t len)
{
    return ts_buf_add((struct ts_buf *)msg, len);
}

int ts_msg_add_mem(struct thingset_msg* msg, const uint8_t *mem, uint16_t len);

int ts_msg_add_u8(struct thingset_msg* msg, uint8_t val);

static inline uint8_t *ts_msg_pull(struct thingset_msg* msg, uint16_t len)
{
    TS_ASSERT(len <= ts_msg_len(msg), "Pull %u but only %u in message.",
              (unsigned int)len, (unsigned int)ts_msg_len(msg));
    return ts_buf_pull((struct ts_buf *)msg, len);
}

int ts_msg_pull_u8(struct thingset_msg* msg, uint8_t *val);

int ts_msg_pull_path(struct thingset_msg* msg, const char **path, uint16_t *len);

static inline uint8_t *ts_msg_push(struct thingset_msg* msg, uint16_t len)
{
    TS_ASSERT(len <= ts_msg_headroom(msg),
              "Push %u but only %u available in headroom of message 0x%" PRIXPTR ".",
              (unsigned int)len, (unsigned int)ts_msg_headroom(msg), (uintptr_t)msg);
    return ts_buf_push((struct ts_buf *)msg, len);
}

static inline uint8_t *ts_msg_remove(struct thingset_msg* msg, uint16_t len)
{
    TS_ASSERT(len <= ts_msg_len(msg), "Remove %u but only %u in message 0x%" PRIXPTR ".",
              (unsigned int)len, (unsigned int)ts_msg_len(msg), (uintptr_t)msg);
    return ts_buf_remove((struct ts_buf *)msg, len);
}

/*
 * Logging support
 * ---------------
 */

/**
 * @brief Print the message to log buffer.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] log Pointer to the log buffer.
 * @param[in] len Length of the log buffer.
 * @returns Number of characters printed to buffer.
 */
int ts_msg_log(struct thingset_msg* msg, char *log, size_t len);

/*
 * ThingSet export and import support
 * ----------------------------------
 */

/**
 * @brief Add export data in CBOR format for given subset(s).
 *
 * This function creates a specific export message that may later on be used to import the data.
 *
 * @note The function may be used to store data in the EEPROM or other non-volatile memory.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] did Data objects database ID.
 * @param[in] subsets Flags to select which subset(s) of data items should be exported.
 * @returns 0 in case of siccess, <0 on error.
 */
int ts_msg_add_export_cbor(struct thingset_msg* msg, ts_obj_db_id_t did, uint16_t subsets);

/**
 * @brief Add export data in JSON format for given subset(s).
 *
 * This function creates a specific export message that may later on be used to import the data.
 *
 * @note The function may be used to store data in the EEPROM or other non-volatile memory.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] did Data objects database ID.
 * @param[in] subsets Flags to select which subset(s) of data items should be exported.
 * @returns 0 in case of success, <0 on error.
 */
int ts_msg_add_export_json(struct thingset_msg* msg, ts_obj_db_id_t did, uint16_t subsets);

/**
 * @brief Remove export data from message and fill into data objects.
 *
 * @note The function may be used to initialize data objects from previously exported data.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] did Data objects database ID.
 * @returns 0 in case of success, <0 on error.
 */
int ts_msg_pull_export(struct thingset_msg* msg, ts_obj_db_id_t did);


/**
 * @brief Load message from buffer.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] buf Pointer to the buffer where the message is stored.
 * @param[in] buf_len Length of message in buffer.
 * @returns 0 in case of success, <0 on error.
 */
int ts_msg_load(struct thingset_msg* msg, const uint8_t *buf, uint16_t buf_len);

/**
 * @brief Save message to buffer.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] buf Pointer to the buffer where the message shall be stored.
 * @param[in,out] buf_len Length of the buffer. Maximum allowed length of the data on invocation.
 *                        Message data length on return.
 * @returns 0 in case of success, <0 on error.
 */
int ts_msg_save(struct thingset_msg* msg, uint8_t *buf, uint16_t *buf_len);

/*
 * ThingSet Protocol support
 * -------------------------
 */

/**
 * @brief ThingSet message protocol support
 *
 * @defgroup ts_msg_api_proto_priv ThingSet message protocol support (private interface)
 * @{
 */

/**
 * @brief Add ThingSet protocol status code at the end of the message.
 *
 * A status-code = 2( hex ).
 *
 * @param[in] msg Pointer to the message.
 * @param[in] code Status code
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_status(struct thingset_msg* msg, ts_msg_status_code_t code);

/**
 * @brief Add ThingSet object at the end of the message in CBOR format.
 *
 * Increments the data length of the message to account for more data added at the end.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object which shall be serialized.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_object_cbor(struct thingset_msg *msg, thingset_oref_t oref);

/**
 * @brief Add ThingSet object at the end of the message in JSON format.
 *
 * Increments the data length of the message to account for more data added at the end.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object which shall be serialized.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_object_json(struct thingset_msg *msg, thingset_oref_t oref);

/**
 * @brief Add ThingSet protocol GET request in CBOR format.
 *
 * The object to get is either specified by it's parent object identifier
 * or by the path.
 *
 * If path is NULL the object identifier will be used.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] object_id Identifier of object to get.
 * @param[in] path Pointer to path of object to get.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_request_get_cbor(struct thingset_msg* msg, ts_obj_id_t object_id, const char *path);

/**
 * @brief Add ThingSet protocol GET request in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] path Pointer to path of object to get.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_request_get_json(struct thingset_msg *msg, const char *path);

/**
 * @brief Add ThingSet protocol response to GET request in CBOR format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to parent object.
 * @param[in] request Pointer to the request message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_get_cbor(struct thingset_msg* msg, thingset_oref_t oref,
                                 struct thingset_msg* request);

/**
 * @brief Add ThingSet protocol response to GET request in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to parent object.
 * @param[in] request Pointer to the request message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_get_json(struct thingset_msg* msg, thingset_oref_t oref,
                                 struct thingset_msg* request);

/**
 * @brief Add ThingSet protocol FETCH request in CBOR format.
 *
 * The parent object of the objects to fetch is either specified by it's parent object identifier
 * or by the path.
 *
 * If path is NULL the object identifier will be used.
 *
 * The objects are either specified by the object identifiers or by their names.
 *
 * If names is NULL the object identifiers will be used.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] object_id Identifier of the parent object to fetch from.
 * @param[in] path Pointer to path of parent object to fetch from.
 * @param[in] object_count Number of objects to fetch.
 * @param[in] object_ids Array of object identifiers of the objects to fetch.
 * @param[in] object_names Array of names of the objects to fetch.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_request_fetch_cbor(struct thingset_msg* msg, ts_obj_id_t object_id, const char *path,
                                  uint16_t object_count, ts_obj_id_t *object_ids,
                                  const char **object_names);

/**
 * @brief Add ThingSet protocol FETCH request in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] path Pointer to path of parent object to fetch from.
 * @param[in] object_count Number of objects to fetch.
 * @param[in] object_names Array of names of the objects to fetch.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_request_fetch_json(struct thingset_msg* msg, const char *path,
                                  uint16_t object_count, const char **object_names);

/**
 * @brief Add ThingSet protocol response to FETCH request in CBOR format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to parent object.
 * @param[in] request Pointer to the request message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_fetch_cbor(struct thingset_msg* msg, thingset_oref_t oref,
                                   struct thingset_msg* request);

/**
 * @brief Add ThingSet protocol response to FETCH request in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to parent object.
 * @param[in] request Pointer to the request message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_fetch_json(struct thingset_msg* msg, thingset_oref_t oref,
                                   struct thingset_msg* request);

/**
 * @brief Add ThingSet protocol status response in CBOR format.
 *
 * Status code is taken from message status of @p msg.
 *
 * Switches message scratchpad to CBOR encoding.
 *
 * @param[in] msg Pointer to the message.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_status_cbor(struct thingset_msg* msg);

/**
 * @brief Add ThingSet protocol status response in JSON format.
 *
 * Status code is taken from message status of @p msg.
 *
 * Switches message scratchpad to JSON encoding.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] code Status code
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_response_status_json(struct thingset_msg* msg);

/**
 * @brief Add ThingSet protocol statement in CBOR format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_statement_cbor(struct thingset_msg* msg, thingset_oref_t oref);

/**
 * @brief Add ThingSet protocol statement in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_add_statement_json(struct thingset_msg* msg, thingset_oref_t oref);

/**
 * @brief Remove ThingSet object from the message in CBOR format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object which shall be deserialized.
 * @param[in] patch If true patch object, otherwise only check.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_pull_object_cbor(struct thingset_msg *msg, thingset_oref_t oref, bool patch);

/**
 * @brief Remove ThingSet object from the message in JSON format.
 *
 * @param[in] msg Pointer to the message.
 * @param[in] oref Database object reference to object which shall be deserialized.
 * @param[in] patch If true patch object, otherwise only check.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_pull_object_json(struct thingset_msg *msg, thingset_oref_t oref, bool patch);

/**
 * @brief Remove ThingSet protocol request in CBOR format from message.
 *
 * Parse the CBOR request, set request message status and provide object of request.
 *
 * @param[in] msg Pointer to the message.
 * @param[in,out] oref Reference to object. ThingSet object database must be set in reference.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_pull_request_cbor(struct thingset_msg* msg, thingset_oref_t *oref);

/**
 * @brief Remove ThingSet protocol request in JSON format from message.
 *
 * Parse the JSON request and provide the request status and object of request.
 *
 * @param[in] msg Pointer to the message.
 * @param[in,out] oref Reference to object. ThingSet object database must be set in reference.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_pull_request_json(struct thingset_msg* msg, thingset_oref_t *oref);

/**
 * @brief Remove ThingSet protocol status code from message.
 *
 * A status-code = 2( hex ).
 * A status description ends by a trailing '.'
 *
 * @param[in] msg Pointer to the message.
 * @param[out] code Pointer to status code.
 * @param[out] description Pointer to description.
 * @param[out] len Length of the description including trailing '.'.
 * @returns 0 on success, <0 otherwise.
 */
int ts_msg_pull_status(struct thingset_msg* msg, ts_msg_status_code_t *code,
                       const char **description,  uint16_t *len);

/**
 * @} <!-- ts_msg_api_proto_priv -->
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_msg_api_priv -->
 */

#endif /* TS_MSG_H_ */
