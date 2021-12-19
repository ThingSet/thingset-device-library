/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_env.h"

#include "ts_port.h"

#include <string.h>

/* ThingSet device port table. */
const struct thingset_port *const ts_ports[TS_CONFIG_PORT_COUNT] = {
#if TS_CONFIG_PORT_COUNT > 0
    &ts_port_0,
#endif
#if TS_CONFIG_PORT_COUNT > 1
    &ts_port_1,
#endif
#if TS_CONFIG_PORT_COUNT > 2
    &ts_port_2,
#endif
#if TS_CONFIG_PORT_COUNT > 3
    &ts_port_3,
#endif
#if TS_CONFIG_PORT_COUNT > 4
    &ts_port_4,
#endif
#if TS_CONFIG_PORT_COUNT > 5
#error "Device ports limited to 5"
#endif
};

int ts_ports_init(thingset_locid_t locid)
{
    for (thingset_portid_t port_id = 0; port_id < TS_CONFIG_PORT_COUNT; port_id++) {
        const struct thingset_port *port = ts_ports[port_id];
        if (port->loc_id != locid) {
            continue;
        }
        if (port->api->init != NULL) {
            int ret = ts_port_init(port);
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}

const char *thingset_port_name(thingset_portid_t port_id)
{
    return ts_port_by_id(port_id)->name;
}
