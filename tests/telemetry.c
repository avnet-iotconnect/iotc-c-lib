/* Copyright (C) 2020 Avnet - All Rights Reserved
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
#include "iotconnect_common.h"
#include "iotconnect_telemetry.h"

void test() {
    IOTCL_CONFIG config;


    memset(&config, 0, sizeof(config));
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";
    config.device.env = "prod";
    config.telemetry.dtg = "5a913fef-428b-4a41-9927-6e0f4a1602ba";

    IOTCL_Init(&config);

    IOTCL_MESSAGE_HANDLE msg = IOTCL_TelemetryCreate();
    // Initial entry will be created with system timestamp
    // You can call AddWith*Time before making Set* calls in order to add a custom timestamp

    // NOTE: Do not mix epoch and ISO timestamps
    IOTCL_TelemetrySetNumber(msg, "789", 123);
    IOTCL_TelemetrySetString(msg, "boo.abc.tuv", "prs");

    // Create a new entry with different time and values
    IOTCL_TelemetryAddWithIsoTime(msg, IOTCL_ToIsoTimestamp(123457));
    IOTCL_TelemetrySetNumber(msg, "boo.bar", 111);
    IOTCL_TelemetrySetString(msg, "123", "456");
    IOTCL_TelemetrySetNumber(msg, "789", 123.55);

    IOTCL_TelemetryAddWithIsoTime(msg, IOTCL_IsoTimestampNow());
    IOTCL_TelemetrySetNull(msg, "nulltest");
    IOTCL_TelemetrySetBool(msg, "booltest", true);

    const char *str = IOTCL_CreateSerializedString(msg, true);
    IOTCL_TelemetryDestroy(msg);
    printf("%s\n", str);
    IOTCL_DestroySerialized(str);
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
