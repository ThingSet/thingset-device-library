/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet data objects
 */

#include "thingset_env.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ts_mem.h"
#include "ts_obj.h"
#include "ts_log.h"

/* Remote contexts' objects databases pool. */
struct ts_obj_rdb ts_obj_rdbs[TS_CONFIG_REMOTE_COUNT];

/* Objects databases by database id. */
const struct ts_obj_db *const ts_obj_dbs[TS_CONFIG_LOCAL_COUNT + TS_CONFIG_REMOTE_COUNT] = {
#if TS_CONFIG_LOCAL_COUNT > 0
    &ts_obj_db_0,
#endif
#if TS_CONFIG_LOCAL_COUNT > 1
    &ts_obj_db_1,
#endif
#if TS_CONFIG_LOCAL_COUNT > 2
    &ts_obj_db_2,
#endif
#if TS_CONFIG_LOCAL_COUNT > 3
    &ts_obj_db_3,
#endif
#if TS_CONFIG_LOCAL_COUNT > 4
    &ts_obj_db_4,
#endif
#if TS_CONFIG_LOCAL_COUNT > 5
#error "Local context databases limited to 5"
#endif
#if TS_CONFIG_REMOTE_COUNT > 0
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 0].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 1
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 1].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 2
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 2].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 3
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 3].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 4
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 4].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 5
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 5].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 6
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 6].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 7
    &ts_obj_rdbs[TS_CONFIG_LOCAL_COUNT + 7].db,
#endif
#if TS_CONFIG_REMOTE_COUNT > 8
#error "Remote context databases limited to 8"
#endif
};

/* Remote contexts' data objects pool */
struct ts_obj ts_obj_remotes[TS_CONFIG_REMOTE_OBJECT_COUNT];

/* Management data for remote contexts' data objects pool. */
struct ts_obj_remote_mngmt ts_obj_remotes_mngmt[TS_CONFIG_REMOTE_OBJECT_COUNT];

/* Memory pool for remote contexts' data objects data */
TS_MEM_DEFINE(ts_obj_remotes_data, TS_CONFIG_REMOTE_OBJECT_COUNT * 20);

static bool ts_obj_db_initialized = false;

void ts_obj_db_init(void)
{
    if (ts_obj_db_initialized) {
        return;
    }

    /* Initialize data objects databases for local context */
    for (ts_obj_db_id_t did = 0; did < TS_CONFIG_LOCAL_COUNT; did++) {
        /* database for local context objects */
        const struct ts_obj_db *db = ts_obj_db_by_id(did);
        for (unsigned int i = 0; i < db->num; i++) {
            db->meta[i] = db->objects[i].meta_default;
        }
        (void)ts_obj_db_check_id_duplicates(did);
    }

    /* Initialize data objects databases pool for remote context */
    for (ts_obj_db_id_t did = TS_CONFIG_LOCAL_COUNT; did < TS_ARRAY_SIZE(ts_obj_dbs); did++) {
        /* database for remote context objects */
        struct ts_obj_rdb *rdb = &ts_obj_rdbs[did - TS_CONFIG_LOCAL_COUNT];
        rdb->uuid = 0;
        /* Link remote context object database to remote objects pool */
        rdb->remotes = &ts_obj_remotes[0];

        /* Provide same interface as local context data objects database */
        rdb->db.objects = &ts_obj_remotes[0];
        rdb->db.num = 0;
        rdb->db.meta = NULL;
        rdb->db.uuid = &rdb->uuid;
        rdb->db.id = TS_OBJ_DB_ID_INVALID;
    }

    /* Initialize data objects pool for remote context */
    for (ts_obj_db_oid_t oid = 0; oid < TS_CONFIG_REMOTE_OBJECT_COUNT; oid++) {
        ts_obj_remotes_mngmt[oid].ref_count = 0;
        ts_obj_remotes[oid].id = TS_ID_INVALID;
        ts_obj_remotes[oid].type = TS_T_INVALID;
        ts_obj_remotes[oid].name = NULL;
        ts_obj_remotes[oid].data = NULL;
        ts_obj_remotes[oid].parent = TS_ID_INVALID;
        ts_obj_remotes[oid].meta_default.db_id = TS_OBJ_DB_ID_INVALID;
        ts_obj_remotes[oid].meta_default.access = 0;
        ts_obj_remotes[oid].meta_default.detail = 0;
        ts_obj_remotes[oid].meta_default.subsets = 0;
    }

    ts_obj_db_initialized = true;
}

