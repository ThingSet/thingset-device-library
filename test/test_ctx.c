/*
 * Copyright (c) 2021 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/**
 * @brief Test communication context
 *
 * This test verifies communication context usage:
 * - THINGSET_COM_DEFINE()
 * - thingset_init()
 * - thingset_run()
 */
void test_ctx_com(void)
{
    int ret;

    /* Assure communication context is enabled for testing */
    TEST_ASSERT_TRUE(TS_CONFIG_COM);

    /* Assure "instance" test context static init */
    const struct ts_ctx *test_ts_instance = &TS_CAT(ts_ctx_, TEST_INSTANCE_LOCID);
    struct ts_ctx_data *test_ts_instance_data = &TS_CAT(ts_ctx_com_data_, TEST_INSTANCE_LOCID).common;
    const void *test_ts_instance_variant = &TS_CAT(ts_ctx_com_, TEST_INSTANCE_LOCID);

    TEST_ASSERT_EQUAL(1, TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL_UINT8(TS_CTX_TYPE_COM, test_ts_instance->ctx_type);
    TEST_ASSERT_EQUAL_UINT16(TEST_INSTANCE_LOCID, test_ts_instance->db_id);
    TEST_ASSERT_EQUAL_PTR(test_ts_instance_data, test_ts_instance->data);
    TEST_ASSERT_EQUAL_PTR(test_ts_instance_variant, test_ts_instance->variant);

    /* Assure specific context is correctly detected when used as generic context */
    TEST_ASSERT_TRUE(TS_CTX_IS_COM(TEST_INSTANCE_LOCID));

    /* Assure specific context is correctly accessed by generic context functions */
    thingset_authorisation_set(TEST_INSTANCE_LOCID, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, test_ts_instance_data->_auth_flags);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = thingset_init(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL(0, ret);

    ret = thingset_run(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL(0, ret);
}

/**
 * @brief Test communication context port table
 *
 * This test verifies port table usage:
 * - static initialisation by
 *   - THINGSET_APP_DEFINE
 *   - THINGSET_PORT_DEFINE
 */
void test_ctx_com_port(void)
{
    /* Assure communication context is enabled for testing */
    TEST_ASSERT_TRUE(TS_CONFIG_COM);

    /* Assure device port table static init */
    TEST_ASSERT_EQUAL(TEST_PORT_COUNT, TS_CONFIG_PORT_COUNT);

    /* port 0 */
    TEST_ASSERT_EQUAL(0, TEST_APP_INSTANCE_PORTID);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID),
                          ts_ports[TEST_APP_INSTANCE_PORTID]);
    TEST_ASSERT_EQUAL_UINT8(TEST_INSTANCE_LOCID, TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID).loc_id);
    TEST_ASSERT_EQUAL_STRING(TEST_APP_INSTANCE_NAME,
                             TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID).name);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_config_, TEST_APP_INSTANCE_PORTID),
                          TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID).config);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_data_, TEST_APP_INSTANCE_PORTID),
                          TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID).data);
    TEST_ASSERT_EQUAL_PTR(&ts_app_port_api, TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID).api);

    /* port 1 */
    TEST_ASSERT_EQUAL(1, TEST_APP_NEIGHBOUR_PORTID);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID),
                          ts_ports[TEST_APP_NEIGHBOUR_PORTID]);
    TEST_ASSERT_EQUAL_UINT8(TEST_NEIGHBOUR_LOCID,
                            TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID).loc_id);
    TEST_ASSERT_EQUAL_STRING(TEST_APP_NEIGHBOUR_NAME,
                             TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID).name);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_config_, TEST_APP_NEIGHBOUR_PORTID),
                          TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID).config);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_data_, TEST_APP_NEIGHBOUR_PORTID),
                          TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID).data);
    TEST_ASSERT_EQUAL_PTR(&ts_app_port_api, TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID).api);

    /* port 2 */
    TEST_ASSERT_EQUAL(2, TEST_SHELL_PORTID);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_, TEST_SHELL_PORTID), ts_ports[TEST_SHELL_PORTID]);
    TEST_ASSERT_EQUAL_UINT8(TEST_SHELL_LOCID, TS_CAT(ts_port_, TEST_SHELL_PORTID).loc_id);
    TEST_ASSERT_EQUAL_STRING(TEST_SHELL_NAME, TS_CAT(ts_port_, TEST_SHELL_PORTID).name);
    TEST_ASSERT_EQUAL_PTR(&ts_app_port_api, TS_CAT(ts_port_, TEST_SHELL_PORTID).api);


    /* port 3 */
    TEST_ASSERT_EQUAL(3, TEST_PORT_LOOPBACK_A_PORTID);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID),
                          ts_ports[TEST_PORT_LOOPBACK_A_PORTID]);
    TEST_ASSERT_EQUAL_UINT8(TEST_PORT_LOOPBACK_A_LOCID,
                            TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID).loc_id);
    TEST_ASSERT_EQUAL_STRING(TEST_PORT_LOOPBACK_A_NAME,
                             TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID).name);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_config_, TEST_PORT_LOOPBACK_A_PORTID),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID).config);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_data_, TEST_PORT_LOOPBACK_A_PORTID),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID).data);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(loopback_simple, _api),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID).api);

    /* port 4 */
    TEST_ASSERT_EQUAL(4, TEST_PORT_LOOPBACK_B_PORTID);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID),
                          ts_ports[TEST_PORT_LOOPBACK_B_PORTID]);
    TEST_ASSERT_EQUAL_UINT8(TEST_PORT_LOOPBACK_B_LOCID,
                            TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID).loc_id);
    TEST_ASSERT_EQUAL_STRING(TEST_PORT_LOOPBACK_B_NAME,
                             TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID).name);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_config_, TEST_PORT_LOOPBACK_B_PORTID),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID).config);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(ts_port_data_, TEST_PORT_LOOPBACK_B_PORTID),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID).data);
    TEST_ASSERT_EQUAL_PTR(&TS_CAT(loopback_simple, _api),
                          TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID).api);
}

