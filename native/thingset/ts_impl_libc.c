/*
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

char *ts_strnstr(const char *haystack, const char *needle, size_t len)
{
    int i;
    size_t needle_len;

    if (0 == (needle_len = strnlen(needle, len))) {
        return (char *)haystack;
    }

    for (i=0; i<=(int)(len-needle_len); i++) {
        if ((haystack[0] == needle[0]) &&
            (0 == strncmp(haystack, needle, needle_len))) {
            return (char *)haystack;
        }

        haystack++;
    }
    return NULL;
}
