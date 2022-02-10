/*
 * Copyright (c) 2020..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet remote two-way queue (private interface)
 */

#ifndef TS_RBBQ_H_
#define TS_RBBQ_H_

/**
 * @brief ThingSet remote two-way queue.
 *
 * The two-way queue is based on two lock free ring buffers with contiguous reservations.
 *
 * Buffer architecture.
 *
 * Transmit buffer:
 * - buffer data
 *   - raw data blocks
 *     - header (big endian)
 *     - payload == data block
 * - buffer control (big endian)
 *
 * Receive buffer:
 * - buffer data
 *   - raw data blocks
 *     - header (big endian)
 *     - payload == data block
 * - buffer control (big endian)
 *
 * @defgroup ts_rbbq_api_priv ThingSet remote two-way queue (private interface)
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "thingset_rbbq.h"

#include "ts_endian.h"

/* Queue states */
#define TS_RBBQ_QUEUE_STATE_NONE      0
#define TS_RBBQ_QUEUE_STATE_INIT      1
#define TS_RBBQ_QUEUE_STATE_READY     2
#define TS_RBBQ_QUEUE_STATE_START     3
#define TS_RBBQ_QUEUE_STATE_RUNNING   4
#define TS_RBBQ_QUEUE_STATE_STOP      5
#define TS_RBBQ_QUEUE_STATE_SUSPENDED 6

/**
 * @brief Buffer control
 *
 * Buffer control is part of the transmit ring buffer and the receive ring buffer.
 * It is transmitted to the other side of the communication together with the data.
 *
 * @note Shall be initialised by the device on init.
 */
struct ts_rbbq_buffer_control {
    uint16_t this_write_idx;        /**< write index on this ring buffer */
    uint16_t this_watermark_idx;    /**< watermark index on this ring buffer */
    uint16_t other_read_idx;        /**< read index on other ring buffer */
};

/*
 * Port architectur:
 *
 * Queues work on ports.
 */

/**
 * @brief ThingSet remote two-way queue port API
 */
struct ts_rbbq_port_api {
    /**
     * @brief queue requests port to initialise.
     */
    int (*init)(struct thingset_rbbq *queue);
    /**
     * @brief queue requests port to start data exchange.
     */
    int (*start)(struct thingset_rbbq *queue);
    /**
     * @brief queue requests port to stop data exchange.
     */
    int (*stop)(struct thingset_rbbq *queue);
    /**
     * @brief queue informs port that a new transmit data block is available.
     *
     * The port shall take:
     * - queue->alloc_watermark_idx
     * - queue->alloc_write_idx
     * and update at a convenient time (if transmit buffer is not locked by transfer op):
     * - queue->port->tx_control->this_watermark_idx
     * - queue->port->tx_control->this_write_idx
     *
     * The call shall be non-blocking.
     */
    int (*transmit)(struct thingset_rbbq *queue);
    /**
     * @brief queue informs port that a received data block was freed.
     *
     * The port shall take:
     * - queue->free_read_idx
     * and update at a convenient time (if transmit buffer is not locked by transfer op):
     * - queue->port->tx_control->other_read_idx
     *
     * The call shall be non-blocking.
     */
    int (*receive)(struct thingset_rbbq *queue);
    /**
     * @brief queue requests port to monitor data exchange.
     */
    int (*monitor)(struct thingset_rbbq *queue);
};

/**
 * @brief Port management structure.
 */
struct ts_rbbq_port {
    const struct ts_rbbq_port_api *port_api;
    struct ts_rbbq_buffer_control *tx_control;
    struct ts_rbbq_buffer_control *rx_control;
    uint16_t tx_data_size;
    uint8_t  *tx_data;
    uint16_t rx_data_size;
    uint8_t  *rx_data;
};

/**
 * @brief Remote two-way queue management structure.
 *
 * @note Shall be initialised by the queue on init.
 */
struct thingset_rbbq {
    struct thingset_rbbq *next;     /**< Manage all queues */
    const char *name;               /**< Name of the queue */

