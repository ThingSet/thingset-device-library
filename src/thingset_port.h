/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet communication port (public interface)
 */

#ifndef THINGSET_PORT_H_
#define THINGSET_PORT_H_

/**
 * @brief ThingSet communication port.
 *
 * An interface utilized by a ThingSet local context for ThingSet communication.
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_port_api_pub ThingSet communication port (public interface)
 * @{
 */

#include "thingset_env.h"

#include "ts_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration */
typedef uint8_t thingset_locid_t;
struct thingset_port;
struct thingset_msg;

/**
 * @brief ThingSet communication port identifier type.
 *
 * Port identifiers are specifc to a ThingSet device.
 */
typedef uint8_t thingset_portid_t;

/**
 * @def THINGSET_PORT_ID_INVALID
 *
 * @brief Invalid port identifier
 */
#define THINGSET_PORT_ID_INVALID UINT8_MAX

/**
 * @brief Signature of ThingSet communication port initialisation function.
 *
 * @param[in] port Port.
 * @return 0 on success, <0 otherwise.
 */
typedef int (*thingset_port_init_fn_t)(const struct thingset_port *port);

/**
 * @brief Signature of ThingSet communication port run function.
 *
 * @param[in] port Port.
 * @return 0 on success, <0 otherwise.
 */
typedef int (*thingset_port_run_fn_t)(const struct thingset_port *port);


/**
 * @brief Signature of ThingSet communication port transmit function.
 *
 * @param[in] port Port to send at.
 * @param[in] msg Pointer to message buffer to be send.
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @return 0 on success, <0 otherwise.
 */
typedef int (*thingset_port_transmit_fn_t)(const struct thingset_port *port, struct thingset_msg *msg,
                                           thingset_time_ms_t timeout_ms);
/**
 * @} <!-- ts_port_api_pub -->
 * @addtogroup ts_port_api_priv
 * @{
 */

/**
 * @brief ThingSet communication port API function struct.
 */
struct ts_port_api {
    /**
     * @brief Initialize ThingSet communication port.
     *
     * @param[in] port Port.
     */
    int (*init)(const struct thingset_port *port);

    /**
     * @brief Run ThingSet communication port.
     *
     * @param[in] port Port.
     */
    int (*run)(const struct thingset_port *port);

    /**
     * @brief Transmit buffer on port.
     *
     * @param[in] port Port to send at.
     * @param[in] msg Pointer to message buffer to be send.
     * @param[in] timeout_ms maximum time to wait in milliseconds.
     */
    int (*transmit)(const struct thingset_port *port, struct thingset_msg *msg,
                    thingset_time_ms_t timeout_ms);
};

/**
 * @} <!-- ts_port_api_priv -->
 * @addtogroup ts_port_api_pub
 * @{
 */

/**
 * @brief A ThingSet communication port.
 *
 * Runtime port structure (in ROM) per port instance.
 */
struct thingset_port {
    /** @brief Identifier of the ThingSet local context the port is attached to. */
    thingset_locid_t loc_id;
    /** @brief Name of the port instance */
    const char *name;
    /** @brief Address of port instance config information */
    const void *config;
    /** @brief Address of the API structure exposed by the port instance */
    const struct ts_port_api *api;
    /** @brief Address of the port instance private data */
    void *data;
};

/**
 * @def THINGSET_PORT_DATA_STRUCT
 *
 * @brief Define a ThingSet port data structure type.
 *
 * The port data structure type is part of the port's public interface and has to be provided to
 * allow the definition of a port instance by @ref THINGSET_PORT_DEFINE.
 *
 * Use it like this define the port data structure type in an appropriate header file:
 *
 *     THINGSET_PORT_DATA_STRUCT(loopback_simple) {
 *         thingset_portid_t other_port_id;
 *     };
 *
 * @param port_type The type name of the port.
 */
#define THINGSET_PORT_DATA_STRUCT(port_type) struct TS_CAT(port_type, _data)

