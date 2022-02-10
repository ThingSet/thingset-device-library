/*
 * Copyright (c) 2010 Serge Zaitsev
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * @brief ThingSet JSMN (private interface)
 */

#ifndef TS_JSMN_H_
#define TS_JSMN_H_

/**
 * @brief ThingSet JSMN JSON parser.
 *
 * Copied from https://github.com/zserge/jsmn.
 *
 * Reduced storage need for tokens by the following restrictions:
 * - The total length of the JSON data string is limited to a maximum of 2047 characters.
 * - start: The start position of a token in the JSON data string is restricted to 0..2046
 * - end: The end position of a token must be withing 2047 bytes following the start position.
 * - size: The number of tokens of a super token (eg. elements in an array) is restricted to 0..127.
 *
 * @defgroup ts_jsmn_api_priv ThingSet JSMN (private interface)
 * @{
 */

#include "thingset_env.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum position of token in JSON data string */
#define TS_JSMN_TOKEN_POS_MAX 0x07FFU

#define TS_JSMN_TOKEN_POS_MAX_BITS (11)

/** @brief Maximum length of token in JSON data string */
#define TS_JSMN_TOKEN_LEN_MAX 0x07FFU

#define TS_JSMN_TOKEN_LEN_MAX_BITS (11)

/** @brief Maximum muber of tokens of a super token */
#define TS_JSMN_TOKEN_SIZE_MAX 0x007FU

#define TS_JSMN_TOKEN_SIZE_MAX_BITS (7)

/** @brief Maximum muber of tokens of a super token */
#define TS_JSMN_TOKEN_TYPE_MAX 0x0007U

#define TS_JSMN_TOKEN_TYPE_MAX_BITS (3)

/** @brief Maximum total length of JSON data string */
#define TS_JSMN_JSON_LEN_MAX TS_JSMN_TOKEN_POS_MAX


/** @brief JSON undefined type identifier. */
#define TS_JSMN_UNDEFINED   (0)

/** @brief JSON object type identifier. */
#define TS_JSMN_OBJECT      (1)

/** @brief JSON array type identifier. */
#define TS_JSMN_ARRAY       (2)

/** @brief JSON string type identifier. */
#define TS_JSMN_STRING      (3)

/**
 * @brief JSON primitive type identifier.
 *
 * Number, boolean (true/false) or null.
 */
#define TS_JSMN_PRIMITIVE   (4)

enum ts_jsmn_error {
    /* Not enough tokens were provided */
    TS_JSMN_ERROR_NOMEM = -ENOMEM,
    /* Invalid character inside JSON string */
    TS_JSMN_ERROR_INVAL = -EINVAL,
    /* The string is not a full JSON packet, more bytes expected */
    TS_JSMN_ERROR_PART = -ENAVAIL
};

/**
 * @brief JSON token descriptor.
 */
struct ts_jsmn_token {
    /** @brief Type of token */
    uint32_t type : TS_JSMN_TOKEN_TYPE_MAX_BITS;
    /** @brief Start position in JSON data string */
    uint32_t pos : TS_JSMN_TOKEN_POS_MAX_BITS;
    /** @brief Size of token (aka. number of sub-tokens) */
    uint32_t size : TS_JSMN_TOKEN_SIZE_MAX_BITS;
    /** @brief Length of token in JSON data string */
    uint32_t len : TS_JSMN_TOKEN_LEN_MAX_BITS;
#ifdef JSMN_PARENT_LINKS
    int16_t parent;
#endif
};

/**
 * @brief JSON parser context.
 *
 * Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
struct ts_jsmn_context {
    /** @brief Scratchpad memory. */
    union {
        struct {
            /** @brief Offset in the JSON data string. */
            uint16_t pos;
            /** @brief Next token to allocate. */
            uint16_t toknext;
            /** @brief Superior token node, e.g. parent object or array. */
            int16_t toksuper;
        } parser;
        struct {
            /** @brief Pointer to JSON data string that was parsed. */
            const char *js;
            /** @brief Number of tokens that were parsed. */
            uint16_t token_count;
        } result;
    } scratchpad;
    /** @brief Total number of tokens that can be used by the parser. */
    uint16_t num_tokens;
    /** @brief Array of tokens. */
    struct ts_jsmn_token tokens[0];
};

#define TS_JSMN_CONTEXT_NUM_TOKEN_FROM_SIZE(size)  \
    ((size - sizeof(struct ts_jsmn_context)) / sizeof(struct ts_jsmn_token))

#define TS_JSMN_CONTEXT_NUM_TOKEN(buffer_name) \
    TS_JSMN_CONTEXT_NUM_TOKEN_FROM_SIZE(sizeof(buffer_name))

#define TS_JSMN_CONTEXT(buffer_name) (struct ts_jsmn_context *)&buffer_name[0]

/**
 * @brief Create JSON parser over an array of tokens
 */
int ts_jsmn_init(struct ts_jsmn_context *jsmn, uint16_t num_tokens);

/**
 * @brief Run JSON parser.
 *
 * Parse a JSON data string into an array of tokens, each describing a single JSON object.
 *
 * @param[in] jsmn Parser context.
 * @param[in] js Pointer to JSON data string.
 * @param[in] len Length of JSON data string.
 * @returns 0 on success, <0 otherwise
 */
int ts_jsmn_parse(struct ts_jsmn_context *jsmn, const char *js, uint16_t len);

/**
 * @brief Number of parsed tokens.
 *
 * @param[in] jsmn Parser context.
 * @returns Number of parsed tokens.
 */
uint16_t ts_jsmn_token_count(struct ts_jsmn_context *jsmn);

/**
 * @brief Start of token text.
 *
 * @param[in] jsmn Parser context.
 * @param[in] token_idx Token index.
 * @returns Pointer to token JSON text..
 */
const char *ts_jsmn_token_start(struct ts_jsmn_context *jsmn, uint16_t token_idx);

/**
 * @brief Get info of token with given index in token array.
 *
 * @param[in] jsmn Parser context.
 * @param[in] token_idx Token index.
 * @param[out] type Pointer to write type info to.
 * @param[out] size Pointer to write size info to.
 * @param[out] start Pointer to write token text start position to.
 * @param[out] len Pointer to write token text length info to.
 * @returns 0 on success, <0 otherwise.
 */
int ts_jsmn_token_by_index(struct ts_jsmn_context *jsmn, uint16_t token_idx,
                           uint16_t *type, uint16_t *size,  const char **start, uint16_t *len);

/**
 * @brief Dump parser token array in human readable form.
 *
 * @param[in] jsmn Parser context.
 * @param[in] dump Ponter to dump buffer.
 * @param[in] len Length of dump buffer.
 */
void ts_jsmn_dump(struct ts_jsmn_context *jsmn, char *dump, size_t len);

#ifdef __cplusplus
}
#endif

/**
 * @} <!-- ts_jsmn_api_priv -->
 */

#endif /* TS_JSMN_H_ */
