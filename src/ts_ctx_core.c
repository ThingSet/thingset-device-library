/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <malloc.h>

#include "ts_msg.h"
#include "ts_ctx.h"
#include "ts_log.h"


void thingset_dump_json(ts_obj_id_t obj_id, int level)
{
    TS_ASSERT(level == 0, "Deprecated function - only level == 0 supported");

    if (!TS_CONFIG_LOG || (TS_CONFIG_LOG_LEVEL < 3)) {
        return;
    }

    thingset_oref_t oref;
    int ret = ts_obj_db_oref_by_id(ts_ctx_obj_db(TS_CONFIG_CORE_LOCID), obj_id, &oref);
    if (ret != 0) {
        return;
    }

    char dump_buffer[10000];
    thingset_obj_log(oref, &dump_buffer[0], sizeof(dump_buffer));

    TS_LOGI("%s", &dump_buffer[0]);
}

const struct ts_obj *thingset_object_by_id(ts_obj_id_t id)
{
    thingset_oref_t oref;
    int ret = ts_obj_db_oref_by_id(TS_CONFIG_CORE_LOCID, id, &oref);
    if (ret != 0) {
        return NULL;
    }
    return ts_obj(oref);
}

const struct ts_obj *thingset_object_by_name(const char *name, size_t len, int32_t parent)
{
    int ret;
    thingset_oref_t oref = ts_obj_db_oref_any(ts_ctx_obj_db(TS_CONFIG_CORE_LOCID));
    if (parent >= 0) {
        ret = ts_obj_db_oref_by_id(oref.db_id, parent, &oref);
        if (ret != 0) {
            return NULL;
        }
    }

    ret = ts_obj_by_name(oref, name, len, &oref);
    if (ret != 0) {
        return NULL;
    }
    return ts_obj(oref);
}

const struct ts_obj *thingset_object_by_path(const char *path, size_t len)
{
    thingset_oref_t oref;
    int ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TS_CONFIG_CORE_LOCID)), path, len,
                             &oref);
    if (ret != 0) {
        return NULL;
    }
    return ts_obj(oref);
}
