/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet shell app (private interface)
 */

#ifndef TS_SHELL_H_
#define TS_SHELL_H_

/**
 * @brief ThingSet shell app.
 *
 * @defgroup ts_shell_api_priv ThingSet message (private interface)
 * @{
 */

#include "../../src/thingset_env.h"
#include "../../src/thingset_time.h"
#include "../../src/thingset_port.h"
#include "../../src/thingset_app.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @} <!-- ts_shell_api_priv -->
 * @addtogroup ts_impl_api
 * @{
 */

#if TS_DOXYGEN

/**
 * @brief Implementation for @ref TS_SHELL_CMD_HELP_PRINTED.
 */
#define TS_IMPL_SHELL_CMD_HELP_PRINTED

/**
 * @brief Implementation for @ref ts_shell.
 */
#define ts_impl_shell

/**
 * @brief Implementation for @ref ts_shell_print.
 */
#define ts_impl_shell_print(shell, format, ...)

/**
 * @brief Implementation for @ref ts_shell_error.
 */
#define ts_impl_shell_error(shell, format, ...)

#endif /* TS_DOXYGEN */

/**
 * @brief Initialise specific shell implementation.
 *
 * Called by @ref ts_shell_init after basic shell initialisation.
 */
int ts_impl_shell_init(const struct thingset_app *app);

/**
 * @brief Implementation for @ref ts_shell_run.
 */
int ts_impl_shell_run(const struct thingset_app *app);

/**
 * @brief Implementation for @ref ts_shell_get_line.
 */
int ts_impl_shell_get_line(const struct ts_impl_shell *shell, const char *prompt, char **line);

/**
 * @brief Implementation for @ref ts_shell_execute_cmd.
 */
int ts_impl_shell_execute_cmd(const char* cmd);

/**
 * @brief Implementation for @ref ts_shell_execute_output.
 */
int ts_impl_shell_execute_output(size_t *sizep, const char **output);

/**
 * @brief Implementation for @ref ts_shell_join.
 */
int ts_impl_shell_join(void);

/**
 * @}  <!-- ts_impl_api -->
 * @addtogroup ts_shell_api_priv
 * @{
 */

struct ts_shell_config {
};

struct ts_shell_data {
    /** @brief Destination port for messages created by the shell. */
    thingset_portid_t port_id_dst;
    /** @brief Size hint for response message to the shell. */
    uint16_t response_size_hint;
};

extern const struct ts_shell_config ts_shell_config;
extern struct ts_shell_data ts_shell_data;;

/**
 * @def ts_shell
 *
 * Shell structure.
 */
#define ts_shell ts_impl_shell

/**
 * @def TS_SHELL_CMD_HELP_PRINTED
 *
 * Return value of command to indicate help text was printed.
 */
#define TS_SHELL_CMD_HELP_PRINTED TS_IMPL_SHELL_CMD_HELP_PRINTED

/**
 * @brief Print normal message to the shell.
 *
 * @param[in] shell Pointer to the shell instance.
 * @param[in] format Format string.
 * @param[in] ... List of parameters to print.
 */
#define ts_shell_print(shell, format, ...)  ts_impl_shell_print(shell, format, ##__VA_ARGS__)

/**
 * @brief Print error message to the shell.
 *
 * @param[in] shell Pointer to the shell instance.
 * @param[in] format Format string.
 * @param[in] ... List of parameters to print.
 */
#define ts_shell_error(shell, format, ...)  ts_impl_shell_error(shell, format, ##__VA_ARGS__)

int ts_shell_init(const struct thingset_app *app);

static inline int ts_shell_run(const struct thingset_app *app)
{
    return ts_impl_shell_run(app);
}

int ts_shell_cmd_txt(const struct ts_shell *shell, size_t argc, char **argv);

int ts_shell_cmd_node(const struct ts_shell *shell, size_t argc, char **argv);

int ts_shell_cmd_port(const struct ts_shell *shell, size_t argc, char **argv);

int ts_shell_cmd_obj(const struct ts_shell *shell, size_t argc, char **argv);

int ts_shell_cmd_ts(const struct ts_shell *shell, size_t argc, char **argv);

int ts_shell_cmd(const struct ts_shell *shell, size_t argc, char **argv);

/**
 * @brief Get next line of shell input.
 *
 * Line must be freed using ts_shell_free().
 *
 * @param[in] prompt Prompt to display.
 * @param[out] line New input line allocated with ts_shell_alloc().
 * @return 0 on success, <0 otherwise.
 */
static inline int ts_shell_get_line(const struct ts_shell *shell, const char *prompt, char **line)
{
    return ts_impl_shell_get_line(shell, prompt, line);
}

/**
 * @brief Execute command.
 *
 * Pass command line to shell to execute.
 *
 * @note This function should only be used for debugging/diagnostic.
 *
 *	@warning This function shall not be called from shell command context!
 *
 * @param[in] cmd	Command to be executed.
 * @return Result of the execution.
 */
static inline int ts_shell_execute_cmd(const char* cmd)
{
    return ts_impl_shell_execute_cmd(cmd);
}

/**
 * @brief Returns the buffered output of ts_shell_execute_cmd() and resets the pointer
 *
 * The returned data is always followed by a nul character at position *sizep
 *
 * @note This function should only be used for debugging/diagnostic.
 *
 *	@warning This function shall not be called from shell command context!
 *
 * @param[out] sizep	Returns size of data in shell buffer
 * @param[out] output Returns pointer to buffer containing shell execute output
 */
static inline int ts_shell_execute_output(size_t *sizep, const char **output)
{
    return ts_impl_shell_execute_output(sizep, output);
}

/**
 * @brief Wait for (all) shell(s) to end working.
 */
static inline int ts_shell_join(void)
{
    return ts_impl_shell_join();
}

/**
 * @brief Helper.
 *
 * Modifies @p cmd. @p argv array is allocated by the function and shall be freed by ts_shell_free()
 * after usage.
 */
int ts_shell_split_cmd(char *cmd, int *argc, char ***argv);

/**
 * @brief Allocate memory block from shell memory pool.
 *
 * Allocates and returns a memory block from the memory region owned by the shell(s). The total size
 * size of the memory region is given  by @ref TS_CONFIG_SHELL_MEM_SIZE.
 *
 * If no memory is available immediately, the call will block for the specified timeout waiting for
 * memory to be freed. If the allocation cannot be performed by the expiration of the timeout, an
 * error will be returned.
 *
 * @param[in] block_size Size of memory block (in bytes).
 * @param[in] timeout_ms Maximum time to wait in milliseconds.
 * @param[out] mem_block Pointer to memory block.
 * @return 0 on success, <0 otherwise.
 * @retval -ENOMEM Returned without waiting on no memory available.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Invalid data supplied.
 */
int ts_shell_alloc(size_t block_size, thingset_time_ms_t timeout_ms, uint8_t **mem_block);

/**
 * @brief Free memory block allocated from the shell memory pool.
 *
 * Release a previously allocated memory block back to the shell memory pool.
 *
 * @param[in] mem_pool Address of the memory pool.
 * @param[in] mem_block Pointer to memory block.
 * @return 0 on success, <0 otherwise
 */
int ts_shell_free(const uint8_t *mem_block);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @page ts_topic_app_shell ThingSet shell application

*/

/**
 * @} <!-- ts_shell_api_priv -->
 */


#endif /* TS_SHELL_H_ */
