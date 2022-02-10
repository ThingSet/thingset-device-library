/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell app
 */

#include <ctype.h>

#include "../../src/thingset.h"

#include "../../src/ts_mem.h"
#include "../../src/ts_obj.h"
#include "../../src/ts_port.h"
#include "../../src/ts_msg.h"
#include "../../src/ts_ctx.h"
#include "../../src/ts_app.h"

#include "ts_shell.h"

/* Shell application */

const struct ts_shell_config ts_shell_config;

struct ts_shell_data ts_shell_data;

#if TS_CONFIG_SHELL_MEM_SIZE > 0
TS_MEM_DEFINE(ts_shell_mem, TS_CONFIG_SHELL_MEM_SIZE);
#endif

THINGSET_APP_DEFINE(TS_CONFIG_SHELL_PORTID, TS_CONFIG_SHELL_NAME, TS_CONFIG_SHELL_LOCID,
                    ts_shell_init, ts_shell_run, ts_shell_config, ts_shell_data);

int ts_shell_init(const struct thingset_app *app)
{
    /* Ignore app - shell is a singleton that may be called with app == NULL */
    app = &TS_CAT(ts_port_config_, TS_CONFIG_SHELL_PORTID);

    /* By default we route to the local context the shell is attached to */
    ts_shell_data.port_id_dst = THINGSET_PORT_ID_INVALID;
    ts_shell_data.response_size_hint = 500;

    return ts_impl_shell_init(app);
}

/* Shell commands */

const char *ts_shell_help           = "ts [node | obj | port | txt]";
const char *ts_shell_help_node      = "ts node [list | ...]";
const char *ts_shell_help_obj       = "ts obj [dump | ...]";
const char *ts_shell_help_port      = "ts port [list | ...]";
const char *ts_shell_help_txt       = "ts txt <text message>";

