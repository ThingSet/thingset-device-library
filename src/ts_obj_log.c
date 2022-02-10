/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_env.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ts_obj.h"
#include "ts_log.h"

static const char *ts_obj_log_access(uint16_t access)
{
    /* "-,-,-,-,-,-,-" */
    static char access_str[6 * sizeof("TS_MKR_W") + 3];

    if (access & ~TS_ANY_RW) {
        /* Invalid access specification */
        snprintf(&access_str[0], sizeof(access_str),
                 "\"Invalid access specification 0x%04x - do not set 0x%04x",
                 (unsigned int)access, (unsigned int)(access & ~TS_ANY_RW));
        return &access_str[0];
    }

    switch (access) {
    /* read/ write */
    case TS_ANY_RW:
        return "\"TS_ANY_RW\"";
        break;
    case TS_MKR_RW:
        return "\"TS_MKR_RW\"";
        break;
    case TS_EXP_RW:
        return "\"TS_EXP_RW\"";
        break;
    case TS_USR_RW:
        return "\"TS_USR_RW\"";
        break;
    /* write only */
    case TS_ANY_W:
        return "\"TS_ANY_W\"";
        break;
    case TS_MKR_W:
        return "\"TS_MKR_W\"";
        break;
    case TS_EXP_W:
        return "\"TS_EXP_W\"";
        break;
    case TS_USR_W:
        return "\"TS_USR_W\"";
        break;
    /* read only */
    case TS_ANY_R:
        return "\"TS_ANY_R\"";
        break;
    case TS_MKR_R:
        return "\"TS_MKR_R\"";
        break;
    case TS_EXP_R:
        return "\"TS_EXP_R\"";
        break;
    case TS_USR_R:
        return "\"TS_USR_R\"";
        break;
    }
    snprintf(&access_str[0], sizeof(access_str), "\"%s,%s,%s,%s,%s,%s\"",
             (access & TS_MKR_W) ? "TS_MKR_W" : "-",
             (access & TS_EXP_W) ? "TS_EXP_W" : "-",
             (access & TS_USR_W) ? "TS_USR_W" : "-",
             (access & TS_MKR_R) ? "TS_MKR_R" : "-",
             (access & TS_EXP_R) ? "TS_EXP_R" : "-",
             (access & TS_USR_R) ? "TS_USR_R" : "-");
    return &access_str[0];
}

static const char *ts_obj_log_type(uint8_t type)
{
    switch (type) {
    case TS_T_BOOL:
        return "\"TS_T_BOOL\"";
    case TS_T_UINT64:
        return "\"TS_T_UINT64\"";
    case TS_T_INT64:
        return "\"TS_T_INT64\"";
    case TS_T_UINT32:
        return "\"TS_T_UINT32\"";
    case TS_T_INT32:
        return "\"TS_T_INT32\"";
    case TS_T_UINT16:
        return "\"TS_T_UINT16\"";
    case TS_T_INT16:
        return "\"TS_T_INT16\"";
    case TS_T_FLOAT32:
        return "\"TS_T_FLOAT32\"";
    case TS_T_STRING:
        return "\"TS_T_STRING\"";
    case TS_T_BYTES:
        return "\"TS_T_BYTES\"";
    case TS_T_ARRAY:
        return "\"TS_T_ARRAY\"";
    case TS_T_DECFRAC:
        return "\"TS_T_DECFRAC\"";
    case TS_T_GROUP:
        return "\"TS_T_GROUP\"";
    case TS_T_EXEC:
        return "\"TS_T_EXEC\"";
    case TS_T_SUBSET:
        return "\"TS_T_SUBSET\"";
    }
    return "\"<unknown>\"";
}

/**
 * @brief Serialize a object value into a JSON string.
 *
 * @param[in] oref Database object reference to object which shall be serialized.
 * @param[in,out] log Pointer to the buffer where the JSON value should be stored.
 * @param[in] len Size of the buffer, i.e. maximum allowed length of the value.
 * @returns Length of data written to buffer or 0 in case of error.
 */
