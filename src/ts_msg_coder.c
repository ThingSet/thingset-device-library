/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ThingSet message (en/de-)coder support
 * --------------------------------------
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ts_msg.h"

/* Helper */

int ts_msg_scratchpad_json_enc_update(struct thingset_msg *msg, const char *func, const char *s,
                                      uint16_t len)
{
    int ret = 0;
    struct ts_msg_json_encoder *encoder = ts_msg_scratchpad_json_enc_current(msg);

    if (s == NULL) {
        /* JSON string already added to message */
        (void)ts_msg_add(msg, len);
    }
    else if (len > ts_msg_tailroom(msg)) {
            TS_LOGE("JSON encoder: %s advance not possible (%u) - not enough memory (%u)", func,
                    (unsigned int)len, (unsigned int)ts_msg_tailroom(msg));
            return -ENOMEM;
    }
    else {
        (void)ts_msg_add_mem(msg, (const uint8_t *)s, len);
    }

    if (encoder->flags.map_unbounded) {
        if (encoder->flags.key) {
            /* We added a key */
            encoder->flags.key = 0;
            len += 1;
            ret = ts_msg_add_u8(msg, ':');
        }
        else {
            /* We added a value */
            encoder->flags.key = 1;
            len += 1;
            ret = ts_msg_add_u8(msg, ',');
        }
    }
    else if (encoder->flags.map) {
        if (encoder->remaining == 0) {
            TS_LOGE("JSON encoder: %s advance to next key/value after end of map", func);
            ret = -ENOMEM;
        }
        else if (encoder->flags.key) {
            /* We added a key */
            encoder->flags.key = 0;
            len += 1;
            ret = ts_msg_add_u8(msg, ':');
        }
        else {
            /* We added a value */
            encoder->flags.key = 1;
            encoder->remaining--;
            if (encoder->remaining) {
                len += 1;
                ret = ts_msg_add_u8(msg, ',');
            }
        }
    }
    else if (encoder->flags.array_unbounded) {
        len += 1;
        ret = ts_msg_add_u8(msg, ',');
    }
    else if (encoder->flags.array) {
        if (encoder->remaining == 0) {
            TS_LOGE("JSON encoder: %s advance to next value after end of array", func);
            ret = -ENOMEM;
        }
        else {
            encoder->remaining--;
            if (encoder->remaining) {
                len += 1;
                ret = ts_msg_add_u8(msg, ',');
            }
        }
    }
    else if (encoder->remaining != 0) {
        TS_LOGE("JSON encoder: %s remaining elements but no map or array", func);
        ret = -ENOMEM;
    }

    TS_LOGD("JSON encoder: %s advance by %u", func, len);
    return ret;
}

int ts_msg_json_dec_update(struct thingset_msg *msg, const char *func, const char *start,
                           uint16_t len)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->token_idx++;
    if (scratchpad->token_idx < ts_jsmn_token_count(&scratchpad->jsmn)) {
        /* There is a next token - advance message only to next token JSON text start */
        TS_ASSERT((ts_jsmn_token_start(&scratchpad->jsmn, scratchpad->token_idx)
                   >= (const char *)ts_msg_data(msg)),
                  "JSON decoder: %s expects token start increment", func);
        len = ts_jsmn_token_start(&scratchpad->jsmn, scratchpad->token_idx)
              - (const char *)ts_msg_data(msg);
    }
    else {
        /* Account for prefix white space that JSMN already removed */
        len += start - (const char *)ts_msg_data(msg);
    }
    ts_msg_pull(msg, len);
    TS_LOGD("JSON decoder: %s advance by %u", func, len);
    return 0;
}

int ts_msg_scratchpad_tinycbor_enc_update(struct thingset_msg *msg, const char *func, CborError error)
{
    if (error != CborNoError) {
        const char *error_msg;
        int ret;
        switch ((int)error) {
        case CborUnknownError:
            error_msg = "unknown";
            ret = -EFAULT;
            break;
        case CborErrorTooManyItems:
            error_msg = "too may items";
            ret = -E2BIG;
            break;
        case CborErrorTooFewItems:
            error_msg = "too few items";
            ret = -EINVAL;
            break;
        case CborErrorOutOfMemory:
            error_msg = "out of memory";
            ret = -ENOMEM;
            break;
        case CborErrorInternalError:
            error_msg = "internal";
            ret = -ENOTSUP;
            break;
        default:
            error_msg = "unexpected";
            ret = -EFAULT;
            break;
        }
        TS_LOGE("CBOR encoder: %s fails on %s (%s)", func, error_msg, cbor_error_string(error));
        return ret;
    }
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    uint16_t advance = cbor_encoder_get_buffer_size(encoder, ts_msg_tail(msg));
    (void)ts_msg_add(msg, advance);
    TS_LOGD("CBOR encoder: %s advance by %u", func, advance);
    return 0;
}