int ts_obj_db_check_id_duplicates(ts_obj_db_id_t did)
{
    int ret = 0;

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(did, 0), oref_i) {
        TS_OBJ_DB_FOREACH_OREF(oref_i, oref_j) {
            if (ts_obj_db_oref_is_same(oref_i, oref_j)) {
                continue;
            }
            if (ts_obj_id(oref_i) == ts_obj_id(oref_j)) {
                TS_LOGE("OBJ: %s duplicate data object ID 0x%X.\n", __func__,
                        (unsigned int)ts_obj_id(oref_i));
                ret = -EALREADY;
            }
        }
    }

    return ret;
}

thingset_oref_t ts_obj_db_oref_root(ts_obj_db_id_t did)
{
    thingset_oref_t oref;

    oref.db_id = did;
    oref.db_oid = TS_OBJ_DB_OID_ROOT;

    return oref;
}

thingset_oref_t ts_obj_db_oref_any(ts_obj_db_id_t did)
{
    thingset_oref_t oref;

    oref.db_id = did;
    oref.db_oid = TS_OBJ_DB_OID_ANY;

    return oref;
}

thingset_oref_t ts_obj_db_oref_by_object(const struct ts_obj *object)
{
    ts_obj_db_id_t did = object->meta_default.db_id;
    const struct ts_obj_db *db = ts_obj_db_by_id(did);

    size_t idx = ((uint8_t *)object - (uint8_t *)&db->objects[0])/(sizeof(struct ts_obj));
    TS_ASSERT(idx < db->num, "OBJ: %s on invalid object reference '%u' (> %u)", __func__,
              (unsigned int)idx, (unsigned int)db->num);
    TS_ASSERT(&db->objects[idx] == object, "Invalid object to reference conversion");

    thingset_oref_t oref;
    oref.db_oid = (uint16_t)idx;
    oref.db_id = did;
    return oref;
}

bool ts_obj_db_oref_is_valid(thingset_oref_t oref)
{
    if (oref.db_id >= TS_ARRAY_SIZE(ts_obj_dbs)) {
        return false;
    }
    if ((oref.db_oid == TS_OBJ_DB_OID_ROOT) || (oref.db_oid  == TS_OBJ_DB_OID_ANY)) {
        return true;
    }
    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Local context database */
        const struct ts_obj_db *db = ts_obj_db_by_id(oref.db_id);
        if (oref.db_oid >= db->num) {
            return false;
        }
    }
    else {
        /* Remote context database */
        if (oref.db_oid >= TS_CONFIG_REMOTE_OBJECT_COUNT) {
            return false;
        }
        if (ts_obj_meta(oref)->db_id != oref.db_id) {
            return false;
        }
    }
    return true;
}

int ts_obj_db_oref_by_id(ts_obj_db_id_t did, ts_obj_id_t obj_id, thingset_oref_t *oref)
{
    if (obj_id == TS_ID_ROOT) {
        /* Object id of root object */
        oref->db_id = did;
        oref->db_oid = TS_OBJ_DB_OID_ROOT;
        return 0;
    }

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(did, 0), object_oref) {
        if (ts_obj_id(object_oref) == obj_id) {
            *oref = object_oref;
            return 0;
        }
    }

    TS_LOGD("OBJ: %s did not find object with id '%u' in database '%u'",
             __func__, (unsigned int)obj_id, (unsigned int)did);
    return -ENODATA;
}