static int ts_obj_log_value(thingset_oref_t oref, char *log, size_t len)
{
    if (len == 0) {
        return 0;
    }

    size_t log_pos = 0;

    switch (ts_obj_type(oref)) {
#if TS_CONFIG_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIu64, *ts_obj_u64_data(oref));
        break;
    case TS_T_INT64:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIi64, *ts_obj_i64_data(oref));
        break;
#endif
    case TS_T_UINT32:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIu32, *ts_obj_u32_data(oref));
        break;
    case TS_T_INT32:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIi32, *ts_obj_i32_data(oref));
        break;
    case TS_T_UINT16:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIu16, *ts_obj_u16_data(oref));
        break;
    case TS_T_INT16:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIi16, *ts_obj_i16_data(oref));
        break;
    case TS_T_FLOAT32:
        {
            float value = *ts_obj_f32_data(oref);
            if (isnan(value) || isinf(value)) {
                /* JSON spec does not support NaN and Inf, so we need to use null instead */
                return snprintf(log, len, "null,");
            }
            else {
                log_pos = snprintf(&log[log_pos], len - log_pos, "%f", value);
            }
        }
        break;
#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%" PRIi32 "e%" PRIi16,
            *ts_obj_decfrac_mantissa_data(oref), ts_obj_decfrac_exponent_data(oref));
        break;
#endif
    case TS_T_BOOL:
        log_pos = snprintf(&log[log_pos], len - log_pos, "%s",
                (*ts_obj_bool_data(oref) == true ? "true" : "false"));
        break;
    case TS_T_EXEC:
        {
            log_pos = snprintf(&log[log_pos], len - log_pos, "[");
            thingset_oref_t child_oref = oref;
            for (child_oref.db_oid = 0; ts_obj_db_oref_is_valid(child_oref); child_oref.db_oid++) {
                if (ts_obj_parent_id(child_oref) == ts_obj_id(oref)) {
                    log_pos += snprintf(&log[log_pos], len - log_pos, "\"%s\",", ts_obj_name(child_oref));
                }
            }
            if (log_pos > 1) {
                log_pos--; // remove trailing comma
                log_pos += snprintf(&log[log_pos], len - log_pos, "]");
            }
            else {
                log_pos = 0;
                log_pos = snprintf(&log[log_pos], len - log_pos, "null");
            }
        }
        break;
    case TS_T_STRING:
        log_pos = snprintf(&log[log_pos], len - log_pos, "\"%s\"", ts_obj_string_data(oref));
        break;
    case TS_T_SUBSET:
        {
            thingset_oref_t child_oref = oref;
            log_pos = snprintf(&log[log_pos], len - log_pos, "[");
            for (child_oref.db_oid = 0; ts_obj_db_oref_is_valid(child_oref); child_oref.db_oid++) {
                if (ts_obj_subsets(child_oref) & (uint16_t)ts_obj_detail(oref)) {
                    log_pos += snprintf(&log[log_pos], len - log_pos, "\"%s\",", ts_obj_name(child_oref));
                }
            }
            if (log_pos > 1) {
                log_pos--; // remove trailing comma
            }
            log_pos += snprintf(&log[log_pos], len - log_pos, "]");
        }
        break;
    case TS_T_ARRAY:
        {
            struct ts_array_info *array_info = ts_obj_array_data(oref);
            if (!array_info) {
                return 0;
            }
            log_pos += snprintf(&log[log_pos], len - log_pos, "[");
            for (int i = 0; i < array_info->num_elements; i++) {
                switch (array_info->type) {
                case TS_T_UINT64:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIu64 ",",
                                    ((uint64_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT64:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIi64 ",",
                                    ((int64_t *)array_info->ptr)[i]);
                    break;
                case TS_T_UINT32:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIu32 ",",
                                    ((uint32_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT32:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIi32 ",",
                                    ((int32_t *)array_info->ptr)[i]);
                    break;
                case TS_T_UINT16:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIu16 ",",
                                    ((uint16_t *)array_info->ptr)[i]);
                    break;
                case TS_T_INT16:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%" PRIi16 ",",
                                    ((int16_t *)array_info->ptr)[i]);
                    break;
                case TS_T_FLOAT32:
                    log_pos += snprintf(&log[log_pos], len - log_pos, "%.*f,", ts_obj_detail(oref),
                                    ((float *)array_info->ptr)[i]);
                    break;
                default:
                    /* should not happen */
                    TS_ASSERT(false, "OBJ: %s on unknown ThingSet object type (%u)", __func__,
                              (unsigned int)array_info->type);
                    break;
                }
            }
            if (array_info->num_elements > 0) {
                log_pos--; // remove trailing comma
            }
            log_pos += snprintf(&log[log_pos], len - log_pos, "]");
        }
        break;
    default:
        TS_LOGD("OBJ: %s on unexpected type (%u)", __func__,
                (unsigned int)ts_obj_type(oref));
        break;
    }

    return log_pos;
}

