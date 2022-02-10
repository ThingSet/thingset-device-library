/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet context (application) object support
 * ---------------------------------------------
 */

#include "thingset_env.h"
#include "thingset_time.h"

#include "ts_obj.h"
#include "ts_ctx.h"
#include "ts_msg.h"
#include "ts_macro.h"

#include <string.h>

int ts_ctx_obj_connect(thingset_locid_t locid, thingset_uid_t *ctx_uid,
                       ts_obj_id_t obj_id, const char *obj_path, thingset_time_ms_t timeout_ms,
                       thingset_oref_t *oref)
{
    if (TS_CTX_IS_CORE(locid)) {
        /* Core context does not support information about other remote contexts */
        return -EINVAL;
    }
    if (TS_CTX_IS_COM(locid)) {
        if ((ctx_uid == NULL) || ts_ctx_uid_equal(ts_ctx_uid(locid), ctx_uid)) {
            /* local context */
            ts_obj_db_id_t did = ts_ctx_obj_db(locid);
            if (did == TS_OBJ_DB_ID_INVALID) {
                return -ENOMEM;
            }
            if (obj_path != NULL) {
                return ts_obj_by_path(ts_obj_db_oref_any(did), obj_path, strlen(obj_path), oref);
            }
            else {
                return ts_obj_db_oref_by_id(did, obj_id, oref);
            }
        }
    }

    return 0;
}

int ts_ctx_obj_disconnect(thingset_locid_t locid, thingset_oref_t oref)
{
    if (oref.db_id == ts_ctx_obj_db(locid)) {
        /* local context */
        return 0;
    }
    /* remote context */

    return 0;
}
