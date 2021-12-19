/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet data objects (private interface)
 */

#ifndef TS_OBJ_H_
#define TS_OBJ_H_

/**
 * @brief ThingSet data objects.
 *
 * @defgroup ts_obj_api_priv ThingSet data objects (private interface)
 * @{
 */

#include "thingset.h"

#include "ts_mem.h"
#include "ts_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def TS_OBJ_DB_ID_INVALID
 *
 * @brief Invalid data object database identifier.
 */
#define TS_OBJ_DB_ID_INVALID (UINT16_MAX)

/**
 * @def TS_OBJ_DB_OID_INVALID
 *
 * @brief Invalid data object database object identifier.
 */
#define TS_OBJ_DB_OID_INVALID (UINT16_MAX)

/**
 * @def TS_OBJ_DB_OID_ROOT
 *
 * @brief Data object database object identifier for (virtual) root object.
 */
#define TS_OBJ_DB_OID_ROOT (TS_OBJ_DB_OID_INVALID - 1)

/**
 * @def TS_OBJ_DB_OID_ANY
 *
 * @brief Data object database object identifier for any (all) object(s).
 */
#define TS_OBJ_DB_OID_ANY (TS_OBJ_DB_OID_INVALID - 2)

/**
 * @def TS_OBJ_DB_FOREACH_OREF
 *
 * @brief Loop on all database object references of a given data object database.
 *
 * Use like:
 *
 *     TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(did, 0), my_oref) {
 *         printf("%s", ts_obj_name(my_oref))
 *     }
 *
 * @param[in] oref_start Starting object reference.
 * @param[in] oref_iter Name to be used for iterator object reference.
 */
#if TS_CONFIG_REMOTE_COUNT == 0
#define TS_OBJ_DB_FOREACH_OREF(oref_start, oref_iter)                                           \
    for (thingset_oref_t oref_iter = oref_start;                                                \
         ts_obj_db_oref_is_object(oref_iter); oref_iter.db_oid++)
#else
#define TS_OBJ_DB_FOREACH_OREF(oref_start, oref_iter)                                           \
    for (thingset_oref_t oref_iter = oref_start;                                                \
         ts_obj_db_oref_is_object(oref_iter); oref_iter.db_oid++)                               \
        if ((oref_iter.db_id < TS_CONFIG_LOCAL_COUNT) ?                                         \
            true : (ts_obj_remotes[oref_iter.db_oid].meta_default.db_id == oref_iter.db_id))
#endif

/**
 * @brief Remote context data object management data.
 */
struct ts_obj_remote_mngmt {
    /** @brief Reference count to remote object */
    uint8_t ref_count;
};

/**
 * @brief Remote context objects database.
 */
struct ts_obj_rdb {
    /** @brief Local objects database interface for remote context objects database */
    struct ts_obj_db db;

    /** @brief Unique identifier for the ThingSet context of the remote objects database. */
    uint64_t uuid;

    /**  @brief Array of remote context data objects. */
    struct ts_obj *remotes;
};

/* Local context objects databases */
#if TS_CONFIG_LOCAL_COUNT > 0
extern const struct ts_obj_db ts_obj_db_0;
#endif
#if TS_CONFIG_LOCAL_COUNT > 1
extern const struct ts_obj_db ts_obj_db_1;
#endif
#if TS_CONFIG_LOCAL_COUNT > 2
extern const struct ts_obj_db ts_obj_db_2;
#endif
#if TS_CONFIG_LOCAL_COUNT > 3
extern const struct ts_obj_db ts_obj_db_3;
#endif
#if TS_CONFIG_LOCAL_COUNT > 4
extern const struct ts_obj_db ts_obj_db_4;
#endif
#if TS_CONFIG_LOCAL_COUNT > 5
#error "Local context databases limited to 5"
#endif

/**
 * @brief Remote contexts' data objects databases pool.
 */
extern struct ts_obj_rdb ts_obj_rdbs[TS_CONFIG_REMOTE_COUNT];

/**
 *@brief Data objects databases by database id.
 *
 * Array of pointers to data objects databases. Array index == database id.
 */
extern const struct ts_obj_db *const ts_obj_dbs[TS_CONFIG_LOCAL_COUNT + TS_CONFIG_REMOTE_COUNT];

/**
 * @brief Remote contexts' data objects pool.
 *
 * All remote context data objects databases share the same pool for remote objects.
 */
extern struct ts_obj ts_obj_remotes[TS_CONFIG_REMOTE_OBJECT_COUNT];

/**
 * @brief Management data for remote contexts' data objects pool.
 */