int ts_shell_cmd_txt(const struct ts_shell *shell, size_t argc, char **argv)
{
    if (argc <= 1) {
        goto ts_shell_cmd_txt_help;
    }

    int ret;
    struct thingset_msg *request;
    struct thingset_msg *response;
    uint16_t msg_size = strlen(argv[1]);

    for (int i = 2; i < argc; i++) {
        msg_size += 1 + strlen(argv[i]);
    }

    ret = thingset_msg_alloc_json(msg_size, 10, &request);

    for (int i = 1; (i < argc) && (ret == 0); i++) {
        if (i == 1) {
            ret = ts_msg_json_enc_setup(request);
        }
        else {
            ret = ts_msg_add_u8(request, ' ');
        }
        if (ret != 0) {
            break;
        }
        ret = ts_msg_add_mem(request, (const uint8_t *)argv[i], strlen(argv[i]));
    }
    if (ret != 0) {
        return ret;
    }

    ret = ts_msg_proc_setup(request);
    if (ret != 0) {
        return ret;
    }
    ts_msg_proc_set_port_dest(request, ts_shell_data.port_id_dst);
    ts_msg_proc_set_resp_size(request, ts_shell_data.response_size_hint);

    ret = ts_app_request(TS_CONFIG_SHELL_PORTID, request, 10, &response);
    if (ret != 0) {
        return ret;
    }

    ts_shell_print(shell, "%.*s", (int)ts_msg_len(response), (const char *)ts_msg_data(response));
    ret = thingset_msg_unref(response);

    return ret;

ts_shell_cmd_txt_help:
    ts_shell_print(shell, "%s", ts_shell_help_txt);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_cmd_node(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ret = 0;

    if (TS_CTX_IS_CORE(TS_CONFIG_SHELL_LOCID)) {
        ts_shell_print(shell, "not supported by core context");
        ret = -ENOTSUP;
    }
    else if (argc <= 1) {
        goto ts_shell_cmd_node_help;
    }
    else if (strcmp("list", argv[1]) == 0) {
        struct ts_ctx_com_data *data = ts_ctx_com_data(TS_CONFIG_SHELL_LOCID);

        ts_shell_print(shell, "Nodes table (context 0x%" PRIx64 "):",
                    *ts_ctx_uid(TS_CONFIG_SHELL_LOCID));
        for (uint16_t i = 0; i < TS_ARRAY_SIZE(data->node_table.nodes); i++) {
            struct ts_ctx_node *node = &data->node_table.nodes[i];
            if (node->port_id == THINGSET_PORT_ID_INVALID) {
                /* empty */
                ts_shell_print(shell, "%u: <empty>", (unsigned int)i);
            }
            else {
                const char *port_name = thingset_port_name(node->port_id);
                const char *response_port_name;
                if (node->response_port_id == THINGSET_PORT_ID_INVALID) {
                    response_port_name = "<none>";
                }
                else {
                    response_port_name = thingset_port_name(node->response_port_id);
                }
                ts_shell_print(shell, "%u: 0x%" PRIx64
                                   " at port \"%s\" responding to port \"%s\" seen last at %u",
                            (unsigned int)i, node->ctx_uid, port_name, response_port_name,
                            node->last_seen_time);
            }
        }
    }
    else {
        goto ts_shell_cmd_node_help;
    }

    return ret;

ts_shell_cmd_node_help:
    ts_shell_print(shell, "%s", ts_shell_help_node);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_cmd_port(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ret = 0;

    if (TS_CTX_IS_CORE(TS_CONFIG_SHELL_LOCID)) {
        ts_shell_print(shell, "not supported by core context");
        ret = -ENOTSUP;
    }
    else if (argc <= 1) {
        goto ts_shell_cmd_port_help;
    }
    else if (strcmp("list", argv[1]) == 0) {
        ts_shell_print(shell, "Ports table (device):");
        for (thingset_portid_t port_id = 0; port_id < TS_ARRAY_SIZE(ts_ports); port_id++) {
            const char *name = thingset_port_name(port_id);
            if (name != NULL) {
                thingset_uid_t ctx_uid = *ts_ctx_uid(ts_port_by_id(port_id)->loc_id);
                ts_shell_print(shell, "%u: \"%s\" at context 0x%" PRIx64, (unsigned int)port_id, name,
                            ctx_uid);
            }
        }
    }
    else {
        goto ts_shell_cmd_port_help;
    }

    return ret;

ts_shell_cmd_port_help:
    ts_shell_print(shell, "%s", ts_shell_help_port);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_cmd_obj(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ret = 0;

    if (argc <= 1) {
        goto ts_shell_cmd_obj_help;
    }
    else if (strcmp("dump", argv[1]) == 0) {
        char dump_buffer[20000];
        thingset_obj_log(ts_obj_db_oref_root(ts_ctx_obj_db(TS_CONFIG_SHELL_LOCID)),
                         &dump_buffer[0], sizeof(dump_buffer));
        ts_shell_print(shell, "%s", dump_buffer);
    }
    else {
        goto ts_shell_cmd_obj_help;
    }

    return ret;

ts_shell_cmd_obj_help:
    ts_shell_print(shell, "%s", ts_shell_help_obj);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_cmd_ts(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ret;

    if (argc < 1) {
        goto ts_shell_cmd_ts_help;
    }
    else if (strcmp("node", argv[0]) == 0) {
        ret = ts_shell_cmd_node(shell, argc, &argv[0]);
    }
    else if (strcmp("obj", argv[0]) == 0) {
        ret = ts_shell_cmd_obj(shell, argc, &argv[0]);
    }
    else if (strcmp("port", argv[0]) == 0) {
        ret = ts_shell_cmd_port(shell, argc, &argv[0]);
    }
    else if (strcmp("txt", argv[0]) == 0) {
        ret = ts_shell_cmd_txt(shell, argc, &argv[0]);
    }
    else {
        goto ts_shell_cmd_ts_help;
    }

    return ret;

ts_shell_cmd_ts_help:
    ts_shell_print(shell, "%s", ts_shell_help);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_cmd(const struct ts_shell *shell, size_t argc, char **argv)
{
    int ret;

    if (argc < 1) {
        goto ts_shell_cmd_help;
    }
    else if (strcmp("ts", argv[0]) == 0) {
        ret = ts_shell_cmd_ts(shell, argc - 1, &argv[1]);
    }
    else {
        goto ts_shell_cmd_help;
    }

    return ret;

ts_shell_cmd_help:
    ts_shell_print(shell, "%s", ts_shell_help);
    return TS_SHELL_CMD_HELP_PRINTED;
}

int ts_shell_split_cmd(char *cmd, int *argc, char ***argv)
{
    /* Find start of first command argument */
    char *start = cmd;
    while (isspace(*start) != 0) {
        *start = 0;
        start++;
    }

    /* Count occurance of delim in string */
    int count = 0;
    char *end = start;
    while (*end != 0) {
        /* Skip over non white space */
        while ((*end != 0) && (isspace(*end) == 0)) {
            /* Skip over escaped char */
            if (*end == '\\') {
                end += 2;
            }
            /* Skip over (") marked string */
            else if (*end == '"') {
                end++;
                while ((*end != 0) && (*end != '"')) {
                    /* Skip escaped \" */
                    if (*end == '\\') {
                        end += 2;
                    }
                    else {
                        end++;
                    }
                }
            }
            /* Skip over (') marked string */
            else if (*end == '\'') {
                end++;
                while ((*end != 0) && (*end != '\'')) {
                    /* Skip escaped \' */
                    if (*end == '\\') {
                        end += 2;
                    }
                    else {
                        end++;
                    }
                }
            }
            /* Skip over normal char */
            else {
                end++;
            }
        }

        /*
         * Null terminate the string at delimiter
         * and skip past our new null and any following delimiters.
         */
        while ((*end != 0) && (isspace(*end) != 0)) {
            *end = 0;
            end++;
        }
        count++;
    }

    /* Allocate dynamic array */
    char **arguments;
    int ret = ts_shell_alloc(count * sizeof(char *), 100, (uint8_t **)&arguments);
    if (ret != 0) {
        return ret;
    }

    /* Fill command argument vector */
    for (int i = 0; i < count; i++) {
        /* Copy start of string */
        TS_LOGD("SHELL: %s created command argument %d = \"%s\"", __func__, i, start);
        arguments[i] = start;
        /* Forward to next delimiter */
        while ((start < end) && (*start != 0)) {
            start++;
        }
        /* skip delimiter */
        while ((start < end) && (*start == 0)) {
            start++;
        }
    }

    *argv = arguments;
    *argc = count;

    return 0;
}

int ts_shell_alloc(size_t block_size, thingset_time_ms_t timeout_ms, uint8_t **mem_block)
{
#if TS_CONFIG_SHELL_MEM_SIZE == 0
    return -ENOTSUP;
#else
    return ts_mem_alloc(&ts_shell_mem, block_size, timeout_ms, (void **)mem_block);
#endif
}

int ts_shell_free(const uint8_t *mem_block)
{
#if TS_CONFIG_SHELL_MEM_SIZE == 0
    return -ENOTSUP;
#else
    return ts_mem_free(&ts_shell_mem, mem_block);
#endif
}
