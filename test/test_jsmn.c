/*
 * Copyright (c) 2021 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../src/ts_jsmn.h"

#include "test.h"

#define TEST_JSMN_NUM_TOKEN 100

/**
 * @brief Test JSMN JSON parser
 *
 * This test verifies the JSMN JSON parser usage:
 * - ts_jsmn_init()
 * - ts_jsmn_parse()
 * - ts_jsmn_token_by_index()
 * - ts_jsmn_dump()
 *
 */
void test_jsmn(void)
{
    int ret;
    char test_dump[1000];
    uint8_t test_jsmn_context_buffer[sizeof(struct ts_jsmn_context)
                                     + (TEST_JSMN_NUM_TOKEN * sizeof(struct ts_jsmn_token))];
    const char *test_js = "[1,2,3,{\"a\":\"b\"},[4.56,\"cd\",[7.8,-9e10]]],{\"f\":12}";
    const char *test_js_expect =
"#00: ARRAY    5 '[1,2,3,{\"a\":\"b\"},[4.56,\"cd\",[7.8,-9e10]]]'\n"
"#01: PRIMITIV 0 '1'\n"
"#02: PRIMITIV 0 '2'\n"
"#03: PRIMITIV 0 '3'\n"
"#04: OBJECT   1 '{\"a\":\"b\"}'\n"
"#05: STRING   1 'a'\n"
"#06: STRING   0 'b'\n"
"#07: ARRAY    3 '[4.56,\"cd\",[7.8,-9e10]]'\n"
"#08: PRIMITIV 0 '4.56'\n"
"#09: STRING   0 'cd'\n"
"#10: ARRAY    2 '[7.8,-9e10]'\n"
"#11: PRIMITIV 0 '7.8'\n"
"#12: PRIMITIV 0 '-9e10'\n"
"#13: OBJECT   1 '{\"f\":12}'\n"
"#14: STRING   1 'f'\n"
"#15: PRIMITIV 0 '12'\n";

    struct ts_jsmn_context *test_jsmn_ctx = TS_JSMN_CONTEXT(test_jsmn_context_buffer);
    uint16_t test_jsmn_num_tokens = TS_JSMN_CONTEXT_NUM_TOKEN(test_jsmn_context_buffer);
    uint16_t test_js_len = strlen(test_js);

    TEST_ASSERT_EQUAL_size_t(4, sizeof(struct ts_jsmn_token));
    TEST_ASSERT_EQUAL_UINT16(TEST_JSMN_NUM_TOKEN, test_jsmn_num_tokens);
    TEST_ASSERT_EQUAL_UINT16(50, test_js_len);

    ret = ts_jsmn_init(test_jsmn_ctx, test_jsmn_num_tokens);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_jsmn_token_count(test_jsmn_ctx);
    TEST_ASSERT_EQUAL(0, ret); /* number of tokens */

    ret = ts_jsmn_parse(test_jsmn_ctx, test_js, test_js_len);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_jsmn_token_count(test_jsmn_ctx);
    TEST_ASSERT_EQUAL(16, ret); /* number of tokens */

    for(uint16_t i = 0; i < test_jsmn_ctx->num_tokens; i++) {
        uint16_t tok_type;
        uint16_t tok_size;
        const char *tok_start;
        uint16_t tok_len;
        int ret = ts_jsmn_token_by_index(test_jsmn_ctx, i, &tok_type, &tok_size, &tok_start,
                                         &tok_len);
        if (ret != 0) {
            TEST_ASSERT_GREATER_OR_EQUAL_UINT16(16, i);
        }
        else {
            TEST_ASSERT_LESS_THAN_UINT16(16, i);
            TEST_ASSERT_NOT_NULL(tok_start);
            TEST_ASSERT_LESS_THAN_UINT64((uintptr_t)test_js + test_js_len, (uintptr_t)tok_start);
            TEST_ASSERT_GREATER_OR_EQUAL_UINT64((uintptr_t)test_js, (uintptr_t)tok_start);
            TEST_ASSERT_NOT_EQUAL_UINT16(0, tok_len);
            TEST_ASSERT_LESS_THAN_UINT16(test_js_len, tok_len);
        }
    }

    ts_jsmn_dump(test_jsmn_ctx, &test_dump[0], sizeof(test_dump));
    TEST_ASSERT_EQUAL_STRING(test_js_expect, &test_dump[0]);
}

void tests_jsmn(void)
{
    UNITY_BEGIN();

    //  Operating system abstraction
    RUN_TEST(test_jsmn);

    UNITY_END();
}