int ts_obj_by_name(thingset_oref_t parent, const char *name, size_t len, thingset_oref_t *oref)
{
    TS_ASSERT(ts_obj_db_oref_is_valid(parent),
              "OBJ: %s for '%.*s' on invalid parent object reference (%u:%u)",
              __func__, (int)len, name, (unsigned int)parent.db_id, (unsigned int)parent.db_oid);

    ts_obj_id_t parent_id;
    if (ts_obj_db_oref_is_any(parent)) {
        parent_id = TS_ID_INVALID;
    }
    else {
        parent_id = ts_obj_id(parent);
    }

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(parent.db_id, 0), child_oref) {
        if ((parent_id == TS_ID_INVALID) || (parent_id == ts_obj_parent_id(child_oref))) {
            const char *child_name = ts_obj_name(child_oref);
            if ((child_name != 0) && (strncmp(child_name, name, len) == 0) &&
                /* without length check foo and fooBar would be recognized as equal */
                (strnlen(child_name, len + 1) == len)) {
                *oref = child_oref;
                return 0;
            }
        }
    }

    TS_LOGD("OBJ: %s did not find child '%.*s' of parent object (%u:%u) with id '%u'",
             __func__, (int)len, name, (unsigned int)parent.db_id, (unsigned int)parent.db_oid,
            (unsigned int)parent_id);
    return -ENODATA;
}

int ts_obj_by_path(thingset_oref_t parent, const char *path, size_t len, thingset_oref_t *oref)
{
    TS_ASSERT(ts_obj_db_oref_is_valid(parent),
              "OBJ: %s for '%.*s' on invalid parent object reference (%u:%u)",
              __func__, (int)len, path, (unsigned int)parent.db_id, (unsigned int)parent.db_oid);

    const char *start = path;
    const char *end;

    if (path[0] == '/') {
        /* Absolute path always starts at root parent */
        parent = ts_obj_db_oref_root(parent.db_id);
    }

    /* maximum depth of 10 assumed */
    for (int i = 0; i < 10; i++) {
        end = memchr(start, '/', ((uintptr_t)path + len) - (uintptr_t)start);
        if ((end == NULL) || (end >= (path + len))) {
            /* we are at the end of the path */
            return ts_obj_by_name(parent, start, ((uintptr_t)path + len) - (uintptr_t)start, oref);
        }
        else if (end == (path + len - 1)) {
            /* path ends with slash */
            return ts_obj_by_name(parent, start, (uintptr_t)end - (uintptr_t)start, oref);
        }
        else {
            /* go further down the path */
            int ret = ts_obj_by_name(parent, start, (uintptr_t)end - (uintptr_t)start, &parent);
            if (ret != 0) {
                break;
            }
            start = end + 1; /* Skip slash */
        }
    }

    TS_LOGD("OBJ: %s did not find child '%.*s' of parent object (%u:%u)",
             __func__, (int)len, path, (unsigned int)parent.db_id, (unsigned int)parent.db_oid);
    return -ENODATA;
}

const struct ts_obj *ts_obj(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (!ts_obj_db_oref_is_object(oref)) {
        return NULL;
    }

    const struct ts_obj_db *db = ts_obj_db_by_id(oref.db_id);
    const struct ts_obj *obj;
    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Local context database */
        obj = &db->objects[oref.db_oid];
    }
    else {
        /* Remote context database objects have mutable meta data in meta default */
        obj = &db->rdb->remotes[oref.db_oid];
    }
    return obj;
}

struct ts_obj_meta *ts_obj_meta(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (!ts_obj_db_oref_is_object(oref)) {
        return NULL;
    }

    const struct ts_obj_db *db = ts_obj_db_by_id(oref.db_id);
    struct ts_obj_meta *meta;
    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Local context database */
        meta = &db->meta[oref.db_oid];
    }
    else {
        /* Remote context database objects have mutable meta data in meta default */
        meta = &db->rdb->remotes[oref.db_oid].meta_default;
    }
    return meta;
}

const struct ts_obj_meta *ts_obj_meta_default(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (!ts_obj_db_oref_is_object(oref)) {
        return NULL;
    }

    const struct ts_obj_db *db = ts_obj_db_by_id(oref.db_id);
    const struct ts_obj_meta *meta;
    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Local context database */
        meta = &db->objects[oref.db_oid].meta_default;
    }
    else {
        /* Remote context database */
        meta = &db->rdb->remotes[oref.db_oid].meta_default;
    }
    return meta;
}

