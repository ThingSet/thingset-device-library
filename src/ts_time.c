/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_env.h"
#include "thingset_time.h"

#include <math.h>

thingset_time_ms_t thingset_time_ms(void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }
    return (thingset_time_ms_t)((s * 1000) + ms);
}

thingset_time_ms_t thingset_time_ms_delta(thingset_time_ms_t reftime)
{
    thingset_time_ms_t curtime = thingset_time_ms();

    if (curtime >= reftime) {
        return curtime - reftime;
    }
    return (THINGSET_TIME_MS_MAX - reftime) + curtime;
}

struct timespec thingset_time_timeout_spec(thingset_time_ms_t timeout_ms)
{
    struct timespec timeout;

    if (timeout_ms == THINGSET_TIMEOUT_FOREVER) {
        timeout.tv_sec =  THINGSET_TIMEOUT_FOREVER;
        timeout.tv_nsec = 1e9L - 1;
    }
    else if (timeout_ms == THINGSET_TIMEOUT_IMMEDIATE) {
        /* now */
        clock_gettime(CLOCK_REALTIME, &timeout);
    }
    else {
        clock_gettime(CLOCK_REALTIME, &timeout);

        time_t timeout_sec = timeout_ms / 1000;
        timeout.tv_sec += timeout_sec;
        timeout.tv_nsec += (timeout_ms - (timeout_sec * 1000)) * 1000;
        if (timeout.tv_nsec >= 1000000000L) {
            timeout.tv_sec += 1;
            timeout.tv_sec -= 1000000000L;
        }
    }

    return timeout;
}
