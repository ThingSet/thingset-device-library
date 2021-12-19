# Copyright (c) 2021 Martin Jäger / Libre Solar
# Copyright (c) 2021 Bobby Noelte.
#
# SPDX-License-Identifier: Apache-2.0

if(DEFINED CONFIG_THINGSET_PORT_LOOPBACK_SIMPLE)
target_sources(ts PRIVATE loopback_simple.c)
message(STATUS "ThingSet: Added Simple Loopback Port")
endif()
