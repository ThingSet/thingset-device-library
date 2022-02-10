/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet communication port (private interface)
 */

#ifndef TS_PORT_H_
#define TS_PORT_H_

/**
 * @brief ThingSet communication port.
 *
 * @defgroup ts_port_api_priv ThingSet communication port (private interface)
 * @{
 */

#include <stdint.h>

#include "thingset_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def TS_PORT_ID_PHANTOM
 *
 * @brief Phantom port identifier.
 *
 * A phantom port represents a port that we assume to exist but needs to be fully evaluated.
 * This may be the case if we want to access a ThingSet device but do not know which port to use
 * for the ThingSet messages.
 */
#define TS_PORT_ID_PHANTOM (THINGSET_PORT_ID_INVALID - 1)

/* Device ThingSet ports */
#if TS_CONFIG_PORT_COUNT > 0
extern const struct thingset_port ts_port_0;
#endif
#if TS_CONFIG_PORT_COUNT > 1
extern const struct thingset_port ts_port_1;
#endif
#if TS_CONFIG_PORT_COUNT > 2
extern const struct thingset_port ts_port_2;
#endif
#if TS_CONFIG_PORT_COUNT > 3
extern const struct thingset_port ts_port_3;
#endif
#if TS_CONFIG_PORT_COUNT > 4
extern const struct thingset_port ts_port_4;
#endif
#if TS_CONFIG_PORT_COUNT > 5
#error "Device ports limited to 5"
#endif

/**
 * @brief ThingSet device port table.
 *
 * Array of pointers to ports. Array index == port id.
 */
extern const struct thingset_port *const ts_ports[TS_CONFIG_PORT_COUNT];

/**
 * @brief Initialize the ports of the ThingSet device port table.
 *
 * @return 0 on success, <0 otherwise.
 */
int ts_ports_init(thingset_locid_t locid);

/**
 * @brief Get ThingSet port by port ID.
 *
 * @param[in] port_id Port ID.
 * @return Pointer to port structure.
 */
static inline const struct thingset_port *ts_port_by_id(thingset_portid_t port_id)
{
    TS_ASSERT(port_id < TS_ARRAY_SIZE(ts_ports),
              "PORT: %s invalid port id %u given with maximum port id %d", __func__,
              (unsigned int)port_id, (unsigned int)TS_ARRAY_SIZE(ts_ports));

#if TS_CONFIG_PORT_COUNT == 0
    return NULL;
#else
    return ts_ports[port_id];
#endif
}

/**
 * @brief Initialize ThingSet port.
 *
 * @param[in] port Port.
 */
static inline int ts_port_init(const struct thingset_port *port)
{
#if TS_CONFIG_PORT_COUNT == 0
    return 0;
#else
    return port->api->init(port);
#endif
};

/**
 * @brief Run ThingSet communication port.
 *
 * @param[in] port Port.
 */
static inline int ts_port_run(const struct thingset_port *port)
{
    return port->api->run(port);
};

/*
 * Helpers
 * -------
 */

/**
 * @brief Check for same port.
 *
 * @param port_a Pointer to port.
 * @param port_b Pointer to port.
 * @returns true on same, false otherwise.
 */
static inline bool ts_port_same(const struct thingset_port *port_a, const struct thingset_port *port_b)
{
    return (port_a == port_b);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_port_api_priv -->
 */

#endif /* TS_PORT_H_ */