/**
 * @brief Test communication context node table
 *
 * This test verifies node table usage:
 * - ts_ctx_node_init_phantom() - called by ts_ctx_node_get()
 * - ts_ctx_node_get()
 * - ts_ctx_node_lookup()
 */
void test_ctx_com_node(void)
{
    int ret;
    uint16_t node_idx;

    /* Prepare test context */

    ret = thingset_init(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL(0, ret);

    ret = thingset_run(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL(0, ret);

    /* Run tests */

    ret = ts_ctx_node_lookup(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    node_idx = UINT16_MAX;
    ret = ts_ctx_node_get(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, node_idx);

    node_idx = UINT16_MAX;
    ret = ts_ctx_node_lookup(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, node_idx);

    node_idx = UINT16_MAX;
    ret = ts_ctx_node_lookup(TEST_INSTANCE_LOCID, &
    test_uid_instance, &node_idx);
    TEST_ASSERT_NOT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, node_idx);

    /* Get already existing device table entry */
    ret = ts_ctx_node_get(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, node_idx);

    node_idx = UINT16_MAX;
    ret = ts_ctx_node_lookup(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, node_idx);

    struct ts_ctx_com_data *test_ts_instance_data = &TS_CAT(ts_ctx_com_data_, TEST_INSTANCE_LOCID);

    /* Fake node 0 to be the oldest one */
    test_ts_instance_data->node_table.nodes[0].last_seen_time = 0;
    for (uint16_t i = 1; i < TS_ARRAY_SIZE(test_ts_instance_data->node_table.nodes); i++) {
        test_ts_instance_data->node_table.nodes[i].last_seen_time = 1;
    }
    node_idx = ts_ctx_node_evict(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL_UINT16(0, node_idx);

    /* Free all nodes - silently ignore if node is already freed */
    for (uint16_t i = 1; i < TS_ARRAY_SIZE(test_ts_instance_data->node_table.nodes); i++) {
        ts_ctx_node_free(TEST_INSTANCE_LOCID, i);
        TEST_ASSERT_EQUAL_UINT16(THINGSET_PORT_ID_INVALID,
                                 test_ts_instance_data->node_table.nodes[i].port_id);
    }

    /* Aquire all nodes */
    thingset_uid_t node_ids[TS_ARRAY_SIZE(test_ts_instance_data->node_table.nodes)];
    for (uint16_t i = 1; i < TS_ARRAY_SIZE(test_ts_instance_data->node_table.nodes); i++) {
        node_ids[i] = i;
        ret = ts_ctx_node_get(TEST_INSTANCE_LOCID, &node_ids[i], &node_idx);
        TEST_ASSERT_EQUAL(0, ret);
    }
    /* Additional aquire should not fail */
    ret = ts_ctx_node_get(TEST_INSTANCE_LOCID, &test_uid_neighbour, &node_idx);
    TEST_ASSERT_EQUAL(0, ret);
}

/**
 * @brief Test core context
 */
void test_ctx_core(void)
{
    /* Assure core context is enabled for testing */
    TEST_ASSERT_TRUE(TS_CONFIG_CORE);

    /* Assure "core" test context static init */
    const struct ts_ctx *test_ts_core = &TS_CAT(ts_ctx_, TEST_CORE_LOCID);
    struct ts_ctx_data *test_ts_core_data = &TS_CAT(ts_ctx_core_data_, TEST_CORE_LOCID).common;
    const void *test_ts_core_variant = &TS_CAT(ts_ctx_core_, TEST_CORE_LOCID);

    TEST_ASSERT_EQUAL(0, TEST_CORE_LOCID);
    TEST_ASSERT_EQUAL_UINT8(TS_CTX_TYPE_CORE, test_ts_core->ctx_type);
    TEST_ASSERT_EQUAL_UINT16(TEST_CORE_LOCID, test_ts_core->db_id);
    TEST_ASSERT_EQUAL_PTR(test_ts_core_data, test_ts_core->data);
    TEST_ASSERT_EQUAL_PTR(test_ts_core_variant, test_ts_core->variant);

    /* Assure specific context is correctly detected when used as generic context */
    TEST_ASSERT_TRUE(TS_CTX_IS_CORE(TEST_CORE_LOCID));

    /* Assure specific context is correctly accessed by generic context functions */
    thingset_authorisation_set(TEST_CORE_LOCID, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, test_ts_core->data->_auth_flags);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, thingset_authorisation(TEST_CORE_LOCID));

    int ret = thingset_init(TEST_CORE_LOCID);
    TEST_ASSERT_EQUAL(0, ret);
}

void tests_ctx(void)
{
    UNITY_BEGIN();

    // Test environment
    RUN_TEST(test_ctx_core);
    RUN_TEST(test_ctx_com);
    RUN_TEST(test_ctx_com_port);
    RUN_TEST(test_ctx_com_node);

    UNITY_END();
}