int ts_obj_child_count(thingset_oref_t parent_oref, uint16_t *count)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(parent_oref),
              "OBJ: %s on invalid parent object reference (%u:%u)", __func__,
              (unsigned int)parent_oref.db_id, (unsigned int)parent_oref.db_oid);

    if (!ts_obj_db_oref_is_tree(parent_oref)) {
        return -EINVAL;
    }

    if ((ts_obj_type(parent_oref) == TS_T_GROUP) ||
        (ts_obj_type(parent_oref) == TS_T_EXEC) ||
        (ts_obj_type(parent_oref) == TS_T_SUBSET)) {
        int num_elements = 0;
        thingset_oref_t child_oref = parent_oref;
        for (child_oref.db_oid = 0; ts_obj_db_oref_is_valid(child_oref); child_oref.db_oid++) {
            if (child_oref.db_oid == parent_oref.db_oid) {
                /* That are we - never count */
                continue;
            }
            if (ts_obj_parent_id(child_oref) == ts_obj_id(parent_oref)) {
                num_elements++;
            }
        }
        *count = num_elements;
    }
    else {
        *count = 0;
    }
    return 0;
}

int ts_obj_child_first(thingset_oref_t parent_oref, thingset_oref_t *child_oref)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(parent_oref),
              "OBJ: %s on invalid parent object reference (%u:%u)",
              __func__, (unsigned int)parent_oref.db_id, (unsigned int)parent_oref.db_oid);

    if (!ts_obj_db_oref_is_tree(parent_oref)) {
        return -EINVAL;
    }

    ts_obj_id_t parent_id = ts_obj_id(parent_oref);
    if ((ts_obj_type(parent_oref) == TS_T_GROUP) ||
        (ts_obj_type(parent_oref) == TS_T_EXEC) ||
        (ts_obj_type(parent_oref) == TS_T_SUBSET)) {
        thingset_oref_t oref = parent_oref;
        for (oref.db_oid = 0; ts_obj_db_oref_is_valid(oref); oref.db_oid++) {
            if (oref.db_oid == parent_oref.db_oid) {
                /* That are we - never count */
                continue;
            }
            if (ts_obj_parent_id(oref) == parent_id) {
                *child_oref = oref;
                return 0;
            }
        }
    }
    else {
        return -EINVAL;
    }
    return -ENODATA;
}

int ts_obj_child_next(thingset_oref_t *child_oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(*child_oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)child_oref->db_id, (unsigned int)child_oref->db_oid);

    if (!ts_obj_db_oref_is_object(*child_oref)) {
        return -EINVAL;
    }

    ts_obj_id_t parent_id = ts_obj_id(*child_oref);
    thingset_oref_t oref = *child_oref;
    for (; ts_obj_db_oref_is_valid(oref); oref.db_oid++) {
        if (oref.db_oid == child_oref->db_oid) {
            /* That are we - never count */
            continue;
        }
        if (ts_obj_parent_id(oref) == parent_id) {
            *child_oref = oref;
            return 0;
        }
    }

    return -ENODATA;
}

ts_obj_id_t ts_obj_id(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        return TS_ID_ROOT;
    }

    return ts_obj(oref)->id;
}

const char *ts_obj_name(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        return "/";
    }

    return ts_obj(oref)->name;
}

