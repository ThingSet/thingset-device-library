/* ThingSet protocol library
 * Copyright (c) 2017-2018 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ts_config.h"
#include "thingset.h"
#include "jsmn.h"

#include <string.h>
#include <stdio.h>

#define DEBUG 0

void thingset_process(ts_buffer_t *req, ts_buffer_t *resp, ts_data_t *data)
{
    static ts_parser_t tsp;

    if (req->data.bin[0] == TS_FUNCTION_READ) {
        thingset_read_cbor(req, resp, data);
    }
    else if (req->data.bin[0] == TS_FUNCTION_WRITE) {
        thingset_write_cbor(req, resp, data);
    }
    else if (req->data.str[0] == '!') {      // JSON request
        jsmn_init(&(tsp.parser));
        if (req->pos > 4 && strncmp(req->data.str, "!read", 5) == 0) {
            tsp.str = req->data.str+6;
            tsp.tok_count = jsmn_parse(&(tsp.parser), tsp.str, req->pos-6, tsp.tokens, TS_NUM_JSON_TOKENS);
            //printf("read command, data: %s, num_tok: %d\n", tsp.str, tsp.tok_count);
            thingset_read_json(&tsp, resp, data);
        }
        else if (req->pos > 5 && strncmp(req->data.str, "!write", 6) == 0) {
            tsp.str = req->data.str+7;
            tsp.tok_count = jsmn_parse(&(tsp.parser), tsp.str, req->pos-7, tsp.tokens, TS_NUM_JSON_TOKENS);
            //printf("write command, data: %s, num_tok: %d\n", tsp.str, tsp.tok_count);
            thingset_write_json(&tsp, resp, data);
        }
        else if (req->pos > 5 && strncmp(req->data.str, "!list", 5) == 0) {
            tsp.str = req->data.str+6;
            tsp.tok_count = jsmn_parse(&(tsp.parser), tsp.str, req->pos-6, tsp.tokens, TS_NUM_JSON_TOKENS);
            thingset_list_json(&tsp, resp, data);
        }
        // quick and dirty hack to go into DFU mode!
        //else if (req_len >= 4 && strncmp(req, "!dfu", 4) == 0) {
            //dfu_run_bootloader();
        //}
        else if (req->pos >= 4 && strncmp(req->data.str, "!pub", 4) == 0) {
            // TODO!!
        }
        else {
            thingset_status_message_json(resp, TS_STATUS_UNKNOWN_FUNCTION);
        }
    }
    else {
        // not a thingset command --> ignore and set response to empty string
        resp->data.str[0] = 0;
        resp->pos = 0;
        //thingset_status_message(ts, TS_STATUS_UNKNOWN_FUNCTION);
    }
}
