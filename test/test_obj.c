/*
 * Copyright (c) 2021 Bobby Noelte
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/**
 * @brief Test object database initialisation
 *
 * This test verifies object database usage:
 * - THINGSET_DATABASE_DEFINE()
 * - ts_obj_db_init()
 */
void test_obj_db_init(void)
{
    int ret;

    /* Assert two local databases are configured for test */
    TEST_ASSERT_EQUAL(3, TS_CONFIG_LOCAL_COUNT);

    /* Check static initialisation constants */
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_R, (TS_MKR_R | TS_EXP_R | TS_USR_R));
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_W, (TS_MKR_W | TS_EXP_W | TS_USR_W));
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_RW, (TS_ANY_R | TS_ANY_W));

    /* check static initialisation */
    thingset_oref_t oref;
    ret = ts_obj_db_oref_by_id(ts_ctx_obj_db(TEST_INSTANCE_LOCID), ID_ROOT, &oref);
    TEST_ASSERT_EQUAL_INT(0, ret);

    /* Check dynamic initialisation */
    ts_obj_db_init();

    ret = ts_obj_db_check_id_duplicates(TEST_INSTANCE_LOCID);
    TEST_ASSERT_EQUAL_INT(0, ret);

    /* Check whether mutable object meta data were correctly initialised */
    const struct ts_obj_db *test_db_instance = ts_obj_db_by_id(TEST_INSTANCE_LOCID);
    for (unsigned int i = 0; i < test_db_instance->num; i++) {
        TEST_ASSERT_EQUAL_UINT16(test_db_instance->objects[i].meta_default.subsets,
                                 test_db_instance->meta[i].subsets);
        TEST_ASSERT_EQUAL_UINT16(test_db_instance->objects[i].meta_default.detail,
                                 test_db_instance->meta[i].detail);
        TEST_ASSERT_EQUAL_UINT16(test_db_instance->objects[i].meta_default.access,
                                 test_db_instance->meta[i].access);
    }
}

/**
 * @brief Test static initialisation of data objects
 *
 * This test verifies static initialsation macro usage:
 * - TS_OBJ()
 */
void test_obj_static_init(void)
{
    int ret;
    thingset_oref_t oref;

    /* Test TS_GROUP static initialization */

    ret = ts_obj_db_oref_by_id(ts_ctx_obj_db(TEST_CORE_LOCID), ID_CONF, &oref);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_UINT8(TS_T_GROUP, ts_obj_type(oref));
    TEST_ASSERT_EQUAL_STRING("conf", ts_obj_name(oref));
    TEST_ASSERT_EQUAL_UINT16(ID_ROOT, ts_obj_parent_id(oref));
    TEST_ASSERT_EQUAL_PTR(&test_core_conf_callback, ts_obj_exec_data(oref));

    ret = ts_obj_db_oref_by_id(ts_ctx_obj_db(TEST_CORE_LOCID), ID_INPUT, &oref);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_UINT8(TS_T_GROUP, ts_obj_type(oref));
    TEST_ASSERT_NULL(ts_obj_exec_data(oref));

}

/**
 * @brief Test static initialisation of data objects
 *
 * This test verifies object logging:
 * - thingset_dump_json()
 * - thingset_obj_log()
 */
void test_obj_log(void)
{
    int ret;
    thingset_oref_t oref;

    /* Assure dump works on core context database */
    thingset_dump_json(TS_ID_ROOT, 0);

    ret = ts_obj_db_oref_by_id(ts_ctx_obj_db(TEST_NEIGHBOUR_LOCID), TS_ID_ROOT, &oref);
    TEST_ASSERT_EQUAL_INT(0, ret);
    thingset_obj_log(oref, &test_log_buf[0], sizeof(test_log_buf));
    TEST_ASSERT_EQUAL_STRING_LEN(
"{\n\
    \"DeviceID\": {\n\
        \"db_oid\": 0,\n\
        \"obj_id\": 29,\n\
        \"access\": \"TS_ANY_R\",\n\
        \"access_default\": \"TS_ANY_R\",\n\
        \"type\": \"TS_T_STRING\",\n\
        \"data\": \"ABCD5678\"\n\
    },\n\
    \"Ambient_degC\": {\n\
        \"db_oid\": 1,\n\
        \"obj_id\": 113,\n\
        \"access\": \"TS_ANY_R\",\n\
        \"access_default\": \"TS_ANY_R\",\n\
        \"type\": \"TS_T_FLOAT32\",\n\
        \"data\": 0.000000\n\
    },\n\
    \"HeaterEnable\": {\n\
        \"db_oid\": 2,\n\
        \"obj_id\": 97,\n\
        \"access\": \"TS_ANY_RW\",\n\
        \"access_default\": \"TS_ANY_RW\",\n\
        \"type\": \"TS_T_BOOL\",\n\
        \"data\": false\n\
    },\n\
    \"x-reset\": {\n\
        \"db_oid\": 3,\n\
        \"obj_id\": 225,\n\
        \"access\": \"TS_ANY_RW\",\n\
        \"access_default\": \"TS_ANY_RW\",\n\
        \"type\": \"TS_T_EXEC\",\n\
        \"data\": null\n\
    }\n\
}\n",
    &test_log_buf[0], sizeof(test_log_buf));
}

void tests_obj(void)
{
    UNITY_BEGIN();

    // Test environment
    RUN_TEST(test_obj_db_init);
    RUN_TEST(test_obj_static_init);
    RUN_TEST(test_obj_log);

    UNITY_END();
}
