/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet environment.
 */


#ifndef THINGSET_ENV_H_
#define THINGSET_ENV_H_

/**
 * @brief ThingSet environment.
 *
 * The ThingSet device library expects a certain environment (functions, macros) to be available.
 * This environment has to be provided by a specific underlying implementation.
 *
 * @defgroup ts_impl_api ThingSet environment (implementation interface)
 * @{
 */

#if CONFIG_THINGSET_ZEPHYR
#include "../zephyr/thingset/ts_impl_env.h"
#else
/* Default environment */
#include "../native/thingset/ts_impl_env.h"
#endif

/**
 * @} <!-- ts_impl_api -->
 */

#endif /* THINGSET_ENV_H_ */
