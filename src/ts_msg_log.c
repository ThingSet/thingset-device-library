/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Logging support
 * ---------------
 */

#include "thingset_env.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ts_msg.h"
#include "ts_obj.h"


struct ts_msg_log_cbor_token {
    char *log;
    size_t len;
};

/**
 * @brief CborStreamFunction for output to log buffer.
 */
static CborError ts_msg_log_cbor(void *token, const char *fmt, ...)
{
    struct ts_msg_log_cbor_token *tok = (struct ts_msg_log_cbor_token *)token;

    va_list args;
    va_start(args, fmt);
    size_t len = vsnprintf(tok->log, tok->len, fmt, args);
    va_end(args);

    tok->log = &tok->log[len];
    tok->len -= len;

    return CborNoError;
}

int ts_msg_log(struct thingset_msg* msg, char *log, size_t len)
{
    int ret = 0;
    size_t log_pos = 0;

    /* Report actual buffer positions */
    log_pos += snprintf(&log[log_pos], len - log_pos, "%.4u,%.4u,%.4u,",
                        (unsigned int)ts_msg_headroom(msg), (unsigned int)ts_msg_len(msg),
                        (unsigned int)ts_msg_tailroom(msg));

    /* Report authorisation rights */
    log_pos += snprintf(&log[log_pos], len - log_pos, "h'%.4x',", (unsigned int)ts_msg_auth(msg));

    /* remember actual message buffer position */
    struct ts_msg_state msg_state;
    ts_msg_state_save(msg, &msg_state);

    /* rewind to start of message */
    (void)ts_msg_push(msg, ts_msg_headroom(msg));

    /* Get command */
    uint8_t command_val;
    ret = ts_msg_pull_u8(msg, &command_val);
    if (ret != 0) {
        log_pos += snprintf(&log[log_pos], len - log_pos, "error(\"initial command missing.\")");
        goto end;
    }
    const char *command = "simple";
    bool proto_bin = false;
    bool proto_response = false;
    switch (command_val) {
    case TS_MSG_CODE_BIN_GET:
        command = "bin-get";
        proto_bin = true;
        break;
    case TS_MSG_CODE_BIN_POST:
        command = "bin-get";
        proto_bin = true;
        break;
    case TS_MSG_CODE_BIN_DELETE:
        command = "bin-delete";
        proto_bin = true;
        break;
    case TS_MSG_CODE_BIN_FETCH:
        command = "bin-fetch";
        proto_bin = true;
        break;
    case TS_MSG_CODE_BIN_PATCH:
        command = "bin-ipatch";
        proto_bin = true;
        break;
    case TS_MSG_CODE_BIN_STATEMENT:
        command = "bin-statement";
        proto_bin = true;
        break;
    case TS_MSG_CODE_TXT_GET:
        command = "txt-get";
        break;
    case TS_MSG_CODE_TXT_PATCH:
        command = "txt-patch";
        break;
    case TS_MSG_CODE_TXT_CREATE:
        command = "txt-create";
        break;
    case TS_MSG_CODE_TXT_DELETE:
        command = "txt-delete";
        break;
    case TS_MSG_CODE_TXT_EXEC:
        command = "txt-exec";
        break;
    case TS_MSG_CODE_TXT_STATEMENT:
        command = "txt-statement";
        break;
    case TS_MSG_CODE_TXT_RESPONSE:
        command = "txt-reponse";
        proto_response = true;
        break;
    default:
        if (command_val >= 0x80U) {
            command = "bin-reponse";
            proto_bin = true;
            proto_response = true;
        }
        else {
            log_pos += snprintf(&log[log_pos], len - log_pos, "ERROR: invalid command (0x%.2x).",
                                (unsigned int)command_val);
            goto end;
        }
        break;
    }
    if (proto_response && !proto_bin) {
        /* txt response */
        log_pos += snprintf(&log[log_pos], len - log_pos, "%s(\"%c\"),", command, command_val);
        ts_msg_status_code_t code;
        const char *desc;
        uint16_t desc_len;
        ret = ts_msg_pull_status(msg, &code, &desc, &desc_len);
        if (ret != 0) {
            log_pos += snprintf(&log[log_pos], len - log_pos, "ERROR: invalid/ missing status.");
            goto end;
        }
        log_pos += snprintf(&log[log_pos], len - log_pos, "h'%.2x',\"%.*s\"", code,
                            desc_len, desc);
    }
    else if (proto_bin) {
        /* bin request and bin response */
        log_pos += snprintf(&log[log_pos], len - log_pos, "%s(h'%.2x')", command,
                            (unsigned int)command_val);
    }
    else {
        /* txt request */
        const char *path;
        uint16_t path_len;
        ret = ts_msg_pull_path(msg, &path, &path_len);
        if (ret != 0) {
            log_pos += snprintf(&log[log_pos], len - log_pos, "ERROR: path missing.");
            goto end;
        }
        if ((command_val == '?') && (ts_msg_len(msg) > 0)) {
            /* Some data is following */
            command = "txt-fetch";
        }
        log_pos += snprintf(&log[log_pos], len - log_pos, "%s(\"%c\"),\"%.*s\"",
                            command, command_val,
                            path_len, path != NULL ? path : "");
    }

    if (proto_bin) {
        /* Cbor message type */
        CborError error;
        struct ts_msg_log_cbor_token token;
        CborValue it;
        CborParser parser;
        CborType type;
        const uint8_t *next_byte;

        error = cbor_parser_init(ts_msg_data(msg), ts_msg_len(msg), 0, &parser, &it);
        if (!cbor_value_at_end(&it)) {
            log_pos += snprintf(&log[log_pos], len - log_pos, ",");
        }
        while (!cbor_value_at_end(&it) && (log_pos < len) && (error == CborNoError)) {
            token.log = &log[log_pos];
            token.len = len - log_pos;
            type = cbor_value_get_type(&it);
            error = cbor_value_to_pretty_stream(ts_msg_log_cbor, (void *)&token, &it,
                                                CborPrettyDefaultFlags);
            if (error == CborErrorUnknownType) {
                TS_LOGE("CBOR: unknown type: %u (major type: %u, 5bit value: %u)",
                        (unsigned int)type,
                        (unsigned int)(it.source.ptr[0] >> 5),
                        (unsigned int)(it.source.ptr[0] & 0x01FU));
            }
            next_byte = cbor_value_get_next_byte(&it);
            if (next_byte < ts_msg_tail(msg)) {
                error = cbor_parser_init(next_byte, next_byte - ts_msg_tail(msg), 0, &parser, &it);
            }
            log_pos = token.log - log;
            if (log_pos >= len) {
                goto end;
            }
            log[log_pos] = ',';
            log_pos++;
        }
    }
    else {
        /* JSON message type */
        if (ts_msg_len(msg) > 1) {
            log_pos += snprintf(&log[log_pos], len - log_pos, ",%.*s",
                                ts_msg_len(msg) - 1, ts_msg_data(msg) + 1);
        }
    }
end:
    /* terminate log buffer */
    if (log_pos >= len) {
        log_pos = len - 1;
    }
    else if (log[log_pos - 1] == ',') {
        log_pos -= 1;
    }
    log[log_pos] = '\0';

    /* wind buffer to saved position */
    ts_msg_state_restore(msg, &msg_state);

    return log_pos;
}
