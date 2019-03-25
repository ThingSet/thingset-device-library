/* ThingSet protocol library
 * Copyright (c) 2017-2019 Martin Jäger (www.libre.solar)
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
#include "cbor.h"

#include <string.h>
#include <stdio.h>

#define DEBUG 0

ThingSet::ThingSet(const data_object_t *data, size_t num)
{
    data_objects = data;
    num_objects = num;
}

ThingSet::ThingSet(const data_object_t *data, size_t num_obj, const ts_pub_channel_t *channels, size_t num_ch)
{
    data_objects = data;
    num_objects = num_obj;
    pub_channels = channels;
    num_channels = num_ch;
}

ThingSet::~ThingSet()
{

}

void ThingSet::set_pub_channels(const ts_pub_channel_t *channels, size_t num)
{
    pub_channels = channels;
    num_channels = num;
}

int ThingSet::process(uint8_t *req, size_t req_len, uint8_t *response, size_t resp_size)
{
    set_request(req, req_len);
    return get_response(response, resp_size);
}

void ThingSet::set_request(uint8_t *buf, size_t length)
{
    req = buf;
    req_len = length;
}

// returns the length of the response written to response buffer or 0 in case of error
int ThingSet::get_response(uint8_t *response, size_t resp_size)
{
    // check if proper request was set before asking for a response
    if (req == NULL || req_len < 1)
        return 0;

    int category = 0;
    if (req[0] <= TS_CAT_EXEC) {          // CBOR list/read/write request
        if (req_len == 2 && (req[1] == CBOR_NULL || req[1] == CBOR_ARRAY || req[1] == CBOR_MAP)) {
            //printf("list_cbor\n");
            return list_cbor(response, resp_size, req[0], req[1] == CBOR_MAP, req[1] == CBOR_NULL);
        }
        else if ((req[1] & CBOR_TYPE_MASK) == CBOR_MAP) {
            //printf("write_cbor\n");
            int len = write_cbor(response, resp_size, req[0], false);
            if ((response[0] - 0x80) == TS_STATUS_SUCCESS &&
                req[0] == TS_CAT_CONF && conf_callback != NULL) {
                conf_callback();
            }
            return len;
        }
        else {  // array or single data object
            if (req[0] == TS_CAT_EXEC) {
                return exec_cbor(response, resp_size);
            }
            else {
                //printf("read_cbor\n");
                return read_cbor(response, resp_size, req[0]);
            }
        }
    }
    else if (req[0] == '!') {      // JSON request

        unsigned int len_function = 0;
        if (req_len >= 5 && strncmp((char *)req, "!info", 5) == 0) {
            category = TS_CAT_INFO;
            len_function = 5;
        }
        else if (req_len >= 5 && strncmp((char *)req, "!conf", 5) == 0) {
            category = TS_CAT_CONF;
            len_function = 5;
        }
        else if (req_len >= 6 && strncmp((char *)req, "!input", 6) == 0) {
            category = TS_CAT_INPUT;
            len_function = 6;
        }
        else if (req_len >= 7 && strncmp((char *)req, "!output", 7) == 0) {
            category = TS_CAT_OUTPUT;
            len_function = 7;
        }
        else if (req_len >= 4 && strncmp((char *)req, "!rec", 4) == 0) {
            category = TS_CAT_REC;
            len_function = 4;
        }
        else if (req_len >= 4 && strncmp((char *)req, "!cal", 4) == 0) {
            category = TS_CAT_CAL;
            len_function = 4;
        }
        else if (req_len >= 5 && strncmp((char *)req, "!exec", 5) == 0) {
            category = TS_CAT_EXEC;
            len_function = 5;
        }
        else {
            return status_message_json((char *)response, resp_size, TS_STATUS_UNKNOWN_FUNCTION);
        }
        // valid function / category found

        if (req_len > len_function) {
            jsmn_parser parser;
            jsmn_init(&(parser));

            json_str = (char *)req + len_function + 1;      // +1 because blank is requested between function and JSON data
            tok_count = jsmn_parse(&parser, json_str, req_len - (len_function + 1), tokens, sizeof(tokens));

            if (tok_count == JSMN_ERROR_NOMEM) {
                return status_message_json((char *)response, resp_size, TS_STATUS_REQUEST_TOO_LONG);
            }
            else if (tok_count < 0) {
                return status_message_json((char *)response, resp_size, TS_STATUS_WRONG_FORMAT);
            }
            else if (tok_count == 0) {
                //printf("list_json: %s\n", json_str);
                return list_json((char *)response, resp_size, category);
            }
            else if (tok_count == 1 && tokens[0].type == JSMN_OBJECT) {
                return list_json((char *)response, resp_size, category, true);
            }
            else {
                if (tokens[0].type == JSMN_OBJECT) {
                    //printf("write_json: %s\n", json_str);
                    int len = write_json((char *)response, resp_size, category);
                    if (strncmp((char *)response, ":0", 2) == 0 && conf_callback != NULL &&
                        (category == TS_CAT_CONF || category == TS_CAT_INFO)) {
                        conf_callback();
                    }
                    return len;
                }
                else {
                    if (category == TS_CAT_EXEC) {
                        //printf("exec_json: %s\n", json_str);
                        return exec_json((char *)response, resp_size);
                    }
                    else {
                        //printf("read_json: %s\n", json_str);
                        return read_json((char *)response, resp_size, category);
                    }
                }
            }
        }
        else {  // only function without any blank characters --> list
            return list_json((char *)response, resp_size, category);
        }
    }
    else {
        // not a thingset command --> ignore and set response to empty string
        response[0] = 0;
        return 0;
        //thingset_status_message(ts, TS_STATUS_UNKNOWN_FUNCTION);
    }
}

void ThingSet::set_conf_callback(void (*callback)(void))
{
    conf_callback = callback;
}

const data_object_t* ThingSet::get_data_object(char *str, size_t len)
{
    //printf("get_data_object(%.*s)\n", len, str);
    for (unsigned int i = 0; i < num_objects; i++) {
        //printf("i=%d num_obj=%d name=%s\n", i, num_objects, data_objects[i].name);
        if (strncmp(data_objects[i].name, str, len) == 0
            && strlen(data_objects[i].name) == len) {  // otherwise e.g. foo and fooBar would be recognized as equal
            return &(data_objects[i]);
        }
    }
    return NULL;
}

const data_object_t* ThingSet::get_data_object(uint16_t id)
{
    for (unsigned int i = 0; i < num_objects; i++) {
        if (data_objects[i].id == id) {
            return &(data_objects[i]);
        }
    }
    return NULL;
}