    /* Port signal on receive */
    pthread_cond_t port_receive_cond;     /**< Condition var for port to signal new receive */
    pthread_mutex_t port_receive_mutex;   /**< Mutex to lock access to receive condition var */
    /* Port signal on transmit */
    pthread_cond_t port_transmit_cond;    /**< Condition var for port to signal new transmit */
    pthread_mutex_t port_transmit_mutex;  /**< Mutex to lock access to transmit condition var */

    const struct ts_rbbq_port *port;

    pthread_mutex_t alloc_mutex;    /**< Mutex to lock allocation operation up to transmit */
    uint8_t *alloc_data;            /**< Data block currently allocated */
    uint16_t alloc_write_idx;       /**< write index of allocated data on tx buffer */
    uint16_t alloc_watermark_idx;   /**< watermark index of allocated data on tx buffer */
    atomic_uint_fast16_t tx_shadow_read_idx;    /**< Shadow read idx on tx buffer */

    pthread_mutex_t receive_mutex;  /**< Mutex to lock receive operation from receive to free */
    uint8_t *free_data;             /**< Data block currently allocated */
    uint16_t free_read_idx;         /**< read index of freed data on rx buffer */

    atomic_uint_fast8_t state;      /**< Queue state. */
};


/*
 * Data block architectur:
 *
 * Header:
 * - channel: 2 bytes in big endian byte order
 * - payload size: 2 bytes in big endian byte order
 * Payload:
 * - data: N bytes
 */

/**
 * @brief Data block header
 */
struct ts_rbbq_data_header {
    uint16_t channel;
    uint16_t size;
};

/**
 * @brief Get the state of the queue.
 *
 * Atomic get of the queue state.
 */
static inline uint8_t ts_rbbq_state(struct thingset_rbbq *queue)
{
    return atomic_load(&queue->state);
}


/**
 * @brief Set the state of the queue.
 *
 * Atomic set of the queue state.
 */
static inline void ts_rbbq_state_set(struct thingset_rbbq *queue, uint8_t state)
{
    atomic_store(&queue->state, state);
}

/**
 * @brief Compare and set the state of the queue.
 *
 * This routine performs an atomic compare-and-set on the queue state.
 *
 * If the current value of the queue state equals old_state, the queue state is set to new_state.
 * If the current value of the queue state does not equal old_state, the queue state
 * is left unchanged.
 *
 * @param queue
 * @param old_state Original state to compare against.
 * @param new_state New state to store.
 * @return True if queue state is set to new_state, false otherwise.
 */
static inline bool ts_rbbq_state_cas(struct thingset_rbbq *queue, uint8_t old_state, uint8_t new_state)
{
    return atomic_compare_exchange_strong(&queue->state, &old_state, new_state);
}

/**
 * @brief Get the channel of the raw data block.
 */
static inline uint16_t rbbq_data_channel(uint8_t *raw_data)
{
    struct ts_rbbq_data_header *header = (struct ts_rbbq_data_header *)raw_data;

    return ts_endian_get_be16((const uint8_t *)&header->channel);
}


/**
 * @brief Set the channel of the raw data block.
 */
static inline void rbbq_data_channel_set(uint8_t *raw_data, uint16_t channel)
{
    struct ts_rbbq_data_header *header = (struct ts_rbbq_data_header *)raw_data;

    sys_put_be16(channel, (uint8_t *)&header->channel);
}


/**
 * @brief Get the payload size of the raw data block.
 */
static inline uint16_t rbbq_data_payload_size(uint8_t *raw_data)
{
    struct ts_rbbq_data_header *header = (struct ts_rbbq_data_header *)raw_data;

    return ts_endian_get_be16((const uint8_t *)&header->size);
}


/**
 * @brief Set the payload size of the raw data block.
 */
static inline void rbbq_data_payload_size_set(uint8_t *raw_data, uint16_t size)
{
    struct ts_rbbq_data_header *header = (struct ts_rbbq_data_header *)raw_data;

    sys_put_be16(size, (uint8_t *)&header->size);
}


/**
 * @brief Get the payload of the raw data block.
 */
static inline uint8_t *rbbq_data_payload(uint8_t *raw_data)
{
    return raw_data + sizeof(struct ts_rbbq_data_header);
}


/**
 * @brief Get the size of the raw data block.
 */
