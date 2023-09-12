//
// Copyright: Avnet, Softweb Inc. 2022
// Created by Nik Markovic <nikola.markovic@avnet.com> on 5/15/22.
//

#include <stddef.h>
#include <stdlib.h>
#include "cJSON.h"
#include "iotconnect_common.h"
#include "iotconnect_discovery_v3.h"

IotclDiscoveryV3Response * iotcl_parse_discovery_v3_response(const char* response_data) {
	IotclDiscoveryV3Response *response = (IotclDiscoveryV3Response *) calloc(1, sizeof(IotclDiscoveryV3Response));
    if (NULL == response) {
        return NULL;
    }
    response->ec = -1;
    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        response->message = iotcl_strdup("IOTCL Parsing Error");
        cJSON_Delete(json_root);
        return response;
    }
    cJSON *json_data = cJSON_GetObjectItem(json_root, "d");
    if (!json_data) {
    	response->message = iotcl_strdup("IOTCL: Discovery data object is missing");
        cJSON_Delete(json_root);
    	return response;
    }
    response->ec = cJSON_GetNumberValue(cJSON_GetObjectItem(json_data, "ec"));
    response->bu = safe_get_string_and_strdup(json_data, "bu");
    if (!response->bu) {
        response->message = iotcl_strdup("IOTCL: Discovery base URL is NULL. Out of memory?");
        cJSON_Delete(json_root);
        return response;
    }
    response->message = safe_get_string_and_strdup(json_root, "message");

    // we have duplicated strings, so we can now free the result
    cJSON_Delete(json_root);
    return response;
}

void iotcl_free_discovery_v3_response(IotclDiscoveryV3Response *response) {
	if (response) {
		free(response->bu);
		free(response->message);
		free(response);
	}
}
