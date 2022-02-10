/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet logging (private interface)
 */

#ifndef TS_LOG_H_
#define TS_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ThingSet environment implementation interface */

/**
 * @addtogroup ts_impl_api
 * @{
 */

#if TS_DOXYGEN

/**
 * @brief Implementation for @ref TS_LOGE.
 */
#define TS_IMPL_LOGE(...)

/**
 * @brief Implementation for @ref TS_LOGW.
 */
#define TS_IMPL_LOGW(...)

/**
 * @brief Implementation for @ref TS_LOGI.
 */
#define TS_IMPL_LOGI(...)

/**
 * @brief Implementation for @ref TS_LOGD.
 */
#define TS_IMPL_LOGD(...)

#endif /* TS_DOXYGEN */

/**
 * @}  <!-- ts_impl_api -->
 */

/**
 * @brief ThingSet logging.
 *
 * @defgroup ts_log_api_priv ThingSet logging (private interface)
 * @{
 */

/**
 * @def TS_LOGE
 *
 * @brief Writes an ERROR level message to the log.
 *
 * It’s meant to report severe errors, such as those from which it’s not possible to recover.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] ... A string optionally containing printf valid conversion specifier,
 *                followed by as many values as specifiers.
 */
#define TS_LOGE(...) TS_IMPL_LOGE(__VA_ARGS__)

/**
 * @def TS_LOGW
 *
 * @brief Writes an WARNING level message to the log.
 *
 * It’s meant to report messages related to unusual situations that are not necessarily errors.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] ... A string optionally containing printf valid conversion specifier,
 *                followed by as many values as specifiers.
 */
#define TS_LOGW(...) TS_IMPL_LOGW(__VA_ARGS__)

/**
 * @def TS_LOGI
 *
 * @brief Writes an INFO level message to the log.
 *
 * It’s meant to write generic user oriented messages.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] ... A string optionally containing printf valid conversion specifier,
 *                followed by as many values as specifiers.
 */
#define TS_LOGI(...) TS_IMPL_LOGI(__VA_ARGS__)

/**
 * @def TS_LOGD
 *
 * @brief Writes an DEBUG level message to the log.
 *
 * It’s meant to write developer oriented information.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] ... A string optionally containing printf valid conversion specifier,
 *                followed by as many values as specifiers.
 */
#define TS_LOGD(...) TS_IMPL_LOGD(__VA_ARGS__)

/**
 * @} <!-- ts_log_api_priv -->
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TS_LOG_H_ */
