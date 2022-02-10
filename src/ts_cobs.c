/*
 * Copyright (c) 2021 Charles Nicholson, charles.nicholson@gmail.com
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Unlicense
 *
 * Copied from https: https://github.com/charlesnicholson/nanocobs.
 */

/**
 * @file
 * @brief ThingSet COBS
 */

#include "ts_cobs.h"

#define COBS_ISV TS_COBS_INPLACE_SENTINEL_VALUE

typedef unsigned char cobs_byte_t;



int ts_cobs_decode_inplace(unsigned len, void *buf)
{
    if (!buf || (len < 2)) {
        return -EINVAL;
    }

    cobs_byte_t *const src = (cobs_byte_t *)buf;
    unsigned ofs, cur = 0;
    while ((ofs = src[cur]) != TS_COBS_FRAME_DELIMITER) {
        src[cur] = 0;
        cur += ofs;
        if (cur > len) {
            return -EBADMSG;
        }
    }

    if (cur != len - 1) {
        return -EBADMSG;
    }
    src[0] = COBS_ISV;
    src[len - 1] = COBS_ISV;
    return 0;
}

int ts_cobs_encode_inplace(unsigned int len, void *buf)
{
    if (!buf || (len < 2)) {
        return -EINVAL;
    }

    cobs_byte_t *const src = (cobs_byte_t *)buf;
    if ((src[0] != COBS_ISV) || (src[len - 1] != COBS_ISV)) {
        return -EBADMSG;
    }

    unsigned patch = 0, cur = 1;
    while (cur < len - 1) {
        if (src[cur] == TS_COBS_FRAME_DELIMITER) {
            unsigned const ofs = cur - patch;
            if (ofs > 255) {
                return -EBADMSG;
            }
            src[patch] = (cobs_byte_t)ofs;
            patch = cur;
        }
        ++cur;
    }
    unsigned const ofs = cur - patch;
    if (ofs > 255) {
        return -EBADMSG;
    }
    src[patch] = (cobs_byte_t)ofs;
    src[cur] = 0;
    return 0;
}

int ts_cobs_decode(unsigned int enc_len, void const *enc, unsigned int dec_max,
                   unsigned int *out_dec_len, void *out_dec)
{
    if (!enc || !out_dec || !out_dec_len) {
        return -EINVAL;
    }
    if (enc_len < 2) {
        return -EINVAL;
    }

    cobs_byte_t const *const src = (cobs_byte_t const *)enc;
    cobs_byte_t *const dst = (cobs_byte_t *)out_dec;

    if ((src[0] == TS_COBS_FRAME_DELIMITER) || (src[enc_len - 1] != TS_COBS_FRAME_DELIMITER)) {
        return -EBADMSG;
    }

    unsigned int src_idx = 0, dst_idx = 0;

    while (src_idx < (enc_len - 1)) {
        unsigned int const code = src[src_idx++];
        if (!code) {
            return -EBADMSG;
        }
        if ((src_idx + code) > enc_len) {
            return -EBADMSG;
        }

        if ((dst_idx + code - 1) > dec_max) {
            return -ENOMEM;
        }
        for (unsigned i = 0; i < code - 1; ++i) {
            dst[dst_idx++] = src[src_idx++];
        }

        if ((src_idx < (enc_len - 1)) && (code < 0xFF)) {
            if (dst_idx >= dec_max) {
                return -ENOMEM;
            }
            dst[dst_idx++] = 0;
        }
    }

    *out_dec_len = dst_idx;
    return 0;
}

int ts_cobs_encode(unsigned int dec_len, void const *dec, unsigned int enc_max,
                   unsigned int *out_enc_len, void *out_enc)
{
    if (!out_enc_len) {
        return -EINVAL;
    }

    struct ts_cobs_enc_ctx ctx;
    int r;
    r = ts_cobs_encode_inc_begin(out_enc, enc_max, &ctx);
    if (r != 0) {
        return r;
    }
    r = ts_cobs_encode_inc(&ctx, dec_len, dec);
    if (r != 0) {
        return r;
    }
    r = ts_cobs_encode_inc_end(&ctx, out_enc_len);
    return r;
}


int ts_cobs_encode_inc_begin(struct ts_cobs_enc_ctx *ctx, unsigned int enc_max, void *out_enc)
{
    if (!out_enc || !ctx) {
        return -EINVAL;
    }
    if (enc_max < 2) {
        return -EINVAL;
    }

    ctx->dst = out_enc;
    ctx->dst_max = enc_max;
    ctx->cur = 1;
    ctx->code = 1;
    ctx->code_idx = 0;
    ctx->need_advance = 0;
    return 0;
}

int ts_cobs_encode_inc(struct ts_cobs_enc_ctx *ctx, unsigned int dec_len, void const *dec)
{
    if (!ctx || !dec) {
        return -EINVAL;
    }
    unsigned dst_idx = ctx->cur;
    unsigned const enc_max = ctx->dst_max;
    if ((enc_max - dst_idx) < dec_len) {
        return -ENOMEM;
    }

    unsigned dst_code_idx = ctx->code_idx;
    unsigned code = ctx->code;
    int need_advance = ctx->need_advance;

    cobs_byte_t const *const src = (cobs_byte_t const *)dec;
    cobs_byte_t *const dst = (cobs_byte_t *)ctx->dst;
    unsigned src_idx = 0;

    if (need_advance) {
        if (++dst_idx >= enc_max) {
            return -ENOMEM;
        }
        need_advance = 0;
    }

    while (dec_len--) {
        cobs_byte_t const byte = src[src_idx];
        if (byte) {
            dst[dst_idx] = byte;
            if (++dst_idx >= enc_max) {
                return -ENOMEM;
            }
            ++code;
        }

        if ((byte == 0) || (code == 0xFF)) {
            dst[dst_code_idx] = (cobs_byte_t)code;
            dst_code_idx = dst_idx;
            code = 1;

            if ((byte == 0) || dec_len) {
                if (++dst_idx >= enc_max) {
                    return -ENOMEM;
                }
            } else {
                need_advance = !dec_len;
            }
        }
        ++src_idx;
    }

    ctx->cur = dst_idx;
    ctx->code = code;
    ctx->code_idx = dst_code_idx;
    ctx->need_advance = need_advance;
    return 0;
}


int ts_cobs_encode_inc_end(struct ts_cobs_enc_ctx *ctx, unsigned int *out_enc_len)
{
    if (!ctx || !out_enc_len) {
        return -EINVAL;
    }

    cobs_byte_t *const dst = (cobs_byte_t *)ctx->dst;
    unsigned cur = ctx->cur;
    dst[ctx->code_idx] = (cobs_byte_t)ctx->code;
    dst[cur++] = TS_COBS_FRAME_DELIMITER;
    *out_enc_len = cur;
    return 0;
}