extern struct ts_obj_remote_mngmt ts_obj_remotes_mngmt[TS_CONFIG_REMOTE_OBJECT_COUNT];

/**
 * @var ts_obj_remotes_data
 *
 * @brief Memory pool for remote contexts' data objects data.
 */
TS_MEM_DECLARE(ts_obj_remotes_data);

/**
 * @brief Get data objects database by database ID.
 *
 * @param[in] did Data objects database ID.
 * @return Pointer to data objects database.
 */
static inline const struct ts_obj_db *ts_obj_db_by_id(ts_obj_db_id_t did)
{
    TS_ASSERT(did < TS_ARRAY_SIZE(ts_obj_dbs),
              "OBJ: %s invalid database id %u given with maximum database id %d", __func__,
              (unsigned int)did, (unsigned int)TS_ARRAY_SIZE(ts_obj_dbs));
    return ts_obj_dbs[did];
}

/**
 * @brief Get data objects database by object reference.
 *
 * @param[in] oref Object reference.
 * @return Pointer to data objects database.
 */
static inline const struct ts_obj_db *ts_obj_db_by_oref(thingset_oref_t oref)
{
    return ts_obj_db_by_id(oref.db_id);
}

/**
 * @brief Initialise ThingSet objects databases.
 */
void ts_obj_db_init(void);

/**
 * @brief Check ThingSet objects database for consistent usage of object ids.
 *
 * Object ids shall be unique within the database.
 *
 * @param[in] did Data objects database ID.
 * @return 0 on success, <0 on error.
 */
int ts_obj_db_check_id_duplicates(ts_obj_db_id_t did);

/**
 * @brief Create object reference to database object.
 *
 * @param[in] did Data objects database ID.
 * @param[in] oid Object identifier of the object within the data objects database.
 * @return object reference
 */
static inline thingset_oref_t ts_obj_db_oref(ts_obj_db_id_t did, ts_obj_db_oid_t oid)
{
    thingset_oref_t oref;

    oref.db_id = did;
    oref.db_oid = oid;

    return oref;
}

/**
 * @brief Initialise object reference to link to object database.
 *
 * The reference is still invalid - but it is linked to the database.
 *
 * @param[in] did Data objects database ID.
 * @param[out] oref Pointer to object reference.
 */
static inline void ts_obj_db_oref_init(ts_obj_db_id_t did, thingset_oref_t *oref)
{
    oref->db_id = did;
    oref->db_oid = TS_OBJ_DB_OID_INVALID;
}

/**
 * @brief Get object reference to root object.
 *
 * @param[in] did Data objects database ID.
 * @return Object reference to root object.
 */
thingset_oref_t ts_obj_db_oref_root(ts_obj_db_id_t did);

/**
 * @brief Get object reference to any (all) object(s).
 *
 * @param[in] did Data objects database ID.
 * @return Object reference to any (all) object(s).
 */
thingset_oref_t ts_obj_db_oref_any(ts_obj_db_id_t did);

/**
 * @brief Get object reference of database object.
 *
 * @param[in] object Pointer to ThingSet object.
 * @return object reference
 */
thingset_oref_t ts_obj_db_oref_by_object(const struct ts_obj *object);

/**
 * @brief Get object reference to object of given database with given object id.
 *
 * @param[in] did Data objects database ID.
 * @param[in] obj_id ThingSet object ID.
 * @param[out] oref Pointer to object reference.
 */
int ts_obj_db_oref_by_id(ts_obj_db_id_t did, ts_obj_id_t obj_id, thingset_oref_t *oref);

/**
 * @brief Is object reference a valid reference.
 *
 * Returns true also for reference to root object or any object.
 *
 * @param[in] oref Object reference.
 * @returns True if object reference is valid, false otherwise.
 */
bool ts_obj_db_oref_is_valid(thingset_oref_t oref);

/**
 * @brief Is the object reference referencing an object.
 *
 * Returns false for reference to root object or for reference to any object.
 *
 * @param[in] oref Object reference.
 * @returns True if object reference references a data object, false otherwise.
 */
static inline bool ts_obj_db_oref_is_object(thingset_oref_t oref)
{
    ts_obj_db_oid_t oid_max = (oref.db_id < TS_CONFIG_LOCAL_COUNT) ?
                              ts_obj_dbs[oref.db_id]->num : TS_CONFIG_REMOTE_OBJECT_COUNT;
    return oref.db_oid < oid_max;
}

/**
 * @brief Is the object reference referencing a single tree object.
 *
 * Returns also true for reference to root object, but false for reference to any object.
 *
 * @param[in] oref Object reference.
 * @returns True if object reference references a single tree object, false otherwise.
 */