int ts_msg_scratchpad_tinycbor_dec_update(struct thingset_msg *msg, bool advance_value,
                                          const char *func, CborError error)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);

    if (error != CborNoError) {
        TS_LOGE("CBOR decoder: %s faulting (%s)", func, cbor_error_string(error));
        return -EINVAL;
    }
    if (advance_value) {
        if (ts_msg_len(msg) == 0) {
            TS_LOGD("CBOR decoder: %s advance to next value after end of message", func);
            return -ENOMEM;
        }
        error = cbor_value_advance(it);
        if (error != CborNoError) {
            TS_LOGE("CBOR decoder: %s faulting on advance to next value (%s)", func,
                    cbor_error_string(error));
            return -ENOMEM;
        }
    }
    uint16_t advance = cbor_value_get_next_byte(it) - ts_msg_data(msg);
    (void)ts_msg_pull(msg, advance);
    if (ts_msg_len(msg) == 0) {
        TS_LOGD("CBOR decoder: %s advanced by %u to message end", func, advance);
    }
    else if (ts_msg_scratchpad_tinycbor_dec_is_top(msg)) {
        /* reinit parser to next value - tinycbor handles top level different from container */
        TS_LOGD("CBOR decoder: %s advance top level by %u", func, advance);
        TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(decoder, msg);
        error = cbor_parser_init(ts_msg_data(msg), ts_msg_len(msg), 0,
                                 &decoder->parser, &decoder->value[0]);
        if (error != CborNoError) {
            TS_LOGE("CBOR decoder: %s faulting on top level advance to next value (%s)", func,
                    cbor_error_string(error));
            return -ENOMEM;
        }
    }
    else {
        TS_LOGD("CBOR decoder: %s advance sub level by %u", func, advance);
    }
    return 0;
}

