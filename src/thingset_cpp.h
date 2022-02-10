/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet local context (public C++ interface)
 */

#ifndef THINGSET_CPP_H_
#define THINGSET_CPP_H_

#include "thingset_env.h"
#include "thingset_msg.h"
#include "thingset_obj.h"
#include "thingset_ctx.h"

#ifdef __cplusplus

extern "C" {

/* Provide C++ naming for C constructs. */
typedef ts_obj_id_t ThingSetObjectId;
typedef struct ts_bytes_buffer ThingSetBytesBuffer;
typedef struct ts_array_info ThingSetArrayInfo;
typedef const struct ts_obj ThingSetObject;

#if TS_CONFIG_CPP_LEGACY
/* compatibility to legacy CPP interface */
typedef ThingSetBytesBuffer TsBytesBuffer __attribute__((deprecated));
typedef ThingSetArrayInfo ArrayInfo __attribute__((deprecated));
typedef ThingSetObject DataNode __attribute__((deprecated));
#endif

} /* extern 'C' */

/**
 * @brief Main ThingSet class.
 *
 * Class ThingSet is a C++ shim for the C implementation of ThingSet.
 * See the respective C functions for a detailed description.
 *
 * Class ThingSet is a singleton and works on the single core variant ThingSet local context. The
 * context has to be defined by @ref THINGSET_CORE_DEFINE(). The associated data objects database
 * has to be defined by @ref THINGSET_CORE_DATABASE_DEFINE().
 */
class ThingSet
{
public:

    inline ThingSet(void)
    {
        (void)thingset_core_init();
    };

    inline int process(uint8_t *request, size_t req_len, uint8_t *response, size_t resp_size)
    {
        return thingset_core_process(request, req_len, response, resp_size);
    };

    inline void dump_json(ts_obj_id_t obj_id = TS_ID_ROOT, int level = 0)
    {
        thingset_dump_json(obj_id, level);
    };

    inline void set_authorisation(uint16_t flags)
    {
        thingset_authorisation_set(TS_CONFIG_CORE_LOCID, flags);
    };

    inline int txt_export(char *buf, size_t size, const uint16_t subsets)
    {
        return thingset_txt_export(buf, size, subsets);
    };

    inline int txt_statement(char *buf, size_t size, ThingSetObject *object)
    {
        return thingset_txt_statement(buf, size, object);
    };

    inline int txt_statement(char *buf, size_t size, const char *path)
    {
        return thingset_txt_statement_by_path(buf, size, path);
    };

    inline int txt_statement(char *buf, size_t size, ThingSetObjectId id)
    {
        return thingset_txt_statement_by_id(buf, size, id);
    };

    inline int bin_export(uint8_t *buf, size_t size, const uint16_t subsets)
    {
        return thingset_bin_export(buf, size, subsets);
    };

    inline int bin_import(uint8_t *buf, size_t size, uint16_t auth_flags, const uint16_t subsets)
    {
        return thingset_bin_import(buf, size, auth_flags, subsets);
    };

    inline int bin_statement(uint8_t *buf, size_t size, ThingSetObject *object)
    {
        return thingset_bin_statement(buf, size, object);
    };

    inline int bin_statement(uint8_t *buf, size_t size, const char *path)
    {
        return thingset_bin_statement_by_path(buf, size, path);
    };

    inline int bin_statement(uint8_t *buf, size_t size, ThingSetObjectId id)
    {
        return thingset_bin_statement_by_id(buf, size, id);
    };

    inline ThingSetObject *get_object(ThingSetObjectId id)
    {
        return thingset_object_by_id(id);
    };

    inline ThingSetObject *get_object(const char *name, size_t len, int32_t parent = -1)
    {
        return thingset_object_by_name(name, len, parent);
    };

    inline ThingSetObject *get_endpoint(const char *path, size_t len)
    {
        return thingset_object_by_path(path, len);
    };

    /*
     * Deprecated functions from ThingSet v0.3 interface
     */

    /**
     * Generate statement (previously known as publication message) in JSON format.
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
     * @param subset Flag to select which subset of data items should be published
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    inline int txt_pub(char *buf, size_t size, const uint16_t subset)
        __attribute__((deprecated))
    {
        buf[0] = '#';
        buf[1] = ' ';
        int ret = thingset_txt_export(&buf[2], size - 2, subset);
        return (ret > 0) ? 2 + ret : 0;
    };

    /**
     * Generate statement (previously known as publication message) in CBOR format.
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param buf_size Size of the message buffer, i.e. maximum allowed length of the message
     * @param subset Flag to select which subset of data items should be published
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    inline int bin_pub(uint8_t *buf, size_t size, const uint16_t subset)
        __attribute__((deprecated))
    {
        buf[0] = TS_PUBMSG;
        int ret = thingset_bin_export(&buf[1], size - 1, subset);
        return (ret > 0) ? 1 + ret : 0;
    };

    /**
     * Update data objects based on values provided by from other pub msg.
     *
     * @param buf Buffer containing pub message and data that should be written to the data objects
     * @param len Length of the data in the buffer
     * @param auth_flags Authentication flags to be used in this function (to override _auth_flags)
     * @param subset Subscribe channel (as bitfield)
     *
     * @returns ThingSet status code
     */
    inline int bin_sub(uint8_t *cbor_data, size_t len, uint16_t auth_flags, uint16_t subsets)
        __attribute__((deprecated))
    {
        return thingset_bin_import(cbor_data + 1, len - 1, auth_flags, subsets);
    };

};

#endif /* __cplusplus */

#endif /* THINGSET_CPP_H_ */
