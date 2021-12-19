/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet application (public interface)
 */

#ifndef THINGSET_APP_H_
#define THINGSET_APP_H_

/**
 * @brief ThingSet application.
 *
 * @note All structure definitions and functions that start with the prefix 'ts_' are not part of
 *       the public API and are just here for technical reasons. They should not be used in
 *       applications.
 *
 * @defgroup ts_app_api_pub ThingSet application (public interface)
 * @{
 */

#include "thingset_env.h"

#include "thingset_time.h"
#include "thingset_msg.h"
#include "thingset_obj.h"
#include "thingset_port.h"
#include "thingset_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief ThingSet application ID.
 *
 * The application ID is the same as the ID of the port that connects the application to a local
 * context.
 */
typedef thingset_portid_t thingset_appid_t;

/**
 * @brief ThingSet application.
 */
struct thingset_app {
    /** @brief Pointer to application instance immutable data */
    const void *config;

    /** @brief Pointer to application instance mutable data */
    void *data;

    /**
    * @brief Initialise application.
    *
    * @param[in] app Pointer to application.
    * @return 0 on success, <0 otherwise.
    */
    int (*init)(const struct thingset_app *app);

    /**
    * @brief Run application.
    *
    * @param[in] app Pointer to application.
    * @return 0 on success, <0 otherwise.
    */
    int (*run)(const struct thingset_app *app);
};

/**
 * @brief Signature of a ThingSet application initialisation function.
 *
 * @param[in] app Pointer to application.
 * @return 0 on success, <0 otherwise.
 */
typedef int (*thingset_app_init_fn_t)(const struct thingset_app *app);

/**
 * @brief Signature of a ThingSet application run function.
 *
 * @param[in] app_port Port.
 * @return 0 on success, <0 otherwise.
 */
typedef int (*thingset_app_run_fn_t)(const struct thingset_app *app);

/**
 * @} <!-- ts_app_api_pub -->
 * @addtogroup ts_app_api_priv
 * @{
 */

/**
 * @brief Mutable data structure of application port.
 */
struct ts_app_port_data {
    /**
     * @brief Port receive buffer queue.
     *
     * The queue the messages that are send to the application.
     */
    struct ts_impl_bufq rx_bufq;
};

/**
 * @} <!-- ts_app_api_priv -->
 * @addtogroup ts_app_api_pub
 * @{
 */

/**
 * @def THINGSET_APP_DEFINE
 *
 * @brief Define a ThingSet application instance.
 *
 * A ThingSet application is identified by it's application ID:
 *
 * - @p app_id : The application identifier is the identifier of the ThingSet port that connects
 *               the application to a local context. Therefor application port id and application id
 *               are just two different ways to refer to the same identifier.
 *
 * - @p app_name : The name of the application must be unique.
 *
 * A ThingSet application has to provide two functions that connect it to the ThingSet context.
 *
 *  - @p app_init : The initialisation function called to initialise the application. The signature
 *                  of the init function has to be of @ref thingset_app_init_fn_t.
 *  - @p app_run : The run function called to run the application. The signature
 *                  of the run function has to be of @ref thingset_app_run_fn_t.
 *
 * A ThingSet application may have it's private data:
 *
 * - @p app_config : immutable data of the application
 * - @p app_data : mutable data of the application
 *
 * This macro also initialises a ThingSet application port. That is the ThingSet communication port
 * that this application uses to communicate. The port - defined by @p app_id - links the
 * application to the local context given by @p app_loc_id.
 *
 * @param[in] app_id Application (port) identifier. Paramter must expand to a positiv number.
 *                   Limit defined by @ref TS_CONFIG_PORT_COUNT.
 * @param[in] app_name Name of the application.
 * @param[in] app_loc_id Identifier of ThingSet local context the application is attached to.
 * @param[in] app_init Init function to be called by the ThingSet context to ininitialize this
 *                     application.
 * @param[in] app_run Run function to be called by the ThingSet context to run this application.
 * @param[in] app_config Pointer to configuration data of this application.
 * @param[in] app_data Pointer to the mutable data of this application.
 */
