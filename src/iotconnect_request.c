char* iotcl_request_create_hello(void);
/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stddef.h>
#include <cJSON.h>

#include "iotconnect_lib.h"
#include "iotconnect_event.h"

#include "iotconnect_request.h"

char* iotcl_request_create_request(IotclEventType requestEventType) {
    char * ret = NULL;
    cJSON *root_value = NULL;
    IotclConfig *config = iotcl_get_config();
    if (!config) return NULL;
    if (!config->request.sid) return NULL;
    root_value = cJSON_CreateObject();

    if (!cJSON_AddStringToObject(root_value, "sid", config->request.sid)) goto cleanup;
    if (!cJSON_AddNumberToObject(root_value, "v", 2)) goto cleanup;
    if (!cJSON_AddNumberToObject(root_value, "mt", requestEventType )) goto cleanup; // telemetry message type (zero)

    ret = cJSON_Print(root_value);

    // ret could be null, but fall through and it will be handled correctly

cleanup:
    if (root_value) {
        // we can free root value always
        cJSON_Delete(root_value);
    }
    return ret;
}

char* iotcl_request_create_hello(void) {
    return iotcl_request_create_request(REQ_HELLO);
}

void iotcl_request_destroy(char *serialized_string) {
    cJSON_free((void *)serialized_string); // use cJSON_free so that we can track mallocs
}
