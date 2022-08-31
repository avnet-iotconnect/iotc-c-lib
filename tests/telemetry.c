/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

// Only for malloc leak tracking. User code should NOT include cJSON
#include <stdlib.h>
#include <cJSON.h>

#include "iotconnect_lib.h"
#include "iotconnect_common.h"
#include "iotconnect_telemetry.h"

void test() {
    IotclConfig config;


    memset(&config, 0, sizeof(config));
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";
    config.device.env = "prod";
    config.telemetry.dtg = "5a913fef-428b-4a41-9927-6e0f4a1602ba";

    iotcl_init(&config);

    IotclMessageHandle msg = iotcl_telemetry_create();
    // Initial entry will be created with system timestamp
    // You can call AddWith*Time before making Set* calls in order to add a custom timestamp

    // NOTE: Do not mix epoch and ISO timestamps
    iotcl_telemetry_set_number(msg, "789", 123);
    iotcl_telemetry_set_string(msg, "boo.abc.tuv", "prs");

    // Create a new entry with different time and values
    iotcl_telemetry_add_with_iso_time(msg, iotcl_to_iso_timestamp(123457));
    iotcl_telemetry_set_number(msg, "boo.bar", 111);
    iotcl_telemetry_set_string(msg, "123", "456");
    iotcl_telemetry_set_number(msg, "789", 123.55);

    iotcl_telemetry_add_with_iso_time(msg, iotcl_iso_timestamp_now());
    iotcl_telemetry_set_null(msg, "nulltest");
    iotcl_telemetry_set_bool(msg, "booltest", true);

    const char *str = iotcl_create_serialized_string(msg, true);
    iotcl_telemetry_destroy(msg);
    printf("%s\n", str);
    iotcl_destroy_serialized(str);

    // FIXME Does this need an iotcl_deinit() -- config is no longer valid when this routine returns -- if there are multiple calls to test() 
}

static int tracker;

void *p_malloc(size_t size) {
    tracker++;
    //printf("---%d---\n", tracker);

    return malloc(size);
}

void p_free(void *ptr) {
    if (NULL != ptr) {
        tracker--;
    }
    free(ptr);
}

int main() {
    printf("---%d---\n", tracker);

    cJSON_Hooks hooks;
    hooks.malloc_fn = p_malloc;
    hooks.free_fn = p_free;
    cJSON_InitHooks(&hooks);

    tracker = 0;
    test();
    printf("---(%d)---\n", tracker);
}
