# Copyright (c) 2021 Martin Jäger / Libre Solar
# Copyright (c) 2021..2022 Bobby Noelte.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_app.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_cobs.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_cmd.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_core.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_export.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_msg.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_node.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_obj.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_process.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_jsmn.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_coder.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_export.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_log.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_proto.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_value.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_obj.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_obj_log.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_port.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_rbbq.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_time.c)
