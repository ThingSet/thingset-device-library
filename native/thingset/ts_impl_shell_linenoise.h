/*
 * Copyright (c) 2010-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * SPDX-License-Identifier: BSD-2-Clause
 */

/**
 * @file
 * @brief Linenoise for native implementation of ThingSet shell app.
 */

/* linenoise.h -- VERSION 1.0
 *
 * Guerrilla line editing library against the idea that a line editing lib
 * needs to be 20,000 lines of C code.
 *
 * See ts_impl_shell_linenoise.c for more.
 */

#ifndef TS_IMPL_SHELL_LINENOISE_H_
#define TS_IMPL_SHELL_LINENOISE_H_

#include <stdbool.h>

/* Forward declaration */
struct linenoiseCompletions;

typedef void(linenoiseCompletionCallback)(const char *, struct linenoiseCompletions *);
typedef char*(linenoiseHintsCallback)(const char *, int *color, int *bold);
typedef void(linenoiseFreeHintsCallback)(void *);
void linenoiseSetCompletionCallback(linenoiseCompletionCallback *);
void linenoiseSetHintsCallback(linenoiseHintsCallback *);
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback *);
void linenoiseAddCompletion(struct linenoiseCompletions *, const char *);

int linenoiseProbe(void);
char *linenoise(const char *prompt);
void linenoiseFree(void *ptr);
int linenoiseHistoryAdd(const char *line);
int linenoiseHistorySetMaxLen(int len);
int linenoiseHistorySave(const char *filename);
int linenoiseHistoryLoad(const char *filename);
void linenoiseHistoryFree(void);
void linenoiseClearScreen(void);
void linenoiseSetMultiLine(int ml);
void linenoiseSetDumbMode(int set);
bool linenoiseIsDumbMode(void);
void linenoisePrintKeyCodes(void);
void linenoiseAllowEmpty(bool);
int linenoiseSetMaxLineLen(size_t len);

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_SHELL_LINENOISE_H_ */
