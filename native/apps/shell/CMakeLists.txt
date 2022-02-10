# Copyright (c) 2022 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

target_sources(ts PRIVATE ${THINGSET_BASE}/apps/shell/ts_shell.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_shell.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_shell_linenoise.c)
message(STATUS "ThingSet: Added Shell Application")
