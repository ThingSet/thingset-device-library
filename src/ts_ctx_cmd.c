/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet context application command support
 * --------------------------------------------
 */

#include "thingset_env.h"
#include "thingset_time.h"

#include "ts_obj.h"
#include "ts_ctx.h"
#include "ts_msg.h"
#include "ts_macro.h"

#include <string.h>

int ts_ctx_cmd_publish(thingset_locid_t locid, thingset_portid_t *app_port_id,
                       thingset_oref_t oref, thingset_time_ms_t period_ms);

int ts_ctx_cmd_subscribe(thingset_locid_t locid, thingset_portid_t *app_port_id,
                         thingset_oref_t oref);

int ts_ctx_cmd_change(thingset_locid_t locid, thingset_portid_t *app_port_id,
                      thingset_oref_t oref, thingset_time_ms_t timeout_ms);

int ts_ctx_cmd_fetch(thingset_locid_t locid, thingset_portid_t *app_port_id,
                     thingset_oref_t oref, uint16_t subsets, thingset_time_ms_t timeout_ms);

int ts_ctx_cmd_exec(thingset_locid_t locid, thingset_portid_t *app_port_id,
                    thingset_oref_t oref, thingset_time_ms_t timeout_ms);
