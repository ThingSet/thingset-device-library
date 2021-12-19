/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet application (private interface)
 */

#ifndef TS_APP_H_
#define TS_APP_H_

/**
 * @brief ThingSet application.
 *
 * @defgroup ts_app_api_priv ThingSet application (private interface)
 * @{
 */

#include "thingset_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Application port API.
 */
extern const struct ts_port_api ts_app_port_api;

int ts_app_port_init(const struct thingset_port *port);
int ts_app_port_run(const struct thingset_port *port);
int ts_app_port_transmit(const struct thingset_port *port, struct thingset_msg *msg,
                         thingset_time_ms_t timeout_ms);

/**
 * @brief Issue request message at application port and get response.
 *
 * The function consumes the request message buffer (decreases the reference count).
 *
 * @note This is a low level functions - if possible, use instead one of thingset_publish(),
 *       thingset_connect(), thingset_subscribe(), thingset_change(), thingset_fetch(),
 *       thingset_exec().
 *
 * @param[in] tsc Pointer to ThingSet data and communication context.
 * @param[in] request Pointer to request message buffer to be send.
 * @param[in] timeout_ms maximum time to wait for response in milliseconds.
 * @param[out] response Pointer to return response message buffer to.
 * @returns 0 on success, <0 otherwise.
 */
int ts_app_request(thingset_appid_t app_id, struct thingset_msg *request,
                   thingset_time_ms_t timeout_ms, struct thingset_msg **response);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_app_api_priv -->
 */

#endif /* TS_APP_H_ */
