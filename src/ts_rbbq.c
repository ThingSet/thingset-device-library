/**
 * Copyright (c) 2020..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet two-way queue with contiguous reservation
 */

#include <string.h>

#include "ts_rbbq.h"

#define RBBQ_MESSAGE_ALLOC_NOMEM        (-1)
#define RBBQ_MESSAGE_ALLOC_AT_END       0
#define RBBQ_MESSAGE_ALLOC_AT_START     1
#define RBBQ_MESSAGE_ALLOC_AT_MIDDLE    2
#define RBBQ_MESSAGE_FREE_NOMEM         (-1)
#define RBBQ_MESSAGE_FREE_TO_WRITE      0
#define RBBQ_MESSAGE_FREE_TO_WATERMARK  1
#define RBBQ_MESSAGE_CORRUPTED          (-2)


/**
 * @brief Head of linked list of registered queues.
 */
static struct thingset_rbbq *rbbq_queues = 0;

static inline int rbbq_alloc_avail(const struct thingset_rbbq *queue, uint16_t size)
{
    uint16_t write_idx = queue->alloc_write_idx;
    uint16_t read_idx = atomic_load(&queue->tx_shadow_read_idx);
    uint16_t end_idx = queue->port->tx_data_size;

    if (write_idx >= read_idx) {
        if (size <= (end_idx - write_idx)) {
            return RBBQ_MESSAGE_ALLOC_AT_END;
        } else if (size < read_idx) {
            return RBBQ_MESSAGE_ALLOC_AT_START;
        }
    } else { /* write_idx < read_idx */
        if (size < (read_idx - write_idx)) {
            return RBBQ_MESSAGE_ALLOC_AT_MIDDLE;
        }
    }
    return RBBQ_MESSAGE_ALLOC_NOMEM;
}

int thingset_rbbq_alloc(struct thingset_rbbq *queue, uint16_t channel, uint16_t size, uint8_t **data,
                        thingset_time_ms_t timeout_ms)
{
    int ret;

    if (ts_rbbq_state(queue) != TS_RBBQ_QUEUE_STATE_RUNNING) {
        return -EAGAIN;
    }

    struct timespec timeout = thingset_time_timeout_spec(timeout_ms);
    ret = pthread_mutex_timedlock(&queue->alloc_mutex, &timeout);
    if (ret != 0) {
        /* cannot lock alloc..transmit operation */
        return ret;
    }

    uint8_t *raw_data;
    int alloc_mode = rbbq_alloc_avail(queue, size + sizeof(struct ts_rbbq_data_header));
    switch (alloc_mode) {
    case RBBQ_MESSAGE_ALLOC_AT_END:
        raw_data = queue->port->tx_data + queue->alloc_write_idx;
        rbbq_data_channel_set(raw_data, channel);
        rbbq_data_payload_size_set(raw_data, size);
        *data = rbbq_data_payload(raw_data);

        queue->alloc_data = raw_data;
        queue->alloc_watermark_idx = queue->port->tx_data_size;
        queue->alloc_write_idx += rbbq_data_size(raw_data);
        break;
    case RBBQ_MESSAGE_ALLOC_AT_START:
        raw_data = queue->port->tx_data;
        rbbq_data_channel_set(raw_data, channel);
        rbbq_data_payload_size_set(raw_data, size);
        *data = rbbq_data_payload(raw_data);

        queue->alloc_data = raw_data;
        queue->alloc_watermark_idx = queue->alloc_write_idx;
        queue->alloc_write_idx = rbbq_data_size(raw_data);
        break;
    case RBBQ_MESSAGE_ALLOC_AT_MIDDLE:
        raw_data = queue->port->tx_data + queue->alloc_write_idx;
        rbbq_data_channel_set(raw_data, channel);
        rbbq_data_payload_size_set(raw_data, size);
        *data = rbbq_data_payload(raw_data);

        queue->alloc_data = raw_data;
        queue->alloc_write_idx += rbbq_data_size(raw_data);
        break;
    case RBBQ_MESSAGE_ALLOC_NOMEM:
        ret = -ENOMEM;
        (void)pthread_mutex_unlock(&queue->alloc_mutex);
        break;
    default:
        /* should never happend */
        ret = -EFAULT;
        (void)pthread_mutex_unlock(&queue->alloc_mutex);
        break;
    }
    return ret;
}


