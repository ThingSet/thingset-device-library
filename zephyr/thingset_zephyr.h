/*
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_ZEPHYR_H_
#define THINGSET_ZEPHYR_H_

#if !__ZEPYHR__
#warning "Including thingset_zephyr.h in non Zephyr environment!"
#endif

/* Logging */
#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME thingset
#define LOG_LEVEL CONFIG_THINGSET_LOG_LEVEL
#endif
#include <logging/log.h>

#ifdef THINGSET_MAIN
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#else
LOG_MODULE_DECLARE(LOG_MODULE_NAME);
#endif

#define LOG_ALLOC_STR(str)	((str == NULL) ? log_strdup("null") : \
                                                log_strdup(str))

#ifdef CONFIG_MINIMAL_LIBC
/*
 * Zephyr's minimal libc is missing some functions.
 * Provide !!sufficient!! replacements here.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define isnan(value) __builtin_isnan(value)
#define isinf(value) __builtin_isinf(value)

inline long long int llroundf(float x)
{
    return __builtin_llroundf(x);
};

double ts_strtod(const char * string, char **endPtr);
inline double strtod(const char * string, char **endPtr)
{
    return ts_strtod(string, endPtr);
};

inline long long strtoll(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (long long)strtol(str, endptr, base);
};

inline unsigned long long strtoull(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (unsigned long long)strtoul(str, endptr, base);
};
#endif

#include <zephyr.h>

#endif /* THINGSET_ZEPHYR_H_ */
