/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_env.h"
#include "thingset_time.h"

#include "ts_bufq.h"
#include "ts_obj.h"
#include "ts_port.h"
#include "ts_ctx.h"
#include "ts_msg.h"
#include "ts_macro.h"

#include <string.h>

/**
 *@brief Local contexts by context id.
 *
 * Array of pointers to contexts. Array index == context id.
 */
const struct ts_ctx *const ts_ctxs[TS_CONFIG_LOCAL_COUNT] = {
#if TS_CONFIG_LOCAL_COUNT > 0
    &ts_ctx_0,
#endif
#if TS_CONFIG_LOCAL_COUNT > 1
    &ts_ctx_1,
#endif
#if TS_CONFIG_LOCAL_COUNT > 2
    &ts_ctx_2,
#endif
#if TS_CONFIG_LOCAL_COUNT > 3
    &ts_ctx_3,
#endif
#if TS_CONFIG_LOCAL_COUNT > 4
    &ts_ctx_4,
#endif
#if TS_CONFIG_LOCAL_COUNT > 5
#error "Local contexts limited to 5"
#endif
};

int thingset_init(thingset_locid_t locid)
{
    int ret;

    /* Assure initialisation of object database(s). */
    ts_obj_db_init();

    thingset_protocol_set_txt(locid);
    thingset_authorisation_set(locid, TS_USR_MASK);

    pthread_mutexattr_t process_mutex_attr;
    ret = pthread_mutexattr_init(&process_mutex_attr);
    TS_ASSERT(ret == 0, "%s fails on pthread_mutexattr_init with %d", __func__, ret);
    ret = pthread_mutexattr_settype(&process_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    TS_ASSERT(ret == 0, "%s fails on pthread_mutexattr_settype PTHREAD_MUTEX_RECURSIVE with %d",
              __func__, ret);

    if (TS_CTX_IS_CORE(locid)) {
        struct ts_ctx_core_data *data = ts_ctx_core_data(locid);

        ret = pthread_mutex_init(&data->common.process_mutex, &process_mutex_attr);

        data->resp_buf = NULL;
        data->resp_buf_size = 0;
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);

        ret = pthread_mutex_init(&data->common.process_mutex, &process_mutex_attr);
        if (ret != 0) {
            return ret;
        }

        ts_ctx_node_table_init(locid);

        ret = ts_ports_init(locid);
    }
    else {
        TS_ASSERT(false, "%s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

int thingset_run(thingset_locid_t locid)
{
    int ret;

    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "%s can not be used with core context", __func__);
        ret = -ENOTSUP;
    }
    else if (TS_CTX_IS_COM(locid)) {
        ret = 0;
        for (thingset_portid_t port_id = 0; port_id < TS_ARRAY_SIZE(ts_ports); port_id++) {
            const struct thingset_port *port = ts_ports[port_id];
            if ((port->loc_id == locid) && (port->api->run != NULL)) {
                ret = ts_port_run(port);
                if (ret != 0) {
                    break;
                }
            }
        }
    }
    else {
        TS_ASSERT(false, "%s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

void thingset_protocol_set_txt(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    ts_ctxs[locid]->data->app_protocol_use_bin = false;
}

void thingset_protocol_set_bin(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    ts_ctxs[locid]->data->app_protocol_use_bin = true;
}

uint16_t thingset_authorisation(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    return ts_ctxs[locid]->data->_auth_flags;
}

void thingset_authorisation_set(thingset_locid_t locid, uint16_t auth)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    TS_LOGD("CTX: %s set context %d authorisation to 0x%04x", __func__,
            (unsigned int)locid, (unsigned int)auth);
    ts_ctxs[locid]->data->_auth_flags = auth;
}

ts_obj_db_id_t ts_ctx_obj_db(thingset_locid_t locid)
{
    TS_ASSERT(locid < TS_CONFIG_LOCAL_COUNT, "CTX: %s on invalid local context identifier %u",
              __func__, (unsigned int)locid);
    return ts_ctxs[locid]->db_id;
}

const thingset_uid_t *ts_ctx_uid(thingset_locid_t locid)
{
    ts_obj_db_id_t did = ts_ctx_obj_db(locid);

    if (did != TS_OBJ_DB_ID_INVALID) {
        return ts_obj_db_by_id(did)->uuid;
    }

    return NULL;
}

int thingset_port(thingset_locid_t locid, const char *port_name, thingset_portid_t *port_id)
{
    int ret;

    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "%s can not be used with core context", __func__);
        ret = -ENOTSUP;
    }
    else if (TS_CTX_IS_COM(locid)) {
        for (thingset_portid_t id = 0; id < TS_ARRAY_SIZE(ts_ports); id++) {
            const struct thingset_port *port = ts_ports[id];
            if ((port->loc_id == locid) && (strcmp(port->name, port_name) == 0)) {
                *port_id = id;
                return 0;
            }
        }
        ret = -ENAVAIL;
    }
    else {
        TS_ASSERT(false, "%s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

int ts_ctx_transmit(thingset_locid_t locid, thingset_portid_t port_id, struct thingset_msg *msg,
                    thingset_time_ms_t timeout_ms)
{
   int ret;

    if (TS_CTX_IS_CORE(locid)) {
        const struct ts_ctx_core_data *data = ts_ctx_core_data(locid);
        if (ts_msg_len(msg) > data->resp_buf_size ) {
            ret = -ENOMEM;
        }
        else {
            memcpy(data->resp_buf, ts_msg_data(msg), ts_msg_len(msg));
            ret = 0;
        }
    }
    else if (TS_CTX_IS_COM(locid)) {
        const struct thingset_port *port = ts_port_by_id(port_id);
        ret = port->api->transmit(port, msg, timeout_ms);
    }
    else {
        TS_ASSERT(false, "%s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

/* helper functions */

bool ts_ctx_uid_equal(const thingset_uid_t *uuid_a, const thingset_uid_t *uuid_b)
{
    if (uuid_a == uuid_b) {
        return true;
    }
    return *uuid_a == *uuid_b;
}
