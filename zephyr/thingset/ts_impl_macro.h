/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet utility macros.
 */

#ifndef TS_IMPL_MACRO_H_
#define TS_IMPL_MACRO_H_

#include <sys/util_macro.h>

#define TS_IMPL_ARRAY_SIZE(array) ARRAY_SIZE(array)

#define TS_IMPL_FOR_EACH(F, sep, ...) FOR_EACH(F, sep, __VA_ARGS__)

#define TS_IMPL_NUM_VA_ARGS_LESS_1(...) NUM_VA_ARGS_LESS_1(__VA_ARGS__)

#endif /* TS_IMPL_MACRO_H_ */
