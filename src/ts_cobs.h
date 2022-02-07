/*
 * Copyright (c) 2021 Charles Nicholson, charles.nicholson@gmail.com
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Unlicense
 */

/**
 * @file
 * @brief ThingSet COBS (private interface)
 */

#ifndef TS_COBS_H_
#define TS_COBS_H_

/**
 * @brief ThingSet COBS encoder/ decoder.
 *
 * Copied from https://github.com/charlesnicholson/nanocobs.
 *
 * Implementation of the [Consistent Overhead Byte Stuffing]
 * (https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) ("COBS")
 * algorithm, defined in the [paper](http://www.stuartcheshire.org/papers/COBSforToN.pdf) by
 * Stuart Cheshire and Mary Baker.
 *
 * Users can encode and decode data in-place or into separate target buffers. Encoding can be
 * incremental; users can encode multiple small buffers (e.g. header, then payloads) into one target.
 * The `cobs` runtime requires no extra memory overhead.
 *
 * @defgroup ts_cobs_api_priv ThingSet COBS (private interface)
 * @{
 */

#include "thingset_env.h"

enum {
    /**
     * @brief COBS frame delimiter.
     *
     * All COBS frames end with this value. If you're scanning a data source
     * for frame delimiters, the presence of this zero byte indicates the
     * completion of a frame.
     */
    TS_COBS_FRAME_DELIMITER = 0x00,

    /** @brief In-place encoding mandatory placeholder byte values. */
    TS_COBS_INPLACE_SENTINEL_VALUE = 0x5A,

    /** @brief In-place encodings that fit in a buffer of this size will always succeed. */
    TS_COBS_INPLACE_SAFE_BUFFER_SIZE = 256
};

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @def TS_COBS_ENCODE_MAX
 *
 * @brief Maximum possible size of buffer required to encode content.
 *
 * Returns the maximum possible size in bytes of the buffer required to encode
 * a buffer of length @p decoded_len. Cannot fail. Defined as a macro to facilitate
 * compile-time sizing of buffers.
 *
 * @note @p decoded_len is evaluated multiple times; don't call with mutating
 * expressions! e.g. Don't do "TS_COBS_ENCODE_MAX(i++)".
 *
 * @param[in] decoded_len Length of decoded content
 * @return The maximum possible size in bytes of the buffer required to encode
 *         a buffer of length @p decoded_len.
 */
#define TS_COBS_ENCODE_MAX(decoded_len) \
        (1 + (decoded_len) + (((decoded_len) + 253) / 254) + ((decoded_len) == 0))

/**
 * @brief Decode in-place the contents of the provided buffer.
 *
 * Decode in-place the contents of the provided buffer @p buf of length @p len.
 *
 * Because decoding is in-place, the first and last bytes of @p buf will be set
 * to the value TS_COBS_INPLACE_SENTINEL_VALUE if decoding succeeds. The decoded
 * contents are stored in the inclusive span defined by buf[1] and buf[len-2].
 *
 * @warning If the function returns -EBADMSG, the contents of @p buf are
 *          left indeterminate and must not be relied on to be fully encoded or decoded.
 *
 * @param[in] len Length of encoded content.
 * @param[in,out] buf Pointer to buffer.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if a null pointer or invalid length are provided.
 * @retval -EBADMSG if the encoded buffer contains any code bytes that exceed len or if the buffer
 *         starts with a 0 byte, or ends in a nonzero byte.
 */
int ts_cobs_decode_inplace(unsigned len, void *buf);

/**
 * @brief Encode in-place the contents of the provided buffer.
 *
 * Encode in-place the contents of the provided buffer @p buf of length @p len.
 *
 * Because encoding adds leading and trailing bytes, your buffer must reserve
 * bytes 0 and len-1 for the encoding. If the first and last bytes of @p buf
 * are not set to TS_COBS_INPLACE_SENTINEL_VALUE, the function will fail with
 * -EBADMSG.
 *
 * If @p len is less than or equal to TS_COBS_INPLACE_SAFE_BUFFER_SIZE, the
 * contents of @p buf will never cause encoding to fail. If @p len is larger
 * than TS_COBS_INPLACE_SAFE_BUFFER_SIZE, encoding can possibly fail with
 * -EBADMSG if there are more than 254 bytes between zeros.
 *
 * @warning If the function returns -EBADMSG, the contents of @p buf are
 *          left indeterminate and must not be relied on to be fully encoded or decoded.
 *
 * @param[in] len Length of encoded content including sentinel value at bytes 0 and len - 1.
 * @param[in,out] buf Pointer to buffer.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if a null pointer or invalid length are provided.
 * @retval -EBADMSG if the first and last bytes of buf are not set to TS_COBS_INPLACE_SENTINEL_VALUE
 *         or if len is larger than TS_COBS_INPLACE_SAFE_BUFFER_SIZE and there are more than
 *         254 bytes between zeros.
 */
int ts_cobs_encode_inplace(unsigned int len, void *buf);

