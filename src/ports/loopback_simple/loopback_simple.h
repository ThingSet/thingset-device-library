/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Simple ThingSet loopback communication ports.
 *
 * A very basic loopback implementation that lacks a lot of features, e.g. thread safety.
 * Use with most care.
 *
 * Application programming interface.
 */

#ifndef LOOPBACK_SIMPLE_H_
#define LOOPBACK_SIMPLE_H_

#include "../../thingset_ctx.h"

THINGSET_PORT_CONFIG_STRUCT(loopback_simple) {
    thingset_portid_t other_port_id;
};

THINGSET_PORT_DATA_STRUCT(loopback_simple) {
};

#endif /* LOOPBACK_SIMPLE_H_ */