static inline uint16_t rbbq_data_size(uint8_t *raw_data)
{
    struct ts_rbbq_data_header *header = (struct ts_rbbq_data_header *)raw_data;

    return ts_endian_get_be16((const uint8_t *)&header->size) + sizeof(struct ts_rbbq_data_header);
}


/**
 * @brief Calculate the size of the raw data block.
 *
 * @param payload_size The size of the payload == user data block.
 */
static inline uint16_t rbbq_data_size_calc(uint16_t payload_size)
{
    return payload_size + sizeof(struct ts_rbbq_data_header);
}

/**
 * @brief Get rbbq control.
 *
 * @param[in] control Pointer to RBBQ control struct
 * @param[out] this_write_idx Write index of this buffer
 * @param[out] this_watermark_idx Watermark index of this buffer
 * @param[out] other_read_idx Read index of the other buffer
 */
static inline void rbbq_control(const struct ts_rbbq_buffer_control *control,
                                uint16_t *this_write_idx,
                                uint16_t *this_watermark_idx,
                                uint16_t *other_read_idx)
{
    *this_write_idx = ts_endian_get_be16((const uint8_t *)&control->this_write_idx);
    *this_watermark_idx = ts_endian_get_be16((const uint8_t *)&control->this_watermark_idx);
    *other_read_idx = ts_endian_get_be16((const uint8_t *)&control->other_read_idx);
}

/**
 * @brief Get other read index of rbbq control.
 *
 * @param[in] control Pointer to RBBQ control struct
 * @return Read index of the other buffer
 */
static inline uint16_t rbbq_control_other_read_idx(const struct ts_rbbq_buffer_control *control)
{
    return ts_endian_get_be16((const uint8_t *)&control->other_read_idx);
}


/**
 * @brief Set rbbq control.
 *
 * @param control Pointer to RBBQ control struct
 * @param this_write_idx Write index of this buffer
 * @param this_watermark_idx Watermark index of this buffer
 * @param other_read_idx Read index of the other buffer
 */
static inline void rbbq_control_set(struct ts_rbbq_buffer_control *control,
                                    uint16_t this_write_idx,
                                    uint16_t this_watermark_idx,
                                    uint16_t other_read_idx)
{
    sys_put_be16(this_write_idx, (uint8_t *)&control->this_write_idx);
    sys_put_be16(this_watermark_idx, (uint8_t *)&control->this_watermark_idx);
    sys_put_be16(other_read_idx, (uint8_t *)&control->other_read_idx);
}

/**
 * @brief Lock receive buffer.
 *
 * Used internally but also to be called by a rbbq port
 * if the receive buffer can temporarily not be accessed
 * (e.g. due to a non lock free update).
 */
static inline int rbbq_receive_lock(struct thingset_rbbq *queue, thingset_time_ms_t timeout_ms)
{
    struct timespec timeout = thingset_time_timeout_spec(timeout_ms);

    return pthread_mutex_timedlock(&queue->receive_mutex, &timeout);
}


/**
 * @brief Unlock receive buffer.
 */
static inline void rbbq_receive_unlock(struct thingset_rbbq *queue)
{
    (void)pthread_mutex_unlock(&queue->receive_mutex);
}


/**
 * @brief Callback on receive.
 *
 * To be called by rbbq port after new data was received.
 *
 * @param[in] queue Pointer to the remote two-way queue
 */
void ts_rbbq_event_raise_receive(struct thingset_rbbq *queue);


/**
 * @brief Callback on transmit.
 *
 * To be called by rbbq port after data was transmitted.
 *
 * @param[in] queue Pointer to the queue
 */
void rbbq_event_raise_transmit(struct thingset_rbbq *queue);


/**
 * @brief Register the queue to make it available to rbbq_get_binding().
 *
 * The queue struct must at least be partly filled:
 * - .next = NULL
 * - .name = "A queue name"
 * - .port = pointer to port struct
 *
 * @param[in] queue Pointer to the queue
 * @return 0 on success, <0 otherwise
 */
int rbbq_register_binding(struct thingset_rbbq *queue);


#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_rbbq_api_priv -->
 */

#endif /* TS_RBBQ_H_ */
