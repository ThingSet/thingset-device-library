/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet context node handling
 */

#include "thingset_env.h"

#include "ts_port.h"
#include "ts_ctx.h"
#include "ts_macro.h"
#include "ts_log.h"

void ts_ctx_node_table_init(thingset_locid_t locid)
{
    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);

        for (uint16_t i = 0; i < TS_ARRAY_SIZE(data->node_table.nodes); i++) {
            struct ts_ctx_node *node = &data->node_table.nodes[i];

            /* Mark node table entry unused */
            node->port_id = THINGSET_PORT_ID_INVALID;
        }
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
    }
}

void ts_ctx_node_init_phantom(thingset_locid_t locid, uint16_t node_idx,
                              const thingset_uid_t *ctx_uid)
{
    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        struct ts_ctx_node *node = &data->node_table.nodes[node_idx];

        node->ctx_uid = *ctx_uid;
        node->last_seen_time = thingset_time_ms();
        node->port_id = TS_PORT_ID_PHANTOM;
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
    }
}

int ts_ctx_node_get(thingset_locid_t locid, const thingset_uid_t* ctx_uid, uint16_t* node_idx)
{
   int ret;

    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
        ret = -ENOTSUP;
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        uint16_t match_idx = UINT16_MAX;
        uint16_t empty_idx = UINT16_MAX; /* just in case we need it */

        for (uint16_t i = 0; i < TS_ARRAY_SIZE(data->node_table.nodes); i++) {
            struct ts_ctx_node *node = &data->node_table.nodes[i];
            if (node->port_id == THINGSET_PORT_ID_INVALID) {
                /* empty */
                if (empty_idx == UINT16_MAX) {
                    empty_idx = i;
                }
            }
            else if (ts_ctx_uid_equal(ctx_uid, &node->ctx_uid)) {
                /* match */
                match_idx = i;
                break;
            }
        }
        if (match_idx == UINT16_MAX) {
            /* Node is missing in node table */
            if (empty_idx == UINT16_MAX) {
                /* No space left in node table - search node to be evicted */
                match_idx = ts_ctx_node_evict(locid);
                ts_ctx_node_free(locid, match_idx);
            }
            else {
                match_idx = empty_idx;
            }
            /* Initialise node table entry */
            ts_ctx_node_init_phantom(locid, match_idx, ctx_uid);
        }
        *node_idx = match_idx;
        ret = 0;
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;
}

uint16_t ts_ctx_node_evict(thingset_locid_t locid)
{
    uint16_t oldest_idx = UINT16_MAX;

    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        thingset_time_ms_t last_seen_time = thingset_time_ms();

        for (uint16_t i = 0; i < TS_ARRAY_SIZE(data->node_table.nodes); i++) {
            struct ts_ctx_node *node = &data->node_table.nodes[i];
            /* Search oldest entry */
            if (node->last_seen_time <= last_seen_time) {
                last_seen_time = node->last_seen_time;
                oldest_idx = i;
            }
        }

        TS_ASSERT(oldest_idx != UINT16_MAX, "At least one entry must match.");
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
    }

    return oldest_idx;
}

void ts_ctx_node_free(thingset_locid_t locid, uint16_t node_idx)
{
    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        struct ts_ctx_node *node = &data->node_table.nodes[node_idx];

        if (node->port_id == THINGSET_PORT_ID_INVALID) {
            /* Already unused */
            TS_LOGD("CTX: %s multiple free for node with index %u", __func__, (unsigned)node_idx);
            return;
        }
        /* Mark node table entry unused */
        node->port_id = THINGSET_PORT_ID_INVALID;
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
    }
}

int ts_ctx_node_lookup(thingset_locid_t locid, const thingset_uid_t* ctx_uid, uint16_t* node_idx)
{
    int ret;

    if (TS_CTX_IS_CORE(locid)) {
        TS_ASSERT(false, "CTX: %s can not be used with core context", __func__);
        ret = -ENOTSUP;
    }
    else if (TS_CTX_IS_COM(locid)) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(locid);
        for (uint16_t i = 0; i < TS_ARRAY_SIZE(data->node_table.nodes); i++) {
            struct ts_ctx_node *node = &data->node_table.nodes[i];
            if (node->port_id == THINGSET_PORT_ID_INVALID) {
                /* empty */
                continue;
            }
            else if (ts_ctx_uid_equal(ctx_uid, &node->ctx_uid)) {
                /* match */
                *node_idx = i;
                return 0;
            }
        }
        ret = -ENODEV;
    }
    else {
        TS_ASSERT(false, "CTX: %s used with unknown context", __func__);
        ret = -ENOTSUP;
    }

    return ret;

}