static inline bool ts_obj_db_oref_is_tree(thingset_oref_t oref)
{
    ts_obj_db_oid_t oid_max = (oref.db_id < TS_CONFIG_LOCAL_COUNT) ?
                              ts_obj_dbs[oref.db_id]->num : TS_CONFIG_REMOTE_OBJECT_COUNT;
    return (oref.db_oid == TS_OBJ_DB_OID_ROOT) || (oref.db_oid < oid_max);
}

/**
 * @brief Is the object reference referencing any (all) object(s).
 *
 * @param[in] oref Object reference.
 * @returns True if the object reference is referencing any (all) object(s), false otherwise.
 */
static inline bool ts_obj_db_oref_is_any(thingset_oref_t oref)
{
    return (oref.db_oid == TS_OBJ_DB_OID_ANY) && ts_obj_db_oref_is_valid(oref);
}

/**
 * @brief Are the object references referencing the same object.
 *
 * @param[in] oref_a Object reference.
 * @param[in] oref_b Object reference.
 * @returns True if the same object is referenced, false otherwise.
 */
static inline bool ts_obj_db_oref_is_same(thingset_oref_t oref_a, thingset_oref_t oref_b)
{
    return (oref_a.db_id == oref_b.db_id) && (oref_a.db_oid == oref_b.db_oid);
}

/**
 * @brief Get object reference to object of given parent object with given name.
 *
 * @param[in] parent Object reference of parent object or any (all) object(s).
 * @param[in] name Pointer to name.
 * @param[in] len Length of name.
 * @param[out] oref Pointer to object reference.
 */
int ts_obj_by_name(thingset_oref_t parent, const char *name, size_t len, thingset_oref_t *oref);

/**
 * @brief Get object reference to object of given parent object with given path.
 *
 * @param[in] parent Object reference of parent object or any (all) object(s).
 * @param[in] path Pointer to path.
 * @param[in] len Length of path.
 * @param[out] oref Pointer to object reference.
 */
int ts_obj_by_path(thingset_oref_t parent, const char *path, size_t len, thingset_oref_t *oref);

/**
 * @brief Get object given by object reference.
 *
 * For reference to root object NULL is returned.
 *
 * @param[in] oref Object reference.
 * @returns Pointer to object if available, NULL otherwise.
 */
const struct ts_obj *ts_obj(thingset_oref_t oref);

/**
 * @brief Get mutable object metadata for object given by object reference.
 *
 * For reference to root object NULL is returned.
 *
 * @param[in] oref Object reference.
 * @returns Pointer to object mutable metadata if available, NULL otherwise.
 */
struct ts_obj_meta *ts_obj_meta(thingset_oref_t oref);

/**
 * @brief Get immutable object default metadata for object given by object reference.
 *
 * For reference to root object NULL is returned.
 *
 * @param[in] oref Object reference.
 * @returns Pointer to object immutable default metadata if available, NULL otherwise.
 */
const struct ts_obj_meta *ts_obj_meta_default(thingset_oref_t oref);

/**
 * @brief Get default access authorisation of database object.
 *
 * @param[in] oref Database object reference to object.
 * @return Default access authorisation of database object.
 */
static inline const uint16_t ts_obj_access_default(thingset_oref_t oref)
{
    return ts_obj_meta_default(oref)->access;
}

/**
 * @brief Get access authorisation of database object.
 *
 * @param[in] oref Database object reference to object.
 * @return Access authorisation of database object.
 */
static inline uint16_t ts_obj_access(thingset_oref_t oref)
{
    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        return TS_ANY_RW;
    }
    return ts_obj_meta(oref)->access;
}

/**
 * @brief Does authorisation provide read access to given database object.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] auth Authorisation.
 * @return true on read access granted, false otherwise.
 */
static inline bool ts_obj_access_read(thingset_oref_t oref, uint16_t auth)
{
    return (ts_obj_access(oref) & TS_READ_MASK & auth) != 0;
}

/**
 * @brief Does authorisation provide write access to given database object.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] auth Authorisation.
 * @return true on read access granted, false otherwise.
 */
static inline bool ts_obj_access_write(thingset_oref_t oref, uint16_t auth)
{
    return (ts_obj_access(oref) & TS_WRITE_MASK & auth) != 0;
}

/**
 * @brief Count the number of objects that are referencing the parent object as their parent.
 *
 * @param[in] parent_oref Database object reference to parent object.
 * @param[out] count Pointer to count of child objects.
 * @return 0 on success, <0 otherwise.
 */
int ts_obj_child_count(thingset_oref_t parent_oref, uint16_t *count);

