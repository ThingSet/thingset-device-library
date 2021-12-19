/*
 * Copyright (c) 2010 Serge Zaitsev
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: MIT
 *
 * Copied from https://github.com/zserge/jsmn.
 */

/**
 * @file
 * @brief ThingSet JSMN
 */

#include "ts_jsmn.h"
#include "ts_macro.h"

static inline uint16_t token_pos(struct ts_jsmn_token *token)
{
    return (uint16_t)(token->pos);
}


static inline uint16_t token_len(struct ts_jsmn_token *token)
{
    return (uint16_t)(token->len);
}

static inline uint16_t token_end(struct ts_jsmn_token *token)
{
    return (uint16_t)(token->pos + token->len);
}

static inline uint16_t token_type(struct ts_jsmn_token *token)
{
    return (uint16_t)(token->type);
}

static inline uint16_t token_size(struct ts_jsmn_token *token)
{
    return (uint16_t)(token->size);
}

static inline void token_set_pos(struct ts_jsmn_token *token, uint16_t pos)
{
    token->pos = pos;
}

static inline void token_set_len(struct ts_jsmn_token *token, uint16_t len)
{
    token->len = len;
}

static inline void token_set_end(struct ts_jsmn_token *token, uint16_t end)
{
    token->len = end - token->pos;
}

static inline void token_set_type(struct ts_jsmn_token *token, uint16_t type)
{
    token->type = type;
}

static inline void token_set_size(struct ts_jsmn_token *token, uint16_t size)
{
    token->size = size;
}

static inline bool token_has_start(struct ts_jsmn_token *token)
{
    return token->pos != TS_JSMN_TOKEN_POS_MAX;
}

static inline bool token_has_end(struct ts_jsmn_token *token)
{
    return token->len > 0;
}

static inline bool token_is_type(struct ts_jsmn_token *token, uint16_t type)
{
    return token->type == type;
}

static inline bool token_has_size(struct ts_jsmn_token *token)
{
    return token->size > 0;
}

static inline void token_increment_size(struct ts_jsmn_token *token)
{
    token->size += 1;
}

static bool token_is_valid(struct ts_jsmn_token *token)
{
    if (token == NULL) {
        return false;
    }
    uint16_t val;

    val = token_type(token);
    if ((val != TS_JSMN_OBJECT) && (val != TS_JSMN_ARRAY) && (val != TS_JSMN_STRING)
        && (val != TS_JSMN_PRIMITIVE)) {
        return false;
    }
    if (!token_has_start(token) || !token_has_end(token)) {
        return false;
    }
    return true;
}

/**
 * @brief Create new token.
 *
 * Allocate a fresh unused token from the token pool and fill token type and boundaries.
 */
static int new_token(struct ts_jsmn_context *jsmn, uint16_t type, uint16_t start, uint16_t end,
                             struct ts_jsmn_token **token)
{
    /* Don't allow maximum for start position - regarded invalid */
    //TS_ASSERT(TS_JSMN_TOKEN_POS_MAX > start, "start (%u) >= TS_JSMN_TOKEN_POS_MAX (%u)",
    //          (unsigned int)start, (unsigned int)TS_JSMN_TOKEN_POS_MAX);
    //TS_ASSERT(TS_JSMN_TOKEN_POS_MAX >= end, "end (%u) > TS_JSMN_TOKEN_POS_MAX (%u)",
    //          (unsigned int)end, (unsigned int)TS_JSMN_TOKEN_POS_MAX);

    if (jsmn->scratchpad.parser.toknext >= jsmn->num_tokens) {
        return TS_JSMN_ERROR_NOMEM;
    }

    /* Allocate a fresh unused token from the token pool. */
    struct ts_jsmn_token *tok = &jsmn->tokens[jsmn->scratchpad.parser.toknext++];

    /* Fill token type and boundaries */
    token_set_pos(tok, start);
    if (end > start) {
        //TS_ASSERT(TS_JSMN_TOKEN_LEN_MAX >= (end - start), "len (%u) >= TS_JSMN_TOKEN_LEN_MAX (%u)",
        //      (unsigned int)(end - start), (unsigned int)TS_JSMN_TOKEN_LEN_MAX);
        token_set_end(tok, end);
    } else {
        token_set_len(tok, 0);
    }
    token_set_type(tok, type);
    token_set_size(tok, 0);
#ifdef JSMN_PARENT_LINKS
    tok->parent = jsmn->scratch.parser.toksuper;
#endif

