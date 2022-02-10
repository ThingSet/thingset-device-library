# Copyright (c) 2022 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

if(DEFINED CONFIG_THINGSET_SHELL)
target_sources(ts PRIVATE ${THINGSET_BASE}/apps/shell/ts_shell.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/zephyr/thingset/ts_impl_shell.c)
message(STATUS "ThingSet: Added Shell Application")
endif()