int thingset_rbbq_transmit(struct thingset_rbbq *queue, uint8_t *data)
{
    if (ts_rbbq_state(queue) != TS_RBBQ_QUEUE_STATE_RUNNING) {
        return -EAGAIN;
    }

    if ((data == NULL) || (data != rbbq_data_payload(queue->alloc_data))) {
        return -EINVAL;
    }

    /* Inform port that a new transmit data is available. */
    int ret =  queue->port->port_api->transmit(queue);

    queue->alloc_data = NULL;

    (void)pthread_mutex_unlock(&queue->alloc_mutex);

    return ret;
}


/**
 * @brief Is there a messsage received that can be freed.
 *
 * Two possible memory configurations
 * - write leads and read follows (write ≥ read),
 *   the valid data (written, but not yet processed by the reader)
 *   is in the section of the buffer after read and before write;
 * - read leads and write follows (read > write),
 *   the valid data is after read, till the watermark, and from the
 *   start of the buffer till write.
 */
static inline int rbbq_data_free_avail_unprotected(const struct thingset_rbbq *queue)
{
    int ret;
    uint16_t avail_size;
    uint16_t write_idx;
    uint16_t watermark_idx;
    uint16_t read_idx;

    /* Read from rx control */
    uint16_t other_read_idx;
    rbbq_control(queue->port->rx_control, &write_idx, &watermark_idx, &other_read_idx);

    /* We work on this read_idx (not other) */
    read_idx = queue->free_read_idx;

    if (write_idx >= read_idx) {
        avail_size = write_idx - read_idx;
        ret = RBBQ_MESSAGE_FREE_TO_WRITE;
    } else { /* write_idx < read_idx */
        avail_size = watermark_idx - read_idx;
        ret = RBBQ_MESSAGE_FREE_TO_WATERMARK;
    }
    if (avail_size == 0) {
        ret = RBBQ_MESSAGE_FREE_NOMEM;
    } else {
        uint8_t *raw_data = &queue->port->rx_data[read_idx];
        if (rbbq_data_size(raw_data) > avail_size) {
            /* corrupted data */
            ret = RBBQ_MESSAGE_CORRUPTED;
        }
    }
    return ret;
}


int thingset_rbbq_receive(struct thingset_rbbq *queue, uint16_t *channel, uint16_t *size,
                          uint8_t **data, thingset_time_ms_t timeout_ms)
{
    if (ts_rbbq_state(queue) != TS_RBBQ_QUEUE_STATE_RUNNING) {
        LOG_WRN("%s receive request on queue not running (state: %d)",
                thingset_rbbq_name(queue), (int)ts_rbbq_state(queue));
        return -EAGAIN;
    }

    int ret;
    do {
        ret = rbbq_receive_lock(queue, timeout_ms);
        if (ret != 0) {
            /* cannot lock receive..free operation */
            return ret;
        }
        int free_mode = rbbq_data_free_avail_unprotected(queue);
        if (free_mode == RBBQ_MESSAGE_FREE_NOMEM) {
            /* no data available */
            rbbq_receive_unlock(queue);
            ret = thingset_rbbq_wait_receive(queue, timeout_ms);
            if (ret == 0) {
                continue;
            }
            ret = -ENOMEM;
        } else if (free_mode == RBBQ_MESSAGE_CORRUPTED) {
            rbbq_receive_unlock(queue);
            ret = -EFAULT;
        } else {
            ret = 0;
        }
    } while (false);
    if (ret == 0) {
        uint8_t *raw_data = &queue->port->rx_data[queue->free_read_idx];
        *channel = rbbq_data_channel(raw_data);
        *size = rbbq_data_payload_size(raw_data);
        *data = rbbq_data_payload(raw_data);

        queue->free_data = raw_data;
        queue->free_read_idx += rbbq_data_size(raw_data);
    }

    return ret;
}


