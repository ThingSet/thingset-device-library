/*
 * Copyright (c) 2020..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet remote two-way queue (public interface)
 */

#ifndef THINGSET_RBBQ_H_
#define THINGSET_RBBQ_H_

/**
 * @brief ThingSet remote two-way queue.
 *
 * The remote two-way queue connects the local end of the queue with a remote end. It abstracts
 * away the communication between the queue ends.
 *
 * The remote two-way queue is made up of two lock free ring buffers providing contiguous
 * reservations of data blocks. One ring buffer is for transmit, the other one for receive.
 *
 * @defgroup ts_rbbq_api_pub ThingSet remote two-way queue (public interface)
 * @{
 */

#include "thingset_env.h"
#include "thingset_time.h"

/* forward declaration */
struct thingset_rbbq;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate a data block in the transmit ring buffer.
 *
 * The data block has to be transmitted to hand it over to the remote side.
 *
 * @note A data block that is allocated but not transmitted blocks allocation.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @param[in] channel Message channel
 * @param[in] size The size of the data block
 * @param[out] data Pointer to memory allocated to the data block
 * @param[in] timeout_ms maximum time to wait in milliseconds
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_alloc(struct thingset_rbbq *queue, uint16_t channel, uint16_t size, uint8_t **data,
                        thingset_time_ms_t timeout_ms);


/**
 * @brief Transmit allocated data block.
 *
 * @note The data block shall not be accessed after it is transmitted.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @param[in] data Pointer to the data block
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_transmit(struct thingset_rbbq *queue, uint8_t *data);


/**
 * @brief Receive data block.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @param[out] channel Message channel
 * @param[out] size The size of the data block
 * @param[out] data Pointer to the data block
 * @param[in] timeout_ms maximum time to wait
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_receive(struct thingset_rbbq *queue, uint16_t *channel, uint16_t *size,
                          uint8_t **data, thingset_time_ms_t timeout_ms);


/**
 * @brief Free received data block from the receive ring buffer.
 *
 * @note The data block shall not be accessed after it is freed.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @param[in] data Pointer to the data block
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_free(struct thingset_rbbq *queue, uint8_t *data);


/**
 * @brief Initialise a queue.
 *
 * Buffer must be initailized before it can be used.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_init(struct thingset_rbbq *queue);


/**
 * @brief Start data exchange on queue.
 *
 * Buffer must be initailized before data exchange can be started.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_start(struct thingset_rbbq *queue);


/**
 * @brief Stop data exchange on queue.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @return 0 on success, <0 otherwise
 */
int thingset_rbbq_stop(struct thingset_rbbq *queue);


/**
 * @brief Wait for next receive transfer.
 *
 * @param[in] queue Pointer to the queue
 * @param timeout_ms maximum time to wait
 */
int thingset_rbbq_wait_receive(struct thingset_rbbq *queue, thingset_time_ms_t timeout_ms);


/**
 * @brief Wait for next transmit transfer.
 *
 * @param[in] queue Pointer to the queue
 * @param timeout_ms maximum time to wait
 */
int thingset_rbbq_wait_transmit(struct thingset_rbbq *queue, thingset_time_ms_t timeout_ms);


/**
 * @brief Monitor rbbq communcation for health.
 *
 * @param[in] queue Pointer to the remote two-way queue
 * @return 0 on health, <0 otherwise
 */
int thingset_rbbq_monitor(struct thingset_rbbq *queue);


/**
 * @brief Name of the queue.
 *
 * @param[in] queue Pointer to the queue
 * @return Pointer to name, 0 if no name found.
 */
const char *thingset_rbbq_name(struct thingset_rbbq *queue);


/**
 * @brief Retrieve the queue by name.
 *
 * The call locks the queue to the current thread.
 *
 * @param[in] name Buffer name to search for.
 * @return pointer to queue structure; NULL if not found or cannot be used.
 */
struct thingset_rbbq *rbbq_get_binding(const char *name);


#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_rbbq_api_pub -->
 */

/**
 * @page ts_topic_rbbq Remote two-way queue
 *
 * RBBQ is a Single Producer Single Consumer, lockless, no_std, thread safe, two-way queue.
 *
 * The remote two-way queue connects the local end of the queue with a remote end. It abstracts
 * away the communication between the queue ends.
 *
 * The two-way queue is made of two lock free ring buffers with contiguous reservations.
 * The ring buffers are implementations of the [lock-free ring-buffer with contiguous reservations]
 * (https://blog.systems.ethz.ch/blog/2019/the-design-and-implementation-of-a-lock-free-ring-buffer-with-contiguous-reservations.html)
 * described by Andrea Lattuada and James Munns.
 *
 * RBBQ is designed (primarily) to be a First-In, First-Out queue for use with DMA on embedded
 * systems. While Circular/ Ring Buffers allow you to send data between two threads (or from an
 * interrupt to main code), you must push the data one piece at a time. With RBBQ, you instead are
 * granted a block of contiguous memory, which can be filled (or emptied) by a DMA engine.
 *
 * RBBQ works on communication ports. Data may not only be exchanged between threads sharing the
 * ring buffer memory but also between devices that are connected by some kind of communication.
 * This mainly targets communication where you can exchange fixed size (one ring buffer size) data
 * packets to synchronize the ring buffers of the remote two-way queues (e.g. by SPI).
 * Here you gain the benefit of DMA transfers between the devices and DMA transfers to fill or empty
 * the ring buffers.
 */

#endif /* THINGSET_RBBQ_H_ */
