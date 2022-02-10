/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet logging.
 */

#ifndef TS_IMPL_LOG_H_
#define TS_IMPL_LOG_H_

#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME thingset
#define LOG_LEVEL CONFIG_THINGSET_LOG_LEVEL
#endif
#include <logging/log.h>

#ifdef TS_IMPL_LOG_REGISTER
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#else
LOG_MODULE_DECLARE(LOG_MODULE_NAME);
#endif

#define LOG_ALLOC_STR(str)	((str == NULL) ? log_strdup("null") : \
                                                log_strdup(str))

#define TS_IMPL_LOGE(...) LOG_ERR(__VA_ARGS__)
#define TS_IMPL_LOGW(...) LOG_WRN(__VA_ARGS__)
#define TS_IMPL_LOGI(...) LOG_INF(__VA_ARGS__)
#define TS_IMPL_LOGD(...) LOG_DBG(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

/* None */

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_LOG_H_ */