int thingset_rbbq_free(struct thingset_rbbq *queue, uint8_t *data)
{
    if (ts_rbbq_state(queue) == TS_RBBQ_QUEUE_STATE_NONE) {
        return -ENODEV;
    }

    if (queue->free_data == NULL) {
        /* double free */
        return -ENOMEM;
    }
    if ((data == NULL) || (data != rbbq_data_payload(queue->free_data))) {
        return -EINVAL;
    }

    /* Inform port that a received data was freed. */
    int ret = queue->port->port_api->receive(queue);
    queue->free_data = NULL;

    rbbq_receive_unlock(queue);

    return ret;
}


int thingset_rbbq_init(struct thingset_rbbq *queue)
{
    if (!ts_rbbq_state_cas(queue, TS_RBBQ_QUEUE_STATE_NONE, TS_RBBQ_QUEUE_STATE_INIT)) {
        return -EEXIST;
    }

    int ret;

    ret = pthread_cond_init(&queue->port_receive_cond, NULL);
    if (ret != 0) {
        goto thingset_rbbq_init_error;
    }
    ret = pthread_cond_init(&queue->port_transmit_cond, NULL);
    if (ret != 0) {
        goto thingset_rbbq_init_error;
    }
    ret = pthread_mutex_init(&queue->alloc_mutex, NULL);
    if (ret != 0) {
        goto thingset_rbbq_init_error;
    }
    ret = pthread_mutex_init(&queue->receive_mutex, NULL);
    if (ret != 0) {
        goto thingset_rbbq_init_error;
    }

    queue->alloc_data = NULL;
    queue->alloc_write_idx = 0;
    queue->alloc_watermark_idx = 0xFFFF;

    queue->free_data = NULL;
    queue->free_read_idx = 0;

    /* call the port to finalise initialisation */
    ret = queue->port->port_api->init(queue);
    if (ret != 0) {
        goto thingset_rbbq_init_error;
    }

    /* sync alloc and free markers to size of buffer set by port */
    rbbq_control(queue->port->tx_control,
                 &queue->alloc_write_idx,
                 &queue->alloc_watermark_idx,
                 &queue->free_read_idx);

    ts_rbbq_state_set(queue, TS_RBBQ_QUEUE_STATE_READY);
    return 0;

thingset_rbbq_init_error:
    ts_rbbq_state_set(queue, TS_RBBQ_QUEUE_STATE_NONE);
    return ret;
}


int thingset_rbbq_start(struct thingset_rbbq *queue)
{
    atomic_val_t last_state;

    if (!ts_rbbq_state_cas(queue, TS_RBBQ_QUEUE_STATE_READY, TS_RBBQ_QUEUE_STATE_START)) {
        if (!ts_rbbq_state_cas(queue, TS_RBBQ_QUEUE_STATE_SUSPENDED, TS_RBBQ_QUEUE_STATE_START)) {
            return -EBUSY;
        } else {
            last_state = TS_RBBQ_QUEUE_STATE_SUSPENDED;
        }
    } else {
        last_state = TS_RBBQ_QUEUE_STATE_READY;
    }

    /* Request port to start data exchange. */
    int ret = queue->port->port_api->start(queue);
    if (ret != 0) {
        ts_rbbq_state_set(queue, last_state);
    } else {
        ts_rbbq_state_set(queue, TS_RBBQ_QUEUE_STATE_RUNNING);
    }

    return ret;
}