    /* return token */
    *token = tok;
    return 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int parse_primitive(struct ts_jsmn_context *jsmn, const char *js, const size_t len)
{
    int ret;
    struct ts_jsmn_token *token;
    int start = jsmn->scratchpad.parser.pos;

    for (; jsmn->scratchpad.parser.pos < len && js[jsmn->scratchpad.parser.pos] != '\0'; jsmn->scratchpad.parser.pos++) {
        switch (js[jsmn->scratchpad.parser.pos]) {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
            break;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (js[jsmn->scratchpad.parser.pos] < 32 || js[jsmn->scratchpad.parser.pos] >= 127) {
            jsmn->scratchpad.parser.pos = start;
            return TS_JSMN_ERROR_INVAL;
        }
    }

#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    jsmn->scratch.parser.pos = start;
    return TS_JSMN_ERROR_PART;
#endif

found:
    ret = new_token(jsmn, TS_JSMN_PRIMITIVE, start, jsmn->scratchpad.parser.pos, &token);
    if (ret != 0) {
        jsmn->scratchpad.parser.pos = start;
    } else {
        jsmn->scratchpad.parser.pos--;
    }
    return ret;
}

/**
 * Fills next token with JSON string.
 */
static int parse_string(struct ts_jsmn_context *jsmn, const char *js, const size_t len)
{
    int start = jsmn->scratchpad.parser.pos;

    jsmn->scratchpad.parser.pos++;

    /* Skip starting quote */
    for (; jsmn->scratchpad.parser.pos < len && js[jsmn->scratchpad.parser.pos] != '\0'; jsmn->scratchpad.parser.pos++) {
        char c = js[jsmn->scratchpad.parser.pos];

        /* Quote: end of string */
        if (c == '\"') {
            struct ts_jsmn_token *token;
            int ret = new_token(jsmn, TS_JSMN_STRING, start + 1, jsmn->scratchpad.parser.pos, &token);
            if (ret != 0) {
                jsmn->scratchpad.parser.pos = start;
            }
            return ret;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && jsmn->scratchpad.parser.pos + 1 < len) {
            int i;
            jsmn->scratchpad.parser.pos++;
            switch (js[jsmn->scratchpad.parser.pos]) {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                jsmn->scratchpad.parser.pos++;
                for (i = 0; i < 4 && jsmn->scratchpad.parser.pos < len && js[jsmn->scratchpad.parser.pos] != '\0'; i++) {
                    /* If it isn't a hex character we have an error */
                    if (!((js[jsmn->scratchpad.parser.pos] >= 48 && js[jsmn->scratchpad.parser.pos] <= 57) ||   /* 0-9 */
                          (js[jsmn->scratchpad.parser.pos] >= 65 && js[jsmn->scratchpad.parser.pos] <= 70) ||   /* A-F */
                          (js[jsmn->scratchpad.parser.pos] >= 97 && js[jsmn->scratchpad.parser.pos] <= 102))) { /* a-f */
                        jsmn->scratchpad.parser.pos = start;
                        return TS_JSMN_ERROR_INVAL;
                    }
                    jsmn->scratchpad.parser.pos++;
                }
                jsmn->scratchpad.parser.pos--;
                break;
            /* Unexpected symbol */
            default:
                jsmn->scratchpad.parser.pos = start;
                return TS_JSMN_ERROR_INVAL;
            }
        }
    }
    jsmn->scratchpad.parser.pos = start;
    return TS_JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int ts_jsmn_parse(struct ts_jsmn_context *jsmn, const char *js, uint16_t len)
{
    int ret = 0;
    int count = 0;

    if (len >= TS_JSMN_JSON_LEN_MAX) {
        ret = TS_JSMN_ERROR_NOMEM;
        goto ts_jsmn_parse_end;
    }

    /* prepare parser */
    jsmn->scratchpad.parser.pos = 0;
    jsmn->scratchpad.parser.toknext = 0;
    jsmn->scratchpad.parser.toksuper = -1;

    int i;
    struct ts_jsmn_token *token;

    for (; jsmn->scratchpad.parser.pos < len && js[jsmn->scratchpad.parser.pos] != '\0'; jsmn->scratchpad.parser.pos++) {
        char c;
        uint16_t type;

        c = js[jsmn->scratchpad.parser.pos];
        switch (c) {
        case '{':
        case '[':
            type = (c == '{' ? TS_JSMN_OBJECT : TS_JSMN_ARRAY);
            count++;
            /** @todo compare with original jsmn - validity of token->parent */
            ret = new_token(jsmn, type, jsmn->scratchpad.parser.pos, 0, &token);
            if (ret != 0) {
                goto ts_jsmn_parse_end;
            }
            if (jsmn->scratchpad.parser.toksuper != -1) {
                struct ts_jsmn_token *tok = &jsmn->tokens[jsmn->scratchpad.parser.toksuper];
#ifdef JSMN_STRICT
                // In strict mode an object or array can't become a key
                if (token_is_type(tok, TS_JSMN_OBJECT)) {
                    ret = TS_JSMN_ERROR_INVAL;
                    goto ts_jsmn_parse_end;
                }
#endif
                token_increment_size(tok);
            }
            jsmn->scratchpad.parser.toksuper = jsmn->scratchpad.parser.toknext - 1;
            break;
        case '}':
        case ']':
            type = (c == '}' ? TS_JSMN_OBJECT : TS_JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (jsmn->scratch.parser.toknext < 1) {
                ret = TS_JSMN_ERROR_INVAL;
                goto ts_jsmn_parse_end;
            }
            token = &jsmn->tokens[jsmn->scratch.parser.toknext - 1];
            for (;;) {
                if (token_has_start(token) && !token_has_end(token)) {
                    if (!token_is_type(token, type)) {
                        ret = TS_JSMN_ERROR_INVAL;
                        goto ts_jsmn_parse_end;
                    }
                    token_set_end(jsmn->scratch.parser.pos + 1);
                    jsmn->scratch.parser.toksuper = token->parent;
                    break;
                }
                if (token->parent == -1) {
                    if (!token_is_type(token, type) || (jsmn->scratch.parser.toksuper == -1)) {
                        ret = TS_JSMN_ERROR_INVAL;
                        goto ts_jsmn_parse_end;
                    }
                    break;
                }
                token = &tokens[token->parent];
            }
#else
            for (i = jsmn->scratchpad.parser.toknext - 1; i >= 0; i--) {
                token = &jsmn->tokens[i];
                if (token_has_start(token) && !token_has_end(token)) {
                    if (!token_is_type(token, type)) {
                        ret = TS_JSMN_ERROR_INVAL;
                        goto ts_jsmn_parse_end;
                    }
                    jsmn->scratchpad.parser.toksuper = -1;
                    token_set_end(token, jsmn->scratchpad.parser.pos + 1);
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) {
                ret = TS_JSMN_ERROR_INVAL;
                goto ts_jsmn_parse_end;
            }
            for (; i >= 0; i--) {
                token = &jsmn->tokens[i];
                if (token_has_start(token) && !token_has_end(token)) {
                    jsmn->scratchpad.parser.toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            ret = parse_string(jsmn, js, len);
            if (ret != 0) {
                goto ts_jsmn_parse_end;
            }
            count++;
            if (jsmn->scratchpad.parser.toksuper != -1) {
                token_increment_size(&jsmn->tokens[jsmn->scratchpad.parser.toksuper]);
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            jsmn->scratchpad.parser.toksuper = jsmn->scratchpad.parser.toknext - 1;
            break;
        case ',':
            if (jsmn->scratchpad.parser.toksuper != -1 &&
                !token_is_type(&jsmn->tokens[jsmn->scratchpad.parser.toksuper], TS_JSMN_ARRAY) &&
                !token_is_type(&jsmn->tokens[jsmn->scratchpad.parser.toksuper], TS_JSMN_OBJECT)) {
#ifdef JSMN_PARENT_LINKS
                jsmn->scratch.parser.toksuper = jsmn->tokens[jsmn->scratch.parser.toksuper].parent;
#else
                for (i = jsmn->scratchpad.parser.toknext - 1; i >= 0; i--) {
                    if (token_is_type(&jsmn->tokens[i], TS_JSMN_ARRAY) ||
                        token_is_type(&jsmn->tokens[i], TS_JSMN_OBJECT)) {
                        if (token_has_start(&jsmn->tokens[i]) && !token_has_end(&jsmn->tokens[i])) {
                            jsmn->scratchpad.parser.toksuper = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (jsmn->scratch.parser.toksuper != -1) {
                const struct ts_jsmn_token *tok = &jsmn->tokens[jsmn->scratch.parser.toksuper];
                if (token_is_type(tok, TS_JSMN_OBJECT) ||
                    (token_is_type(tok, TS_JSMN_STRING) && token_has_size(tok))) {
                    ret = TS_JSMN_ERROR_INVAL;
                    goto ts_jsmn_parse_end;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            ret = parse_primitive(jsmn, js, len);
            if (ret != 0) {
                goto ts_jsmn_parse_end;
            }
            count++;
            if (jsmn->scratchpad.parser.toksuper != -1) {
                token_increment_size(&jsmn->tokens[jsmn->scratchpad.parser.toksuper]);
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            ret = TS_JSMN_ERROR_INVAL;
            goto ts_jsmn_parse_end;
#endif
        }
    }

    for (i = jsmn->scratchpad.parser.toknext - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        if (token_has_start(&jsmn->tokens[i]) && !token_has_end(&jsmn->tokens[i])) {
            ret = TS_JSMN_ERROR_PART;
            goto ts_jsmn_parse_end;
        }
    }

ts_jsmn_parse_end:
    if ((ret == 0) && (count > 0)) {
        jsmn->scratchpad.result.js = js;
        jsmn->scratchpad.result.token_count = count;
    } else {
        jsmn->scratchpad.result.js = 0;
        jsmn->scratchpad.result.token_count = 0;
    }
    return ret;
}

/**
 * @brief Initialise parser context.
 *
 * Create a new parser based over a given buffer with an array of tokens available.
 */
int ts_jsmn_init(struct ts_jsmn_context *jsmn, uint16_t num_tokens)
{
    /* Assert pointer alignment - align to 4 */
    TS_ASSERT(((uintptr_t)jsmn & 0x03) == 0, "JSM: %s context pointer %p not aligned", __func__,
              jsmn);

    jsmn->scratchpad.result.js = NULL;
    jsmn->scratchpad.result.token_count = 0;

    if (num_tokens == 0) {
        return TS_JSMN_ERROR_NOMEM;
    }
    jsmn->num_tokens = num_tokens;
    for (uint16_t i = 0; i < jsmn->num_tokens; i++) {
        /* set start to invalid */
        token_set_pos(&jsmn->tokens[i], TS_JSMN_TOKEN_POS_MAX);
    }

    return 0;
}

uint16_t ts_jsmn_token_count(struct ts_jsmn_context *jsmn)
{
    return jsmn->scratchpad.result.token_count;
}

const char *ts_jsmn_token_start(struct ts_jsmn_context *jsmn, uint16_t token_idx)
{
    return jsmn->scratchpad.result.js + token_pos(&jsmn->tokens[token_idx]);
}

int ts_jsmn_token_by_index(struct ts_jsmn_context *jsmn, uint16_t token_idx,
                           uint16_t *type, uint16_t *size,  const char **start, uint16_t *len)
{
    if (token_idx >= jsmn->num_tokens) {
        return TS_JSMN_ERROR_NOMEM;
    }
    if (!token_is_valid(&jsmn->tokens[token_idx])) {
        return TS_JSMN_ERROR_NOMEM;
    }
    *type = token_type(&jsmn->tokens[token_idx]);
    *size = token_size(&jsmn->tokens[token_idx]);
    *start = ts_jsmn_token_start(jsmn, token_idx);
    *len = token_len(&jsmn->tokens[token_idx]);

    return 0;
}

void ts_jsmn_dump(struct ts_jsmn_context *jsmn, char *dump, size_t len)
{
    TS_ASSERT(dump != NULL, "JSMN: %s on invalid dump buffer pointer (NULL)", __func__);
    TS_ASSERT(len > 0, "JSMN: %s on invalid dump buffer len (0)", __func__);

    uint16_t tok_type;
    uint16_t tok_size;
    const char *tok_start;
    uint16_t tok_len;

    size_t pos = 0;
    for(uint16_t i = 0; (i < jsmn->num_tokens) && (pos < len); i++) {
        if (ts_jsmn_token_by_index(jsmn, i, &tok_type, &tok_size, &tok_start, &tok_len) != 0) {
            break;
        }
        const char *type;
        if (tok_type == TS_JSMN_UNDEFINED) {
            type = "UNDEF   ";
        }
        else if (tok_type == TS_JSMN_OBJECT) {
            type = "OBJECT  ";
        }
        else if (tok_type == TS_JSMN_ARRAY) {
            type = "ARRAY   ";
        }
        else if (tok_type == TS_JSMN_STRING) {
            type = "STRING  ";
        }
        else if (tok_type == TS_JSMN_PRIMITIVE) {
            type = "PRIMITIV";
        }
        else {
            type = "unknown ";
        }
        pos += snprintf(&dump[pos], len - pos, "#%.*u: %s %u '%.*s'\n",
                        2, (unsigned int)i, type, (unsigned int)tok_size,
                        (int)tok_len, tok_start);
    }
    if (pos >= len) {
        pos = len - 1;
    }
    dump[pos] = '\0';
}
