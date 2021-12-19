/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../src/thingset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "things.h"

thingset_uid_t uid = 0x1111111111111111ULL;

/* variables to be exposed via ThingSet */

// info
char manufacturer[] = "Libre Solar";
static uint32_t timestamp = 12345678;
static char device_id[] = "ABCD1234";

// conf
static float bat_charging_voltage = 14.4;
static float load_disconnect_voltage = 10.8;

// input
static bool enable_switch = false;

// meas
static float battery_voltage = 14.1;
static float battery_current = 5.13;
static int16_t ambient_temp = 22;

// rec
static float bat_energy_hour = 32.2;
static float bat_energy_day = 123;
static int16_t ambient_temp_max_day = 28;

// pub
bool pub_report_enable = false;
uint16_t pub_report_interval = 1000;
bool pub_info_enable = true;

// exec
char auth_password[11];

void reset_function(void)
{
    printf("Reset function called!\n");
}

void auth_function(void)
{
    const char pass_exp[] = "expert123";
    const char pass_mkr[] = "maker456";

    if (strlen(pass_exp) == strlen(auth_password) &&
        strncmp(auth_password, pass_exp, strlen(pass_exp)) == 0)
    {
        thingset_authorisation_set(TS_CONFIG_CORE_LOCID, TS_EXP_MASK | TS_USR_MASK);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        thingset_authorisation_set(TS_CONFIG_CORE_LOCID, TS_MKR_MASK | TS_USR_MASK);
    }
    else {
        thingset_authorisation_set(TS_CONFIG_CORE_LOCID, TS_USR_MASK);
    }

    printf("Auth function called, password: %s\n", auth_password);
}

void conf_callback(void)
{
    printf("Conf callback called!\n");
}

/* The ThingSet data objects database for core context */
THINGSET_CORE_DATABASE_DEFINE(uid,

    // DEVICE INFORMATION /////////////////////////////////////////////////////

    TS_GROUP(ID_INFO, "info", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_STRING(0x19, "Manufacturer", manufacturer, 0,
        ID_INFO, TS_ANY_R, 0),

    TS_ITEM_UINT32(0x1A, "Timestamp_s", &timestamp,
        ID_INFO, TS_ANY_RW, SUBSET_REPORT),

    TS_ITEM_STRING(0x1B, "DeviceID", device_id, sizeof(device_id),
        ID_INFO, TS_ANY_R | TS_MKR_W, 0),

    // CONFIGURATION //////////////////////////////////////////////////////////

    TS_GROUP(ID_CONF, "conf", &conf_callback, ID_ROOT),

    TS_ITEM_FLOAT(0x31, "BatCharging_V", &bat_charging_voltage, 2,
        ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_FLOAT(0x32, "LoadDisconnect_V", &load_disconnect_voltage, 2,
        ID_CONF, TS_ANY_RW, 0),

    // INPUT DATA /////////////////////////////////////////////////////////////

    TS_GROUP(ID_INPUT, "input", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_BOOL(0x61, "EnableCharging", &enable_switch,
        ID_INPUT, TS_ANY_RW, 0),

    // MEASUREMENT DATA ///////////////////////////////////////////////////////

    TS_GROUP(ID_MEAS, "meas", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_FLOAT(0x71, "Bat_V", &battery_voltage, 2,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT | SUBSET_CAN),

    TS_ITEM_FLOAT(0x72, "Bat_A", &battery_current, 2,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT | SUBSET_CAN),

    TS_ITEM_INT16(0x73, "Ambient_degC", &ambient_temp,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT),

    // RECORDED DATA //////////////////////////////////////////////////////////

    TS_GROUP(ID_REC, "rec", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_FLOAT(0xA1, "BatHour_kWh", &bat_energy_hour, 2,
        ID_REC, TS_ANY_R, 0),

    TS_ITEM_FLOAT(0xA2, "BatDay_kWh", &bat_energy_day, 2,
        ID_REC, TS_ANY_R, 0),

    TS_ITEM_INT16(0xA3, "AmbientMaxDay_degC", &ambient_temp_max_day,
        ID_REC, TS_ANY_R, 0),

    // REMOTE PROCEDURE CALLS /////////////////////////////////////////////////

    TS_GROUP(ID_RPC, "rpc", TS_NO_CALLBACK, ID_ROOT),

    TS_FUNCTION(0xE1, "x-reset", &reset_function,
        ID_RPC, TS_ANY_RW),

    TS_FUNCTION(0xE2, "x-auth", &auth_function,
        ID_RPC, TS_ANY_RW),

    TS_ITEM_STRING(0xE3, "Password", auth_password, sizeof(auth_password),
        0xE2, TS_ANY_RW, 0),

    // REPORTS ////////////////////////////////////////////////////////////////

    TS_SUBSET(ID_REPORT, "report", SUBSET_REPORT, ID_ROOT, TS_ANY_RW),

    // PUBLICATION DATA ///////////////////////////////////////////////////////

    TS_GROUP(ID_PUB, ".pub", TS_NO_CALLBACK, ID_ROOT),

    TS_GROUP(0xF1, "report", TS_NO_CALLBACK, ID_PUB),

    TS_ITEM_BOOL(0xF2, "Enable", &pub_report_enable,
        0xF1, TS_ANY_RW, 0),

    TS_ITEM_UINT16(0xF3, "Interval_ms", &pub_report_interval,
        0xF1, TS_ANY_RW, 0),

    TS_GROUP(0xF5, "info", TS_NO_CALLBACK, ID_PUB),

    TS_ITEM_BOOL(0xF6, "OnChange", &pub_info_enable,
        0xF5, TS_ANY_RW, 0),

);

/* The ThingSet core context */
THINGSET_CORE_DEFINE();