uint16_t ts_obj_subsets(thingset_oref_t oref)
{
    uint16_t object_subsets;

    if (!ts_obj_db_oref_is_object(oref)) {
        TS_ASSERT(true, "OBJ: %s on invalid object reference (%u:%u)", __func__,
                  (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        object_subsets = 0;
    }
    else {
        object_subsets = ts_obj_meta(oref)->subsets;
    }
    return object_subsets;
}

int ts_obj_subsets_add(thingset_oref_t oref, uint16_t subsets)
{
    if (!ts_obj_db_oref_is_object(oref)) {
        TS_ASSERT(true, "OBJ: %s on invalid object reference (%u:%u)", __func__,
                  (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        return -EINVAL;
    }
    ts_obj_meta(oref)->subsets |= subsets;
    return 0;
}

int ts_obj_subsets_remove(thingset_oref_t oref, uint16_t subsets)
{
    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        return 0;
    }
    if (!ts_obj_db_oref_is_object(oref)) {
        TS_ASSERT(true, "OBJ: %s on invalid object reference (%u:%u)", __func__,
                  (unsigned int)oref.db_id, (unsigned int)oref.db_oid);
        return -EINVAL;
    }
    ts_obj_meta(oref)->subsets &= ~subsets;
    return 0;
}

uint8_t ts_obj_type(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_tree(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        /* Virtual root object is of type TS_T_GROUP */
        return TS_T_GROUP;
    }

    return ts_obj(oref)->type;
}

int16_t ts_obj_detail(thingset_oref_t oref)
{
    int16_t object_detail;

    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        object_detail = 0;
    }
    else if (!ts_obj_db_oref_is_object(oref)) {
        TS_ASSERT(true, "%s refers to invalid database object with database oid %u", __func__,
                  (unsigned int)oref.db_oid);
        object_detail = 0;
    }
    else {
        object_detail = ts_obj_meta(oref)->detail;
    }
    return object_detail;
}

int ts_obj_db_alloc(thingset_uid_t *ctx_uid, ts_obj_id_t obj_id, thingset_oref_t *oref)
{
    /* Search remote objects database for context given by uid - create database if not available */
    ts_obj_db_id_t did;
    ts_obj_db_id_t empty_did = TS_OBJ_DB_ID_INVALID;
    ts_obj_db_id_t zero_did = TS_OBJ_DB_ID_INVALID;
    for (did = TS_CONFIG_LOCAL_COUNT; did < TS_ARRAY_SIZE(ts_obj_dbs); did++) {
        /* database for remote context objects */
        struct ts_obj_rdb *rdb = &ts_obj_rdbs[did - TS_CONFIG_LOCAL_COUNT];
        if (rdb->db.id == TS_OBJ_DB_ID_INVALID) {
            if (empty_did == TS_OBJ_DB_ID_INVALID) {
                empty_did = did;
            }
            continue;
        }
        if (rdb->uuid == *ctx_uid) {
            /* Database already available - search for object with given ThingSet object ID */
            TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(did, 0), oref_avail) {
                if (ts_obj_id(oref_avail) == obj_id) {
                    /* Already allocated */
                    *oref = oref_avail;
                    return 0;
                }
            }
            break;
        }
        if (rdb->db.num == 0) {
            if (zero_did == TS_OBJ_DB_ID_INVALID) {
                zero_did = did;
            }
        }
    }
    if (did >= TS_ARRAY_SIZE(ts_obj_dbs)) {
        /* No database with given ctx_uid. */
        if (empty_did == TS_OBJ_DB_ID_INVALID) {
            if (zero_did == TS_OBJ_DB_ID_INVALID) {
                return -ENOMEM;
            }
            /* Free available database with zero objects and use space again */
            did = zero_did;
        }
        else {
            /* Use empty slot */
            did = empty_did;
        }
        struct ts_obj_rdb *rdb = &ts_obj_rdbs[did - TS_CONFIG_LOCAL_COUNT];
        rdb->uuid = *ctx_uid;
        rdb->db.id = did;
    }
    for (ts_obj_db_oid_t oid = 0; oid < TS_CONFIG_REMOTE_OBJECT_COUNT; oid++) {
        if (ts_obj_remotes_mngmt[oid].ref_count == 0) {
            /* Init empty Slot */
            TS_ASSERT(ts_obj_remotes[oid].name == NULL,
                      "OBJ: %s data object with name assigned - database #%u object id %u",
                      __func__, (unsigned int)did, (unsigned int)oid);
            TS_ASSERT(ts_obj_remotes[oid].data == NULL,
                      "OBJ: %s data object with data assigned - database #%u object id %u",
                      __func__, (unsigned int)did, (unsigned int)oid);
            ts_obj_remotes[oid].meta_default.db_id = did;
            ts_obj_remotes[oid].meta_default.access = 0;
            ts_obj_remotes[oid].meta_default.detail = 0;
            ts_obj_remotes[oid].meta_default.subsets = 0;
            ts_obj_remotes[oid].id = obj_id;
            ts_obj_remotes[oid].type = TS_T_INVALID;
            ts_obj_remotes[oid].parent = TS_ID_INVALID;

            /* Increment number of objects of database */
            struct ts_obj_rdb *rdb = &ts_obj_rdbs[did - TS_CONFIG_LOCAL_COUNT];
            rdb->db.num++;

            /* Return object reference */
            oref->db_id = did;
            oref->db_oid = oid;

            return 0;
        }
    }
    return -ENOMEM;
}

int ts_obj_unref(thingset_oref_t oref)
{
    if (!ts_obj_db_oref_is_object(oref)) {
        return -EINVAL;
    }

#if TS_CONFIG_REMOTE_COUNT == 0
    return 0;
#else
    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Ignore - this is a static local database */
        return 0;
    }

    if (ts_obj_remotes_mngmt[oref.db_oid].ref_count == 0) {
        /* Unref on already freed object !?! */
        return 0;
    }

    ts_obj_remotes_mngmt[oref.db_oid].ref_count--;
    if (ts_obj_remotes_mngmt[oref.db_oid].ref_count == 0) {
        /* Free object */
        (void)ts_obj_data_free(oref);
        struct ts_obj *obj = ts_obj_writable(oref);
        obj->type = TS_T_INVALID;
        obj->id = TS_ID_INVALID;
        obj->meta_default.db_id = TS_OBJ_DB_ID_INVALID;

        /* Decrement number of objects of database */
        struct ts_obj_rdb *rdb = &ts_obj_rdbs[oref.db_id - TS_CONFIG_LOCAL_COUNT];
        rdb->db.num--;
    }

    return 0;
#endif
}

