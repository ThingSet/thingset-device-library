/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet message (public interface)
 */

#ifndef THINGSET_MSG_H_
#define THINGSET_MSG_H_

/**
 * @brief ThingSet message (buffers).
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_msg_api_pub ThingSet message (public interface)
 * @{
 */

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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ThingSet message (buffer).
 *
 * A ThingSet message buffer is build on top of generic buffers.
 *
 * A thingset message buffer can not be created directly. It always has to be allocated.
 */
struct thingset_msg {};


/**
 * @brief Allocate a ThingSet message buffer from the buffer pool.
 *
 * The message buffer is allocated with reference count set to 1.
 *
 * Allocation accounts for scratchpads minimum size.
 *
 * @param[in] size The size of the message buffer
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int thingset_msg_alloc(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg);

/**
 * @brief Allocate a ThingSet message buffer for binary messages from the buffer pool.
 *
 * The message buffer is allocated with reference count set to 1.
 *
 * Allocation accounts for CBOR scratchpads minimum size.
 *
 * @param[in] msg_size Maximum size of the binary message,
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int thingset_msg_alloc_cbor(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg);

/**
 * @brief Allocate a ThingSet message buffer for text messages from the buffer pool.
 *
 * The message buffer is allocated with reference count set to 1.
 *
 * Allocation accounts for JSON scratchpads minimum size.
 *
 * @param[in] msg_size Maximum size of the text message,
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to message buffer
 * @return 0 on success, <0 otherwise
 */
int thingset_msg_alloc_json(uint16_t msg_size, thingset_time_ms_t timeout_ms, struct thingset_msg **msg);

/**
 * @brief Clone message.
 *
 * Duplicate given message communication buffer including any data currently stored.
 *
 * @param[in] msg Pointer to the source message communication buffer.
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] clone Pointer to the cloned buffer.
 * @return The original tail of the destinatio buffer, before incremented by new data.
 */
int thingset_msg_clone(struct thingset_msg *msg, thingset_time_ms_t timeout_ms, struct thingset_msg **clone);


/**
 * @brief Reset message buffer.
 *
 * Reset buffer data so the message buffer can be reused for other purposes.
 *
 * @param[in] msg Pointer to the message.
 */
void thingset_msg_reset(struct thingset_msg *msg);

/**
 * @brief Mark message used.
 *
 * Increment the reference count of a message.
 *
 * @param[in] msg Pointer to the message.
 * @return 0 on success, <0 otherwise
 */
int thingset_msg_ref(struct thingset_msg *msg);

/**
 * @brief Mark message unused.
 *
 * Decrement the reference count of a message. The message is put back into the
 * message buffer pool if the reference count reaches zero.
 *
 * @note The message shall not be accessed after it is marked unused.
 *
 * @param[in] msg Pointer to the message.
 * @return 0 on success, <0 otherwise
 */
int thingset_msg_unref(struct thingset_msg *msg);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_msg_api_pub -->
 */

#endif /* THINGSET_MSG_H_ */
