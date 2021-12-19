/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../ts_ctx.h"
#include "../../ts_msg.h"

#include "loopback_simple.h"

int loopback_simple_init(const struct thingset_port *port)
{
    return 0;
}

int loopback_simple_run(const struct thingset_port *port)
{
    return 0;
}

int loopback_simple_transmit(const struct thingset_port *port, struct thingset_msg *msg,
                             thingset_time_ms_t timeout_ms)
{
    /* Behave as the other end of the loopback - we fake the receive. */

    /* Pretend to act as a "normal" port that allocates the buffers by itself to better resemble
     * this behaviour for e.g. testing. */
    struct thingset_msg *other_msg;
    int ret = thingset_msg_clone(msg, 0, &other_msg);
    thingset_msg_unref(msg);
    if (ret != 0) {
        return ret;
    }

    ret = ts_msg_proc_setup(other_msg);
    if (ret != 0) {
        return ret;
    }
    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, other_msg);
    scratchpad->port_src = THINGSET_PORT_CONFIG(loopback_simple, port)->other_port_id;
    scratchpad->port_dest = THINGSET_PORT_ID_INVALID;

    ret = thingset_process(ts_port_by_id(scratchpad->port_src)->loc_id, other_msg);
    return ret;
}

THINGSET_PORT_TYPE(loopback_simple, loopback_simple_init, loopback_simple_run,
                   loopback_simple_transmit);
