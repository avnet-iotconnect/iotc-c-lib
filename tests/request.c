/* Copyright (C) 2021 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

// Only for malloc leak tracking. User code should NOT include cJSON
#include <cJSON.h>

#include "iotconnect_lib.h"
#include "iotconnect_event.h"
#include "iotconnect_request.h"

const char *TEST_EVENT_JSON_HELLO_RESPONSE = "{\"d\":{\"ec\":0,\"ct\":200,\"meta\":{\"dtg\":\"12aca123-9690-4a98-a5df-0e544bc4c7bf\",\"g\":\"01398123-60c8-47d0-950d-1425619eb370\",\"at\":1,\"tg\":\"\",\"edge\":0,\"df\":60,\"eg\":\"6123456-a0db-4f1f-ae66-754618ecd374\",\"pf\":1,\"hwv\":\"\",\"swv\":\"\"},\"has\":{\"d\":0,\"attr\":1,\"set\":0,\"r\":0,\"ota\":0}}}";

void on_response(IotclEventData data, IotclEventType event_type) {
    switch (event_type) {
        case REQ_HELLO:
        {
            const char *dtg = iotcl_clone_response_dtg(data);
            if (NULL != dtg) {
                printf("Hello response DTG is: %s\n", dtg);
                free((void *) dtg);
            }
        }
            break;
        case REQ_GET_ATTRIBUTES:
            printf("Got attributes\n");
            break;
        case REQ_GET_DEVICE_SETTINGS:
            printf("Got device settings\n");
            break;
        case REQ_RULES:
            printf("Got rules\n");
            break;
        default:
            printf("Got unknown request\n");
            break;
    }
}

void test() {
    IotclConfig config;


    memset(&config, 0, sizeof(config));
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";
    config.device.env = "prod";
    config.request.sid = "AB1234CAD34FD=12FCA=";

    config.event_functions.response_cb = on_response;

    iotcl_init(&config);

    char *str = iotcl_request_create_hello();
    printf("%s\n", str);
    iotcl_request_destroy(str);
    iotcl_process_event(TEST_EVENT_JSON_HELLO_RESPONSE);
}


static int tracker;

void *p_malloc(size_t size) {
    tracker++;
    //printf("---%d---\n", tracker);

    void*  ret = malloc(size);
    //printf("%08x(%d)++\n", (unsigned int)ret, tracker);
    return ret;
}

void p_free(void *ptr) {
    if (NULL != ptr) {
        tracker--;
    }
    //printf("---%d---\n", tracker);
    //printf("%08x---\n", (unsigned int)ptr);
    free(ptr);
}

int main() {
    printf("---%d---\n", tracker);
    tracker = 0;

    cJSON_Hooks hooks;
    hooks.malloc_fn = p_malloc;
    hooks.free_fn = p_free;
    cJSON_InitHooks(&hooks);

    test();
    printf("---(%d)---\n", tracker);
    if (0 != tracker) {
        return -1;
    }
}