/**
 * @brief Print all data objects as a structured JSON text to log buffer.
 *
 * @warning This is a recursive function and might cause stack overflows if run in constrained
 *          devices with large data object tree. Use with care and for testing only!
 *
 * @param[in] oref Database object reference to root object where to start with printing.
 * @param[in] log Pointer to the log buffer.
 * @param[in] len Length of the log buffer.
 * @param[in] level Indentation level (=depth inside the data object tree)
 * @returns Number of characters printed to buffer, 0 on error
 */
static size_t ts_obj_log_by_level(thingset_oref_t oref, char *log, size_t len, int level)
{
    TS_ASSERT(level < 10, "OBJ: %s on level %d exceeds maximum level (10)", __func__, level);

    if (len == 0) {
        return 0;
    }

    size_t log_pos = 0;

    bool first = true;
    if (oref.db_oid == TS_OBJ_DB_OID_ROOT) {
        log_pos += snprintf(&log[log_pos], len - log_pos, "{");
    }

    TS_OBJ_DB_FOREACH_OREF(ts_obj_db_oref(oref.db_id, 0), child_oref) {
        if (log_pos >= len) {
            break;
        }
        if (ts_obj_parent_id(child_oref) != ts_obj_id(oref)) {
            continue;
        }
        /* Add end of line to line before */
        if (first) {
            log_pos += snprintf(&log[log_pos], len - log_pos, "\n");
            first = false;
        }
        else {
            log_pos += snprintf(&log[log_pos], len - log_pos, ",\n");
        }
        if (log_pos >= len) {
            break;
        }
        /* Add indent & characteristics */
        /*
        log_pos += snprintf(&log[log_pos], len - log_pos,
                            "%*s// db_oid: %u, obj_id: %u, parent_id: %u, access: 0x%04x (default 0x%04x)\n",
                            4 * (level + 1), "", (unsigned int)child_oref.db_oid,
                            (unsigned int)ts_obj_id(child_oref),
                            (unsigned int)ts_obj_parent_id(child_oref),
                            (unsigned int)ts_obj_access(child_oref),
                            (unsigned int)ts_obj_access_default(child_oref));
        */
        /* Add object at indent (level + 1)  */
        log_pos += snprintf(&log[log_pos], len - log_pos, "%*s\"%s\": {",
                            4 * (level + 1), "", ts_obj_name(child_oref));
        if (log_pos >= len) {
            break;
        }
        /* Add database oid at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"db_oid\": %u,", 4 * (level + 2),
                            "", (unsigned int)child_oref.db_oid);
        if (log_pos >= len) {
            break;
        }
        /* Add object id at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"obj_id\": %u,", 4 * (level + 2),
                            "", (unsigned int)ts_obj_id(child_oref));
        if (log_pos >= len) {
            break;
        }
        /* Add object access at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"access\": %s,", 4 * (level + 2),
                            "", ts_obj_log_access(ts_obj_access(child_oref)));
        if (log_pos >= len) {
            break;
        }
        /* Add object access default at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"access_default\": %s,", 4 * (level + 2),
                            "", ts_obj_log_access(ts_obj_access_default(child_oref)));
        if (log_pos >= len) {
            break;
        }
        /* Add object type at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"type\": %s,", 4 * (level + 2),
                            "", ts_obj_log_type(ts_obj_type(child_oref)));
        if (log_pos >= len) {
            break;
        }
        /* Add object data at indent (level + 2) */
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s\"data\": ", 4 * (level + 2), "");
        if (ts_obj_type(child_oref) == TS_T_BYTES) {
            log_pos += snprintf(&log[log_pos], len - log_pos, "<bytes>");
        }
        else if (ts_obj_type(child_oref) == TS_T_GROUP) {
            log_pos += snprintf(&log[log_pos], len - log_pos, "{");
            log_pos += ts_obj_log_by_level(child_oref, &log[log_pos], len - log_pos, level + 2);
            log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s}", 4 * (level + 2), "");
        }
        else {
            log_pos += ts_obj_log_value(child_oref, &log[log_pos], len - log_pos);
        }
        if (log_pos >= len) {
            break;
        }

        log_pos += snprintf(&log[log_pos], len - log_pos, "\n%*s}", 4 * (level + 1), "");
        if (log_pos >= len) {
            break;
        }
    }
    if ((oref.db_oid == TS_OBJ_DB_OID_ROOT) && (len > log_pos + 4)) {
        log_pos += snprintf(&log[log_pos], len - log_pos, "\n}\n");
    }
    if (log_pos >= len) {
        log_pos = len - 1;
    }
    log[log_pos] = '\0';
    return log_pos;
}

void thingset_obj_log(thingset_oref_t oref, char *log, size_t len)
{
    (void)ts_obj_log_by_level(oref, log, len, 0);
}
