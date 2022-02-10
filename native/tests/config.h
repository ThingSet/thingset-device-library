/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief native test configuration.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_THINGSET_CORE            1
#define CONFIG_THINGSET_COM             1
#define CONFIG_THINGSET_LOCAL_COUNT     3
#define CONFIG_THINGSET_PORT_COUNT      5

#define CONFIG_THINGSET_UNIT_TEST       1
#define CONFIG_THINGSET_LOG             0
#define CONFIG_THINGSET_ASSERT          1

#define CONFIG_THINGSET_64BIT_TYPES_SUPPORT   1
#define CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT  1

/* enable testing of shell application */
#define CONFIG_THINGSET_SHELL           1
#define CONFIG_THINGSET_SHELL_NAME      "test_shell"
#define CONFIG_THINGSET_SHELL_LOCID     1
#define CONFIG_THINGSET_SHELL_PORTID    2
#define CONFIG_THINGSET_SHELL_MEM_SIZE  30000

#endif /* CONFIG_H_ */
