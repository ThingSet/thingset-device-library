/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Interactive com example configuration.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_THINGSET_CORE            1
#define CONFIG_THINGSET_COM             1

/* We use test data for the example */

#define CONFIG_THINGSET_LOCAL_COUNT     3
#define CONFIG_THINGSET_PORT_COUNT      5

#define CONFIG_THINGSET_UNIT_TEST       1
#define CONFIG_THINGSET_LOG             1
#define CONFIG_THINGSET_LOG_LEVEL       4
#define CONFIG_THINGSET_ASSERT          1

#define TS_CONFIG_64BIT_TYPES_SUPPORT   1
#define TS_CONFIG_DECFRAC_TYPE_SUPPORT  1

/* enable shell application */
#define CONFIG_THINGSET_SHELL           1
#define CONFIG_THINGSET_SHELL_NAME      "shell"
#define CONFIG_THINGSET_SHELL_LOCID     1
#define CONFIG_THINGSET_SHELL_PORTID    2
#define CONFIG_THINGSET_SHELL_MEM_SIZE  30000

#endif /* CONFIG_H_ */