int ts_msg_json_enc_setup(struct thingset_msg *msg)
{
    if (ts_buf_tailroom((struct ts_buf *)msg) < sizeof(struct ts_msg_json_enc_scratchroom)) {
        TS_LOGE("JSON encoder: %s faulting - msg tailroom less than minimum (%u)", __func__,
                (unsigned int)ts_buf_tailroom((struct ts_buf *)msg));
        return -ENOMEM;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_JSON_ENC;
    scratchpad_std->scratchroom.size = sizeof(struct ts_msg_json_enc_scratchroom);

    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->current = 0;

    scratchpad->instance[0].flags.array = 0;
    scratchpad->instance[0].flags.array_unbounded = 0;
    scratchpad->instance[0].flags.map = 0;
    scratchpad->instance[0].flags.map_unbounded = 0;
    scratchpad->instance[0].flags.key = 0;
    scratchpad->instance[0].remaining = 0;

    return 0;
}

int ts_msg_json_dec_setup(struct thingset_msg *msg)
{
    if (ts_buf_tailroom((struct ts_buf *)msg) < sizeof(struct ts_msg_json_dec_scratchroom)) {
        /* Minimum size not available - assure no negative size calculation later on */
        TS_LOGE("JSON decoder: %s faulting on message 0x%" PRIXPTR
                " - msg tailroom less than minimum (%u)", __func__, (uintptr_t)msg,
                (unsigned int)ts_buf_tailroom((struct ts_buf *)msg));
        return -ENOMEM;
    }

    int ret;
    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_JSON_DEC;
    /*
     * Assure alignment of 4 bytes of jsmn context
     * and at least one remaining tailroom byte for `\0`.
     *
     * Account for offset of JSMN context within JSMN scratchroom.
     */
    size_t offset = offsetof(struct ts_msg_json_dec_scratchroom, jsmn);
    uint16_t align = (uintptr_t)(ts_buf_tail((struct ts_buf *)msg) + offset) & 0x03U;
    if (align > 0) {
        align = 4 - align;
    }
    scratchpad_std->scratchroom.size = ts_buf_tailroom((struct ts_buf *)msg) - align;

    if (scratchpad_std->scratchroom.size
        < (sizeof(struct ts_msg_json_dec_scratchroom) + 16 * sizeof(struct ts_jsmn_token))) {
        /* We need a minimum of 16 token space. */
        TS_LOGE("JSON decoder: %s faulting on message 0x%" PRIXPTR
                " - msg tailroom does not provide for 16 tokens (%u)", __func__, (uintptr_t)msg,
                (unsigned int)(((size_t)scratchpad_std->scratchroom.size -
                               sizeof(struct ts_msg_json_dec_scratchroom)) / sizeof(struct ts_jsmn_token)));
        ret = -ENOMEM;
    }
    else {
        TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
        uint16_t num_tokens = (scratchpad_std->scratchroom.size - sizeof(struct ts_msg_json_dec_scratchroom))
                              / sizeof(struct ts_jsmn_token);
        ret = ts_jsmn_init(&scratchpad->jsmn, num_tokens);
        if (ret == 0) {
            /* Init pull index */
            scratchpad->token_idx = 0;
            /* Assure JSON data string ends with a '\0' for std string functions */
            if (*ts_msg_tail(msg) != 0) {
                TS_ASSERT(ts_msg_tailroom(msg) > 0, "wrong alignment of JSMN scratchpad");
                *ts_msg_tail(msg) = 0;
            }
            /* Parse whole JSON string */
            ret = ts_jsmn_parse(&scratchpad->jsmn, ts_msg_data(msg), ts_msg_len(msg));
            if ((ret == 0) && (ts_jsmn_token_count(&scratchpad->jsmn) == 0)) {
                TS_LOGD("JSON decoder: %s faulting - no JSON data in message", __func__);
                ret = -ENODATA;
            }
        }
    }
    if (ret != 0) {
        scratchpad_std->scratchroom.size = 0;
        scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_STD;
    }

    return ret;
}

int ts_msg_cbor_enc_setup(struct thingset_msg *msg)
{
    if (ts_buf_tailroom((struct ts_buf *)msg) < sizeof(struct ts_msg_cbor_enc_scratchroom)) {
        TS_LOGE("CBOR encoder: %s faulting - msg tailroom less than minimum (%u)", __func__,
                (unsigned int)ts_buf_tailroom((struct ts_buf *)msg));
        return -ENOMEM;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_CBOR_ENC;
    scratchpad_std->scratchroom.size = sizeof(struct ts_msg_cbor_enc_scratchroom);

    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->current = 0;
    cbor_encoder_init(&scratchpad->instance[0], ts_msg_tail(msg), ts_msg_tailroom(msg), 0);

    return 0;
}

int ts_msg_cbor_dec_setup(struct thingset_msg *msg)
{
    struct ts_buf *buf = (struct ts_buf *)msg;

    if (ts_buf_tailroom(buf) < sizeof(struct ts_msg_cbor_dec_scratchroom)) {
        TS_LOGE("CBOR decoder: %s faulting - msg tailroom less than minimum (%u)", __func__,
                (unsigned int)ts_buf_tailroom(buf));
        return -ENOMEM;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);

    scratchpad_std->scratchroom.size = sizeof(struct ts_msg_cbor_dec_scratchroom);
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_CBOR_DEC;

    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);

    CborError error = cbor_parser_init(ts_msg_data(msg), ts_msg_len(msg), 0,
                                       &scratchpad->parser, &scratchpad->value[0]);
    if (error == CborNoError) {
        error = cbor_value_validate_basic(&scratchpad->value[0]);
    }
    if (error != CborNoError) {
        scratchpad_std->scratchroom.size = 0;
        scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_STD;
        return -EINVAL;
    }
    scratchpad->current = 0;
    return 0;
}

int ts_msg_cobs_enc_setup(struct thingset_msg *msg)
{
    if (ts_msg_status_encoding(msg) != TS_MSG_ENCODING_NONE) {
        TS_LOGE("COBS encoder: %s faulting - expecting uncoded message (got %u)", __func__,
                (unsigned int)ts_msg_status_encoding(msg));
        return -EBADMSG;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);
    /* Set buf[2] (aka. size) to 0 bytes - buf[1] may be used by encoder */
    scratchpad_std->scratchroom.size = 0;
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_COBS_ENC;

    uint8_t *buf = ts_msg_data(msg);

    /* Add framing sentinel for encoding */
    TS_ASSERT(buf > &scratchpad_std->scratchroom.buf[1],
              "COBS encoder: %s faulting - msg data start at 0x%" PRIxPTR
              " overlaps scratchpad extra scratchroom buf[1] at 0x%" PRIxPTR, __func__,
              (uintptr_t)buf, (uintptr_t)&scratchpad_std->scratchroom.buf[1]);
    /*
     * Need to work on raw buffer as we may extend the message into std scratchroom at start of
     * message buffer using extra scratchroom buf[1].
     */
    buf = ts_buf_push((struct ts_buf *)msg, 1);
    buf[0] = TS_COBS_INPLACE_SENTINEL_VALUE;
    int ret = ts_msg_add_u8(msg, TS_COBS_INPLACE_SENTINEL_VALUE);
    if (ret != 0) {
        /* Revert starting sentinel */
        (void)ts_buf_pull((struct ts_buf *)msg, 1);
        return ret;
    }

    ret = ts_cobs_encode_inplace(ts_msg_len(msg), buf);
    if (ret != 0) {
        /* Revert starting & ending sentinel */
        (void)ts_buf_pull((struct ts_buf *)msg, 1);
        (void)ts_msg_remove(msg, 1);
        return ret;
    }

    scratchpad_std->status.encoding = TS_MSG_ENCODING_COBS;
    return 0;
}

int ts_msg_cobs_dec_setup(struct thingset_msg *msg)
{
    int ret = 0;

    if (ts_msg_status_encoding(msg) != TS_MSG_ENCODING_COBS) {
        TS_LOGE("COBS decoder: %s faulting - expecting COBS encoded message (got %u)", __func__,
                (unsigned int)ts_msg_status_encoding(msg));
        return -EBADMSG;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);

    uint8_t *buf = ts_msg_data(msg);
    if (scratchpad_std->scratchtype.id == TS_MSG_SCRATCHPAD_COBS_ENC) {
        /* Extra scratchroom buf[1] may be used for first byte of encoded message */
        if (buf < &scratchpad_std->scratchroom.buf[1]) {
            TS_LOGE("COBS decoder: %s faulting - msg data start at 0x%" PRIxPTR
                    " exceeds scratchpad extra scratchroom buf[1] at 0x%" PRIxPTR, __func__,
                    (uintptr_t)buf, (uintptr_t)&scratchpad_std->scratchroom.buf[1]);
            return -EBADMSG;
        }
    }

    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_COBS_DEC;

    uint16_t len = ts_msg_len(msg);

    if (buf[len - 1] != 0) {
        ret = ts_msg_add_u8(msg, 0);
        if (ret != 0) {
            return ret;
        }
        len += 1;
    }

    ret = ts_cobs_decode_inplace(len, buf);
    if (ret == 0) {
        /* Skip decoding sentinels at beginning and end of buffer */
        (void)ts_msg_pull(msg, 1);
        (void)ts_msg_remove(msg, 1);
        scratchpad_std->status.encoding = TS_MSG_ENCODING_NONE;
        scratchpad_std->scratchroom.size = 0;
        scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_STD;
    }

    return ret;
}

int ts_msg_proc_setup(struct thingset_msg *msg)
{
    struct ts_buf *buf = (struct ts_buf *)msg;

    if (ts_buf_tailroom(buf) < sizeof(struct ts_msg_proc_scratchroom)) {
        TS_LOGE("MSG processing: %s faulting - msg tailroom less than minimum (%u)", __func__,
                (unsigned int)ts_buf_tailroom(buf));
        return -ENOMEM;
    }

    TS_MSG_BUF_SCRATCHPAD_PTR_INIT(scratchpad_std, msg);

    scratchpad_std->scratchroom.size = sizeof(struct ts_msg_proc_scratchroom);
    scratchpad_std->scratchtype.id = TS_MSG_SCRATCHPAD_PROC;

    TS_MSG_PROC_SCRATCHPAD_PTR_INIT(scratchpad, msg);

    scratchpad->port_src = THINGSET_PORT_ID_INVALID;
    scratchpad->port_dest = THINGSET_PORT_ID_INVALID;
    scratchpad->ctx_uid = 0;
    scratchpad->import_export_subset = 0;
    scratchpad->response_size_hint = 0;

    return 0;
}
