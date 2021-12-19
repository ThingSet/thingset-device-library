/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2021 Martin Jäger / Libre Solar
 */

#include <stdio.h>

#include <thingset.h>

/* variables to be exposed via ThingSet */
thingset_uid_t uid = 0x1111111111111111ULL;
char device_id[] = "ABCD1234";
bool enable_switch = false;
float ambient_temp = 22.3;

/* function that can be called via ThingSet */
void reset(void)
{
    printf("Reset function called!\n");
}

/* The ThingSet data objects database */
THINGSET_CORE_DATABASE_DEFINE(uid,

    TS_ITEM_STRING(0x1D, "DeviceID", device_id, sizeof(device_id),
        TS_ID_ROOT, TS_ANY_R, 0),

    TS_ITEM_FLOAT(0x71, "Ambient_degC", &ambient_temp, 1,
        TS_ID_ROOT, TS_ANY_R, 0),

    TS_ITEM_BOOL(0x61, "HeaterEnable", &enable_switch,
        TS_ID_ROOT, TS_ANY_RW, 0),

    TS_FUNCTION(0xE1, "x-reset", &reset,
        TS_ID_ROOT, TS_ANY_RW),

);

THINGSET_CORE_DEFINE();

#if CONFIG_THINGSET_ZEPHYR
void main(void)
#else
int main(void)
#endif
{
    uint8_t response[100];

    /* Initialize core variant ThingSet context and assign data objects */
    thingset_core_init();

    /*
     * Below requests are for demonstration of the ThingSet process function only. They would
     * usually be received from a connected serial interface or other communication channels.
     */

    const char request1[] = "= {\"HeaterEnable\":true}";
    printf("Request:   %s\n", request1);
    thingset_core_process((uint8_t *)request1, strlen(request1), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

    const char request2[] = "!x-reset";
    printf("Request:   %s\n", request2);
    thingset_core_process((uint8_t *)request2, strlen(request2), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

    const char request3[] = "?";
    printf("Request:   %s\n", request3);
    thingset_core_process((uint8_t *)request3, strlen(request3), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

#if !CONFIG_THINGSET_ZEPHYR
    return 0;
#endif
}