/**
 * @brief Decode encoded bytes.
 *
 * Decode @p enc_len encoded bytes from @p enc into @p out_dec, storing the decoded
 * length in @p out_dec_len.
 *
 * @param[in] enc_len Length of encoded content.
 * @param[in] enc Pointer to buffer of encoded content.
 * @param[in] dec_max Maximum length for decoded content.
 * @param[out] out_dec_len Pointer to store the length of the decoded content.
 * @param[out] out_dec Pointer to buffer to store the decoded content.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if any of the input pointers are null, or if any of the lengths are invalid.
 * @retval -EBADMSG if enc starts with a 0 byte, or does not end with a 0 byte.
 * @retval -ENOMEM if the decoding exceeds dec_max bytes.
 */
int ts_cobs_decode(unsigned int enc_len, void const *enc, unsigned int dec_max,
                   unsigned int *out_dec_len, void *out_dec);


/**
 * @brief Encode decoded bytes.
 *
 * Encode @p dec_len decoded bytes from @p dec into @p out_enc, storing the encoded
 * length in @p out_enc_len.
 *
 * @param[in] dec_len Length of decoded content.
 * @param[in] dec Pointer to buffer of decoded content.
 * @param[in] enc_max Maximum length for encoded content.
 * @param[out] out_enc_len Pointer to store the length of the encoded content.
 * @param[out] out_enc Pointer to buffer to store the encoded content.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if any of the input pointers are null, or if any of the lengths are invalid.
 * @retval -EBADMSG if enc starts with a 0 byte, or does not end with a 0 byte.
 * @retval -ENOMEM if the encoding exceeds enc_max bytes.
 */
int ts_cobs_encode(unsigned int dec_len, void const *dec, unsigned int enc_max,
                   unsigned int *out_enc_len, void *out_enc);


/* Incremental encoding API */

/** @brief Intermediate encoding state for in   cremental encoding. */
struct ts_cobs_enc_ctx {
  void *dst;
  unsigned dst_max;
  unsigned cur;
  unsigned code_idx;
  unsigned code;
  int need_advance;
};


/**
 * @brief Begin an incremental encoding of data.
 *
 * Begin an incremental encoding of data into @p out_enc. The intermediate encoding state is stored
 * in @p ctx, which can then be passed into future calls to ts_cobs_encode_inc().
 *
 * @warning If the function returns an error, @p ctx shall not be used in future calls to
 *          ts_cobs_encode_inc().
 *
 * @param[in] ctx Pointer to intermediate encoding state.
 * @param[in] enc_max Maximum length for encoded content.
 * @param[in] out_enc Pointer to buffer to store the encoded content.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if out_enc or out_ctx are null, or if enc_max is not large enough to
 *                 hold the smallest possible encoding,
 */
int ts_cobs_encode_inc_begin(struct ts_cobs_enc_ctx *ctx, unsigned int enc_max, void *out_enc);


/**
 * @brief Continue an encoding in progress with the new buffer.
 *
 * Continue an encoding in progress with the new @p dec buffer of length @p dec_len. Encodes
 * @p dec_len decoded bytes from @p dec into the buffer that @p ctx was initialized with in
 * ts_cobs_encode_inc_begin().
 *
 * If the contents pointed to by @p dec can not be encoded in the remaining available buffer space,
 * the function returns -ENOMEM. In this case, @p ctx remains unchanged and incremental encoding can
 * be attempted again with different data, or finished with ts_cobs_encode_inc_end().
 *
 * @param[in] ctx Pointer to intermediate encoding state.
 * @param[in] dec_len Length of decoded content.
 * @param[in] dec Pointer to buffer of decoded content.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if any of the input pointers are null, or dec_len is zero.
 * @retval -ENOMEM if the contents pointed to by dec can not be encoded in the remaining
 *        available buffer space.
 */
int ts_cobs_encode_inc(struct ts_cobs_enc_ctx *ctx, unsigned int dec_len, void const *dec);


/**
 * @brief Finish an incremental encoding.
 *
 * Finish an incremental encoding by writing the final code and delimiter.
 *
 * The final encoded length is written to @p out_enc_len, and the buffer
 * passed to ts_cobs_encode_inc_begin() holds the full COBS-encoded frame.
 *
 * @warning No further calls to ts_cobs_encode_inc() or ts_cobs_encode_inc_end() can be safely made
 *          until @p ctx is re-initialized via a new call to ts_cobs_encode_inc_begin().
 *
 * @param[in] ctx Pointer to intermediate encoding state.
 * @param[out] out_enc_len Pointer to store the length of the encoded content.
 * @return 0 on success, <0 on error.
 * @retval -EINVAL if any of the input pointers are null.
 */
int ts_cobs_encode_inc_end(struct ts_cobs_enc_ctx *ctx, unsigned int *out_enc_len);


#ifdef __cplusplus
}
#endif

/**
 * @} <!-- ts_cobs_api_priv -->
 */

#endif /* TS_COBS_H_ */