/**
 * @def THINGSET_PORT_CONFIG_STRUCT
 *
 * @brief Define a ThingSet port data structure type.
 *
 * The port config structure type is part a the port's public interface and has to be provided to
 * allow the definition of a port instance by @ref THINGSET_PORT_DEFINE.
 *
 * Use it like this define the port config structure type in an appropriate header file:
 *
 *     THINGSET_PORT_CONFIG_STRUCT(loopback_simple) {
 *         const char *other_port;
 *     };
 *
 * @param port_type The type name of the port.
 */
#define THINGSET_PORT_CONFIG_STRUCT(port_type) struct TS_CAT(port_type, _config)

#define THINGSET_PORT_CONFIG(port_type, port)                                       \
    ((THINGSET_PORT_CONFIG_STRUCT(port_type) *)(port->config))

#define THINGSET_PORT_DATA(port_type, port)                                         \
        ((THINGSET_PORT_DATA_STRUCT(port_type) *)(port->data))

/**
 * @def THINGSET_PORT_TYPE
 *
 * @brief Define a ThingSet communication port type.
 *
 * @note To be used for port implementation only. Users (creators) of a port instances do not need
 *       to define the port type. It is implictly made known by @ref THINGSET_PORT_DEFINE.
 *
 * @param port_type The type name of the port.
 * @param init_fn Initialisation function.
 * @param run_fn Run function.
 * @param transmit_fn Transmit function.
 */
#define THINGSET_PORT_TYPE(port_type, init_fn, run_fn, transmit_fn)                 \
    const struct ts_port_api TS_CAT(port_type, _api) = {                            \
        .init = &init_fn,                                                           \
        .run = &run_fn,                                                             \
        .transmit = transmit_fn }

/**
 * @def THINGSET_PORT_DEFINE
 *
 * @brief Define a ThingSet port instance.
 *
 * This macro defines a ThingSet port instance.
 *
 * Use it like this to define a port instance:
 *
 *     THINGSET_PORT_DEFINE(MY_PORT_ID, my_port_type, my_port_name, MY_PORT_LOCID,
 *                          .my_port_config_attr = ...);
 *
 * @param[in] port_id Port identifier. Paramter must expand to a positiv number.
 *                    Limit defined by @ref TS_CONFIG_PORT_COUNT.
 * @param[in] port_type The type of the port.
 * @param[in] port_name The name this instance of the port exposes to the system.
 * @param[in] port_loc_id Identifier of ThingSet local context the port is attached to.
 * @param ... Initialisation values for the port's immutable data
 *            (see @ref THINGSET_PORT_CONFIG_STRUCT).
 */
#define THINGSET_PORT_DEFINE(port_id, port_type, port_name, port_loc_id, ...)                   \
    TS_STATIC_ASSERT(port_id < TS_CONFIG_PORT_COUNT, "port id >= "                              \
                     TS_STRINGIFY(TS_CONFIG_PORT_COUNT));                                       \
    extern const struct ts_port_api TS_CAT(port_type, _api);                                    \
    THINGSET_PORT_DATA_STRUCT(port_type) TS_CAT(ts_port_data_, port_id);                        \
    const THINGSET_PORT_CONFIG_STRUCT(port_type) TS_CAT(ts_port_config_, port_id) = {           \
        __VA_ARGS__ };                                                                          \
    const struct thingset_port TS_CAT(ts_port_, port_id) = {                                    \
        .loc_id = port_loc_id,                                                                  \
        .name = port_name,                                                                      \
        .config = &TS_CAT(ts_port_config_, port_id),                                            \
        .api = &TS_CAT(port_type, _api),                                                        \
        .data = &TS_CAT(ts_port_data_, port_id) }

/**
 * @brief Get name of port instance.
 *
 * @param[in] port_id Port identifier.
 * @return name of post instance.
 */
const char *thingset_port_name(thingset_portid_t port_id);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_port_api_pub -->
 */

/**
 * @page ts_topic_port Ports
 *
 * Ports allow to communicate with other ThingSet instances via physical or logical interfaces.
 * Physical interfaces may be RS232, RS485, SPI, CAN, WiFi, Ethernet, ... . Logical interfaces may
 * be everything that translates between the ThingSet protocol and other worlds. Ports for logical
 * interfaces are called virtual ports.
 *
 * A device holds exactly one ports table with descriptors for all it's ports. A port is associated
 * to just one local context.
 */

#endif /* THINGSET_PORT_H_ */