int thingset_rbbq_stop(struct thingset_rbbq *queue)
{
    if (!ts_rbbq_state_cas(queue, TS_RBBQ_QUEUE_STATE_RUNNING, TS_RBBQ_QUEUE_STATE_STOP)) {
        return -EBUSY;
    }

    /* Request port to stop data exchange. */
    int ret = queue->port->port_api->stop(queue);
    if (ret != 0) {
        ts_rbbq_state_set(queue, TS_RBBQ_QUEUE_STATE_RUNNING);
    } else {
        ts_rbbq_state_set(queue, TS_RBBQ_QUEUE_STATE_SUSPENDED);
    }

    return ret;
}


/*
(void) pthread_mutex_lock(&t.mn);
        t.waiters++;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    rc = 0;
    while (! mypredicate(&t) && rc == 0)
        rc = pthread_cond_timedwait(&t.cond, &t.mn, &ts);
    t.waiters--;
    if (rc == 0) setmystate(&t);
(void) pthread_mutex_unlock(&t.mn);
 */
int thingset_rbbq_wait_receive(struct thingset_rbbq *queue, thingset_time_ms_t timeout_ms)
{
    int ret;
    struct timespec timeout = thingset_time_timeout_spec(timeout_ms);

    (void)pthread_mutex_lock(&queue->port_receive_mutex);

    ret = pthread_cond_timedwait(&queue->port_receive_cond, &queue->port_receive_mutex, &timeout);

    (void)pthread_mutex_unlock(&queue->port_receive_mutex);

    return ret;
}


int thingset_rbbq_wait_transmit(struct thingset_rbbq *queue, thingset_time_ms_t timeout_ms)
{
    int ret;
    struct timespec timeout = thingset_time_timeout_spec(timeout_ms);

    (void)pthread_mutex_lock(&queue->port_transmit_mutex);

    ret = pthread_cond_timedwait(&queue->port_transmit_cond, &queue->port_transmit_mutex, &timeout);

    (void)pthread_mutex_unlock(&queue->port_transmit_mutex);

    return ret;
}


int thingset_rbbq_monitor(struct thingset_rbbq *queue)
{
    return queue->port->port_api->monitor(queue);
}


const char *thingset_rbbq_name(struct thingset_rbbq *queue)
{
    return queue->name;
}


struct thingset_rbbq *rbbq_get_binding(const char *name)
{
    struct thingset_rbbq *queue = rbbq_queues;
    while (queue != NULL) {
        if (strcmp(queue->name, name) == 0) {
            return queue;
        }
        queue = queue->next;
    }
    return NULL;
}

/*
 * -----------------------------------------------
 * INTERNAL interface
 * -----------------------------------------------
 */

void ts_rbbq_event_raise_receive(struct thingset_rbbq *queue)
{
    /*
     * We got a receive buffer update.
     * Store read_idx_other from receive buffer to shadow store to make it
     * available even if the receive buffer may be locked due to corrupted
     * data or receive operation later on.
     */
    atomic_store(&queue->tx_shadow_read_idx, rbbq_control_other_read_idx(queue->port->rx_control));

    pthread_cond_signal(&queue->port_receive_cond);
}


void rbbq_event_raise_transmit(struct thingset_rbbq *queue)
{
    pthread_cond_signal(&queue->port_transmit_cond);
}


int rbbq_register_binding(struct thingset_rbbq *new_queue)
{
    if ((new_queue->next != NULL)
            || (new_queue->name == NULL)
            || (new_queue->port == NULL)) {
        return -EINVAL;
    }

    if (rbbq_queues == NULL) {
        /* first queue */
        rbbq_queues = new_queue;
    } else {
        struct thingset_rbbq *queue = rbbq_queues;
        while (queue != NULL) {
            if (queue == new_queue) {
                /* Already registered */
                return -EALREADY;
            }
            if (queue->next == NULL) {
                queue->next = new_queue;
                new_queue->next = NULL;
                break;
            }
            queue = queue->next;
        }
    }
    return 0;
}