#define THINGSET_APP_DEFINE(app_id, app_name, app_loc_id, app_init, app_run, app_config, app_data) \
    extern const struct ts_port_api ts_app_port_api;                                            \
    struct ts_app_port_data TS_CAT(ts_port_data_, app_id);                                      \
    const struct thingset_app TS_CAT(ts_port_config_, app_id) = {                               \
        .config = &TS_EXPAND(app_config),                                                       \
        .data = &TS_EXPAND(app_data),                                                           \
        .init = &app_init,                                                                      \
        .run = &app_run,                                                                        \
    };                                                                                          \
    const struct thingset_port TS_CAT(ts_port_, app_id) = {                                     \
        .loc_id = app_loc_id,                                                                   \
        .name = app_name,                                                                       \
        .config = &TS_CAT(ts_port_config_, app_id),                                             \
        .api = &ts_app_port_api,                                                                \
        .data = &TS_CAT(ts_port_data_, app_id) }

/**
 * @brief Get name of application instance.
 *
 * @param[in] app_id Application (port) identifier.
 * @return name of application instance.
 */
const char *thingset_app_name(thingset_appid_t app_id);

/**
 * @brief Get ThingSet context the application instance is attached to.
 *
 * @param[in] app_id Application (port) identifier.
 * @return ThingSet local context identifier.
 */
thingset_locid_t thingset_app_ctx(thingset_appid_t app_id);

/**
 * @brief Allocate remote object entry within remote objects database.
 *
 * The remote object is allocated in the remote objects database of the context the application
 * is linked to.
 *
 * @param[in] app_id Application (port) identifier.
 * @param[in] ctx_uid Universal id of the context the remote object lives in.
 * @param[in] obj_id Id of the remote object.
 * @param[in] obj_path Path of the remote object.
 * @param[in] timeout_ms Maximum time to wait in milliseconds.
 * @param[out] oref Pointer to object reference for the remote object.
 * @return 0 on success, <0 otherwise.
 */
int thingset_obj_alloc(thingset_appid_t app_id, thingset_uid_t *ctx_uid,
                       ts_obj_id_t obj_id, const char *obj_path, thingset_time_ms_t timeout_ms,
                       thingset_oref_t *oref);

int thingset_obj_free(thingset_appid_t app_id, thingset_oref_t oref);

int thingset_publish(thingset_appid_t app_id, thingset_oref_t oref, thingset_time_ms_t period_ms);

int thingset_connect(thingset_appid_t app_id, thingset_oref_t oref, thingset_time_ms_t timeout_ms);

int thingset_subscribe(thingset_appid_t app_id, thingset_oref_t oref);

int thingset_change(thingset_appid_t app_id, thingset_oref_t oref, thingset_time_ms_t timeout_ms);

int thingset_fetch(thingset_appid_t app_id, thingset_oref_t oref, uint16_t subsets,
                   thingset_time_ms_t timeout_ms);

int thingset_exec(thingset_appid_t app_id, thingset_oref_t oref, thingset_time_ms_t timeout_ms);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_app_api_pub -->
 */

/** @page ts_topic_app Applications

A ThingSet application is defined by two functions and up to two data sets an an application name.
The application functions are called by the ThingSet context. One function is for application
initialisation, the other one is to run the application.

A ThingSet application is identified by it's name:

    name : The name of the application must be unique.

A ThingSet application has to provide two functions that connect it to the ThingSet context.

    init : int my_app_init(const struct ts_port *app_port)
    run : int my_app_run(const struct ts_port *app_port)

A ThingSet application may have it's private data:

    config : immutable data of the application
    data : mutable data of the application

*/

#endif /* THINGSET_APP_H_ */