/**
 * @brief Get reference to first object that is referencing the parent object as it's parent.
 *
 * @param[in] parent_oref Database object reference to parent object.
 * @param[in,out] child_oref Pointer to object reference to current child on invocation, next child
 *                           on return.
 * @return 0 on success, <0 otherwise.
 */
int ts_obj_child_first(thingset_oref_t parent_oref, thingset_oref_t *child_oref);

/**
 * @brief Get reference to next object that is referencing the same parent object as it's parent.
 *
 * @param[in,out] child_oref Pointer to object reference to current child on invocation, next child
 *                           on return.
 * @return 0 on success, <0 otherwise.
 */
int ts_obj_child_next(thingset_oref_t *child_oref);

/**
 * @brief Get object identifier.
 *
 * @param[in] oref Database object reference to object.
 * @return Object identifier or TS_ID_INVALID on invalid object reference.
 */
ts_obj_id_t ts_obj_id(thingset_oref_t oref);

/**
 * @brief Get object name.
 *
 * @param[in] oref Database object reference to object.
 * @return Pointer to object name or NULL on invalid object reference.
 */
const char *ts_obj_name(thingset_oref_t oref);

/**
 * @brief Get object parent identifier.
 *
 * @param[in] oref Database object reference to object.
 * @return Object parent identifier or TS_ID_INVALID on invalid object reference or no parent.
 */
static inline ts_obj_id_t ts_obj_parent_id(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    const struct ts_obj *object = ts_obj(oref);
    if (object == NULL) {
        return TS_ID_INVALID;
    }
    return object->parent;
}

/**
 * @brief Get the subsets the object is associated to.
 *
 * @param[in] oref Database object reference to object.
 * @return Subsets flags or 0 on invalid object reference.
 */
uint16_t ts_obj_subsets(thingset_oref_t oref);

/**
 * @brief Add subset flags to the subsets the object is associated to.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] subsets Subset flags to be added.
 * @returns 0 on success, <0 on invalid object reference.
 */
int ts_obj_subsets_add(thingset_oref_t oref, uint16_t subsets);

/**
 * @brief Remove subset flags from the subsets the object is associated to.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] subsets Subset flags to be removed.
 * @returns 0 on success, <0 on invalid object reference.
 */
int ts_obj_subsets_remove(thingset_oref_t oref, uint16_t subsets);

/**
 * @brief Get object type.
 *
 * @param[in] oref Database object reference to object.
 * @return Object type or TS_T_INVALID on invalid object reference.
 */
uint8_t ts_obj_type(thingset_oref_t oref);

