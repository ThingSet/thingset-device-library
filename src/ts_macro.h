/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet utility macros (private interface)
 */

#ifndef TS_MACRO_H_
#define TS_MACRO_H_

/* ThingSet environment implementation interface */

/**
 * @addtogroup ts_impl_api
 * @{
 */

#if TS_DOXYGEN

/**
 * @brief Implementation for @ref TS_ASSERT.
 */
#define TS_IMPL_ASSERT(test, fmt, ...)

/**
 * @brief Implementation for @ref TS_ARRAY_SIZE.
 */
#define TS_IMPL_ARRAY_SIZE(array)

/**
 * @brief Implementation for @ref TS_FOR_EACH.
 */
#define TS_IMPL_FOR_EACH(F, sep, ...)

/**
 * @brief Implementation for @ref TS_NUM_VA_ARGS_LESS_1.
 */
#define TS_IMPL_NUM_VA_ARGS_LESS_1(...)

#endif /* TS_DOXYGEN */

/**
 * @}  <!-- ts_impl_api -->
 */

/**
 * @brief ThingSet utility macros.
 *
 * @defgroup ts_macro_api_priv ThingSet utility macros (private interface)
 * @{
 */

/**
 * @def TS_ASSERT
 *
 * @brief Evaluate an expression and generate a debug report when the result is False.
 *
 * @note TS_IMPL_ASSERT to be provided by the implementation
 *
 * @param test Expression to be evaluated
 * @param fmt Format string for debug report.
 * @param ... Variable number of argumentes for the format string.
 */
#define TS_ASSERT(test, fmt, ...)  TS_IMPL_ASSERT(test, fmt, ##__VA_ARGS__)

/**
 * @def TS_STATIC_ASSERT
 *
 * @brief Check boolean condition at compile time.
 *
 * Assertion that is checked at compile time.
 *
 * @param bool_cond Boolean condition.
 * @param message Message to be displayed on assertion fail.
 */
#if ((__STDC__VERSION__ - 0) >= 201112L) || defined(static_assert)
#define TS_STATIC_ASSERT(bool_cond, message) static_assert(bool_cond, message)
#else
#define TS_STATIC_ASSERT(bool_cond, message)                                                    \
    static char const static_assertion##__COUNTER__[ (bool_cond) ? 1 : -1 ] = { message }
#endif

/**
 * @def TS_EXPAND
 *
 * @brief Expand arguments.
 *
 * @param ... Variable number of arguments to be expanded.
 */
#define TS_EXPAND(...) __VA_ARGS__

/**
 * @def TS_STRINGIFY
 *
 * @brief Stringify the argument after it has been expanded.
 *
 * @param x The largument to be stringified.
 */
#define TS_STRINGIFY(x) TS_PRIMITIVE_STRINGIFY(x)

/**
 * @def TS_PRIMITIVE_STRINGIFY
 *
 * @brief Stringify the argument before it is expanded.
 *
 * @param x The largument to be stringified.
 */
#define TS_PRIMITIVE_STRINGIFY(x) #x

/**
 * @def TS_CAT
 *
 * @brief Concatenate arguments after they have been expanded.
 *
 * @param a The left operator of the concatenation.
 * @param ... The right operator of the concatenation.
 */
#define TS_CAT(a, ...) TS_PRIMITIVE_CAT(a, __VA_ARGS__)

/**
 * @def TS_PRIMITIVE_CAT
 *
 * @brief Concatenate arguments before they are expanded.
 *
 * @param a The left operator of the concatenation.
 * @param ... The right operator of the concatenation.
 */
#define TS_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

/**
 * @def TS_CALL
 *
 * @brief Call a macro @p F with given arguments.
 *
 * Example:
 *
 *     #define MY_ARGS (my_var, 4)
 *     #define F(x, y) int x = y;
 *     TS_CALL(F, MY_ARGS)
 *
 * This expands to:
 *
 *     int my_var = 4;
 *
 * @param F Macro to invoke
 * @param args Arguments. Must be in parentheses!
 */
#define TS_CALL(F, args) TS_EXPAND(F)args

/**
 * @def TS_ARRAY_SIZE
 *
 * @brief Number of elements in the given @p array
 *
 * @note TS_IMPL_ARRAY_SIZE to be provided by the implementation
 *
 * @param array Array
 */
#define TS_ARRAY_SIZE(array) TS_IMPL_ARRAY_SIZE(array)

/**
 * @def TS_FOR_EACH
 *
 * @brief Call a macro @p F on each provided argument with a given
 *        separator between each call.
 *
 * Example:
 *
 *     #define F(x) int a##x
 *     TS_FOR_EACH(F, (;), 4, 5, 6);
 *
 * This expands to:
 *
 *     int a4;
 *     int a5;
 *     int a6;
 *
 * @note TS_IMPL_FOR_EACH to be provided by the implementation
 *
 * @param F Macro to invoke
 * @param sep Separator (e.g. comma or semicolon). Must be in parentheses;
 *            this is required to enable providing a comma as separator.
 * @param ... Variable argument list. The macro @p F is invoked as
 *            <tt>F(element)</tt> for each element in the list.
 */
#define TS_FOR_EACH(F, sep, ...) TS_IMPL_FOR_EACH(F, sep, __VA_ARGS__)

/**
 * @def TS_NUM_VA_ARGS_LESS_1
 *
 * @brief Number of arguments in the variable arguments list minus one.
 *
 * @note TS_IMPL_NUM_VA_ARGS_LESS_1 to be provided by the implementation
 *
 * @param ... List of arguments
 * @return  Number of variadic arguments in the argument list, minus one
 */
#define TS_NUM_VA_ARGS_LESS_1(...) TS_IMPL_NUM_VA_ARGS_LESS_1(__VA_ARGS__)

/**
 * @} <!-- ts_msg_api_pub -->
 */

#endif /* TS_MACRO_H_ */
