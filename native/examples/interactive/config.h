/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Interactive example configuration.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define TS_CONFIG_CORE      1
#define TS_CONFIG_COM       0

#define TS_CONFIG_64BIT_TYPES_SUPPORT   1
#define TS_CONFIG_DECFRAC_TYPE_SUPPORT  1

/* enable shell application */
#define CONFIG_THINGSET_SHELL           1
#define CONFIG_THINGSET_SHELL_NAME      "shell"
#define CONFIG_THINGSET_SHELL_LOCID     TS_CONFIG_CORE_LOCID
#define CONFIG_THINGSET_SHELL_PORTID    0
#define CONFIG_THINGSET_SHELL_MEM_SIZE  30000

#endif /* CONFIG_H_ */