static inline void *ts_obj_data(thingset_oref_t oref)
{
    TS_ASSERT(ts_obj_db_oref_is_object(oref), "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    return ts_obj(oref)->data;
}

/**
 * @brief Get object detail.
 *
 * @param[in] oref Database object reference to object.
 * @return Object detail or 0 on invalid object reference.
 */
int16_t ts_obj_detail(thingset_oref_t oref);

static inline struct ts_array_info *ts_obj_array_data(thingset_oref_t oref)
{
    return (struct ts_array_info *)ts_obj_data(oref);
}

/**
 * @brief Get booelean object data.
 *
 * @param[in] oref Database object reference to object.
 * @return Pointer to boolean data.
 */
static inline bool *ts_obj_bool_data(thingset_oref_t oref)
{
    TS_ASSERT((ts_obj_type(oref) == TS_T_BOOL),
              "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    return (bool *)ts_obj_data(oref);
}

/**
 * @brief Get decfrac object mantissa data.
 *
 * @param[in] oref Database object reference to object.
 * @return Pointer to mantissa data.
 */
static inline int32_t *ts_obj_decfrac_mantissa_data(thingset_oref_t oref)
{
    TS_ASSERT((ts_obj_type(oref) == TS_T_DECFRAC),
              "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    return (int32_t *)ts_obj_data(oref);
}

/**
 * @brief Get decfrac object exponent data.
 *
 * @param[in] oref Database object reference to object.
 * @return Exponent data or 0 on invalid object reference.
 */
static inline int16_t ts_obj_decfrac_exponent_data(thingset_oref_t oref)
{
    TS_ASSERT((ts_obj_type(oref) == TS_T_DECFRAC),
              "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    return ts_obj_detail(oref);
}

/**
 * @brief Get group or function object execution data.
 *
 * @param[in] oref Database object reference to object.
 * @return Pointer to execution function or NULL if there is no execution function.
 */
static inline ts_obj_exec_t ts_obj_exec_data(thingset_oref_t oref)
{
    TS_ASSERT((ts_obj_type(oref) == TS_T_GROUP) || (ts_obj_type(oref) == TS_T_EXEC),
              "OBJ: %s on invalid object reference (%u:%u)",
              __func__, (unsigned int)oref.db_id, (unsigned int)oref.db_oid);

    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        /* Virtual root object is of type TS_T_GROUP but does not have an exec function */
        return NULL;
    }

    return (ts_obj_exec_t)ts_obj_data(oref);
}

static inline float *ts_obj_f32_data(thingset_oref_t oref)
{
    return (float *)ts_obj_data(oref);
}

static inline int16_t *ts_obj_i16_data(thingset_oref_t oref)
{
    return (int16_t *)ts_obj_data(oref);
}

static inline int32_t *ts_obj_i32_data(thingset_oref_t oref)
{
    return (int32_t *)ts_obj_data(oref);
}

static inline int64_t *ts_obj_i64_data(thingset_oref_t oref)
{
    return (int64_t *)ts_obj_data(oref);
}

static inline uint8_t *ts_obj_mem_data(thingset_oref_t oref)
{
    return ((struct ts_bytes_buffer *)ts_obj_data(oref))->bytes;
}

static inline uint16_t *ts_obj_mem_len(thingset_oref_t oref)
{
    return &((struct ts_bytes_buffer *)ts_obj_data(oref))->num_bytes;
}

static inline char *ts_obj_string_data(thingset_oref_t oref)
{
    return (char *)ts_obj_data(oref);
}

static inline uint16_t *ts_obj_u16_data(thingset_oref_t oref)
{
    return (uint16_t *)ts_obj_data(oref);
}

static inline uint32_t *ts_obj_u32_data(thingset_oref_t oref)
{
    return (uint32_t *)ts_obj_data(oref);
}

static inline uint64_t *ts_obj_u64_data(thingset_oref_t oref)
{
    return (uint64_t *)ts_obj_data(oref);
}

/*
 * Remote data objects handling
 * ----------------------------
 */

/**
 * @brief Allocate object space in a database for remote objects.
 *
 * The function does not allocate data space for the object. This has to be provided later on based
 * on the object data type using @ref ts_obj_data_alloc().
 *
 * @param[in] ctx_uid Unique id of the remote context the data object belongs to.
 * @param[in] obj_id Id of the data object within the remote context's database.
 * @param[out] oref Pointer to object reference.
 * @return 0 on success, <0 on error.
 */
int ts_obj_db_alloc(thingset_uid_t *ctx_uid, ts_obj_id_t obj_id, thingset_oref_t *oref);

/**
 * @brief Free object and object data space in a database for remote objects.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @param[in] oref Object reference.
 * @return 0 on success, <0 on error.
 */
int ts_obj_unref(thingset_oref_t oref);

/**
 * @brief Allocate memory space for object data.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @note The functions requires memory to be provided by ts_mem_alloc().
 *
 * @param[in] oref Database object reference to object.
 * @param[in] size Size of data to be allocated.
 * @return 0 on success, <0 on error.
 */
int ts_obj_data_alloc(thingset_oref_t oref, uint16_t size);

/**
 * @brief Free memory space for object data.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @param[in] oref Database object reference to object.
 * @return 0 on success, <0 on error.
 */
int ts_obj_data_free(thingset_oref_t oref);

/**
 * @brief Get writeable object given by object reference.
 *
 * For reference to root object NULL is returned.
 *
 * @param[in] oref Object reference.
 * @returns Pointer to object if available, NULL otherwise.
 */
struct ts_obj *ts_obj_writable(thingset_oref_t oref);

/**
 * @brief Initialise object's detail info.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] detail Object detail info.
 */
static inline void ts_obj_init_detail(thingset_oref_t oref, int16_t detail)
{
    ts_obj_meta(oref)->detail = detail;
}

/**
 * @brief Initialise object's id.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] id Object id.
 */
static inline void ts_obj_init_id(thingset_oref_t oref, ts_obj_id_t id)
{
    ts_obj_writable(oref)->id = id;
}

/**
 * @brief Initialise object's name.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @note The functions requires memory to be provided by ts_mem_alloc().
 *
 * @param[in] oref Database object reference to object.
 * @param[in] name Object name.
 */
void ts_obj_init_name(thingset_oref_t oref, const char *name);

/**
 * @brief Initialise object's type.
 *
 * @note Database object reference must reference to database for remote context objects.
 *
 * @param[in] oref Database object reference to object.
 * @param[in] type Object type.
 */
static inline void ts_obj_init_type(thingset_oref_t oref, uint8_t type)
{
    ts_obj_writable(oref)->type = type;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_obj_api_priv -->
 */

#endif // TS_OBJ_H_
