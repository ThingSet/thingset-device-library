/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2021 Martin Jäger / Libre Solar
 */

#include <stdio.h>

#include <thingset.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* variables to be exposed via ThingSet */
char device_id[] = "ABCD1234";
bool enable_switch = false;
float ambient_temp = 22.3;

/* function that can be called via ThingSet */
void reset(void)
{
    printf("Reset function called!\n");
}

/* the ThingSet object definitions */
static struct ts_data_object data_objects[] = {

    TS_ITEM_STRING(0x1D, "DeviceID", device_id, sizeof(device_id),
        TS_ID_ROOT, TS_ANY_R, 0),

    TS_ITEM_FLOAT(0x71, "Ambient_degC", &ambient_temp, 1,
        TS_ID_ROOT, TS_ANY_R, 0),

    TS_ITEM_BOOL(0x61, "HeaterEnable", &enable_switch,
        TS_ID_ROOT, TS_ANY_RW, 0),

    TS_FUNCTION(0xE1, "x-reset", &reset,
        TS_ID_ROOT, TS_ANY_RW),
};

int main(void)
{
    uint8_t response[100];
    struct ts_context ts;

    /* initialize ThingSet context and assign data objects */
    ts_init(&ts, data_objects, ARRAY_SIZE(data_objects));

    /*
     * Below requests are for demonstration of the ThingSet process function only. They would
     * usually be received from a connected serial interface or other communication channels.
     */

    const char request1[] = "= {\"HeaterEnable\":true}";
    printf("Request:   %s\n", request1);
    ts_process(&ts, (uint8_t *)request1, strlen(request1), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

    const char request2[] = "!x-reset";
    printf("Request:   %s\n", request2);
    ts_process(&ts, (uint8_t *)request2, strlen(request2), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

    const char request3[] = "?";
    printf("Request:   %s\n", request3);
    ts_process(&ts, (uint8_t *)request3, strlen(request3), response, sizeof(response));
    printf("Response:  %s\n\n", (char *)response);

    return 0;
}