int ts_obj_data_alloc(thingset_oref_t oref, uint16_t size)
{
#if TS_CONFIG_REMOTE_COUNT == 0
    return -EINVAL;
#else
    if (!ts_obj_db_oref_is_object(oref)) {
        return -EINVAL;
    }

    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* We can not change data size of static database */
        return -EINVAL;
    }

    struct ts_obj *obj = ts_obj_writable(oref);
    if (obj->data != NULL) {
        ts_mem_free(&ts_obj_remotes_data, obj->data);
    }

    return ts_mem_alloc(&ts_obj_remotes_data, size, THINGSET_TIMEOUT_IMMEDIATE, &obj->data);
#endif
}

int ts_obj_data_free(thingset_oref_t oref)
{
#if TS_CONFIG_REMOTE_COUNT == 0
    return -EINVAL;
#else
    if (!ts_obj_db_oref_is_object(oref)) {
        return -EINVAL;
    }

    if (oref.db_id < TS_CONFIG_LOCAL_COUNT) {
        /* Ignore - this is a static local database */
        return 0;
    }

    struct ts_obj *obj = ts_obj_writable(oref);
    if (obj->data != NULL) {
        ts_mem_free(&ts_obj_remotes_data, obj->data);
        obj->data = NULL;
    }

    return 0;
#endif
}

struct ts_obj *ts_obj_writable(thingset_oref_t oref)
{
    TS_ASSERT((oref.db_id < TS_ARRAY_SIZE(ts_obj_dbs)) && (oref.db_id >= TS_CONFIG_LOCAL_COUNT),
              "OBJ: %s refers to data object with invalid database id %u", __func__,
              (unsigned int)oref.db_id);
    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        return NULL;
    }

    /* Remote context database */
    TS_ASSERT(oref.db_oid < TS_CONFIG_REMOTE_OBJECT_COUNT,
              "OBJ: %s refers to data object with invalid database #%u object id %u", __func__,
              (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

#if TS_CONFIG_REMOTE_OBJECT_COUNT == 0
    return NULL;
#else
    return &ts_obj_remotes[oref.db_oid];
#endif
}

void ts_obj_init_name(thingset_oref_t oref, const char *name)
{
    struct ts_obj *obj = ts_obj_writable(oref);
    if (obj->name != NULL) {
        ts_mem_free(&ts_obj_remotes_data, obj->name);
    }
    size_t name_len = strlen(name) + 1;
    void *name_mem;
    int ret = ts_mem_alloc(&ts_obj_remotes_data, name_len, THINGSET_TIMEOUT_IMMEDIATE, &name_mem);
    if (ret != 0) {
        obj->name = NULL;
    }
    else {
        strncpy(name_mem, name, name_len);
        obj->name = name_mem;
    }
}
