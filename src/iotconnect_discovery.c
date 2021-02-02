/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdbool.h>
#include <stdlib.h>

#include <string.h>
#include <cJSON.h>

#include "iotconnect_common.h"
#include "iotconnect_discovery.h"


void c(const char* p) {
    cJSON* root = cJSON_Parse(p);
    char *jsonBaseUrl = (cJSON_GetObjectItem(root, "baseUrl"))->valuestring;
    char * base_url = (char*)malloc(strlen(jsonBaseUrl) + 1);
    strcpy(base_url, jsonBaseUrl);
}

static char *safe_get_string_and_strdup(cJSON *cjson, const char* value_name) {
    cJSON *value = cJSON_GetObjectItem(cjson, value_name);
    if (!value) {
        return NULL;
    }
    const char *str_value = cJSON_GetStringValue(value);
    if (!str_value) {
        return NULL;
    }
    return IOTCL_Strdup(str_value);
}

static bool split_url(IOTCL_DiscoveryResponse *response) {
    size_t base_url_len = strlen(response->url);


    // mutable version that will allow us to modify the url string
    char *base_url_copy = IOTCL_Strdup(response->url);
    if (!base_url_copy) {
        return false;
    }
    int num_found = 0;
    char * host = NULL;
    for (size_t i = 0; i < base_url_len; i++) {
        if (base_url_copy[i] == '/') {
            num_found++;
            if (num_found == 2) {
                host = &base_url_copy[i + 1];
                // host will be terminated below
            } else if (num_found == 3) {
                response->path = IOTCL_Strdup(&base_url_copy[i]); // first make a copy
                base_url_copy[i] = 0; // then terminate host so that it can be duped below
                break;
            }
        }
    }
    response->host = IOTCL_Strdup(host);
    free(base_url_copy);

    return (response->host && response->path);
}

IOTCL_DiscoveryResponse *IOTCL_DiscoveryParseDiscoveryResponse(const char *response_data) {
    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        return  NULL;
    }

    cJSON *base_url_cjson = cJSON_GetObjectItem(json_root, "baseUrl");
    if (!base_url_cjson) {
        cJSON_Delete(json_root);
        return NULL;
    }

    IOTCL_DiscoveryResponse *response = (IOTCL_DiscoveryResponse*)calloc(1, sizeof(IOTCL_DiscoveryResponse));
    if (!response) {
        goto cleanup;
    }

    { // separate the declaration into a block to allow jump without warnings
        char *jsonBaseUrl = base_url_cjson->valuestring;
        if (!jsonBaseUrl) {
            goto cleanup;
        }

        response->url = IOTCL_Strdup(jsonBaseUrl);
        if (split_url(response)) {
            cJSON_Delete(json_root);
            return response;
        } // else cleanup and return null

    }

    cleanup:
    cJSON_Delete(json_root);
    IOTCL_DiscoveryFreeDiscoveryResponse(response);
    return NULL;
}

void IOTCL_DiscoveryFreeDiscoveryResponse(IOTCL_DiscoveryResponse *response) {
    if (response) {
        free(response->url);
        free(response->host);
        free(response->path);
        free(response);
    }
}

IOTCL_SyncResponse *IOTCL_DiscoveryParseSyncResponse(const char *response_data) {
    cJSON* tmp_value = NULL;
    IOTCL_SyncResponse *response = (IOTCL_SyncResponse*)calloc(1, sizeof(IOTCL_SyncResponse));
    if (NULL == response) {
        return NULL;
    }

    cJSON *sync_json_root = cJSON_Parse(response_data);
    if (!sync_json_root) {
        response->ds = IOTCL_SR_PARSING_ERROR;
        return response;
    }
    cJSON *sync_res_json = cJSON_GetObjectItemCaseSensitive(sync_json_root, "d");
    if (!sync_res_json) {
        cJSON_Delete(sync_json_root);
        response->ds = IOTCL_SR_PARSING_ERROR;
        return response;
    }
    tmp_value = cJSON_GetObjectItem(sync_res_json, "ds");
    if (!tmp_value) {
        response->ds = IOTCL_SR_PARSING_ERROR;
    } else {
        response->ds = cJSON_GetNumberValue(tmp_value);
    }
    if (response->ds == IOTCL_SR_OK) {
        response->cpid = safe_get_string_and_strdup(sync_res_json, "cpId");
        response->dtg = safe_get_string_and_strdup(sync_res_json, "dtg");
        tmp_value = cJSON_GetObjectItem(sync_res_json, "ee");
        if (!tmp_value) {
            response->ee = -1;
        }
        tmp_value = cJSON_GetObjectItem(sync_res_json, "rc");
        if (!tmp_value) {
            response->rc = -1;
        }
        tmp_value = cJSON_GetObjectItem(sync_res_json, "at");
        if (!tmp_value) {
            response->at = -1;
        }
        cJSON *p = cJSON_GetObjectItemCaseSensitive(sync_res_json, "p");
        if (p) {
            response->broker.name = safe_get_string_and_strdup(p, "n");
            response->broker.client_id = safe_get_string_and_strdup(p, "id");
            response->broker.host = safe_get_string_and_strdup(p, "h");
            response->broker.user_name = safe_get_string_and_strdup(p, "un");
            response->broker.pass = safe_get_string_and_strdup(p, "pwd");
            response->broker.sub_topic = safe_get_string_and_strdup(p, "sub");
            response->broker.pub_topic = safe_get_string_and_strdup(p, "pub");
            if (
                    !response->cpid ||
                    !response->dtg ||
                    !response->broker.host ||
                    !response->broker.client_id ||
                    !response->broker.user_name ||
                    // !response->broker.pass || << password may actually be null or empty
                    !response->broker.sub_topic ||
                    !response->broker.pub_topic
                    ) {
                // Assume parsing eror, but it could alo be (unlikely) allocation error
                response->ds = IOTCL_SR_PARSING_ERROR;
            }
        } else {
            response->ds = IOTCL_SR_PARSING_ERROR;
        }
    } else {
        switch (response->ds) {
            case IOTCL_SR_DEVICE_NOT_REGISTERED:
            case IOTCL_SR_UNKNOWN_DEVICE_STATUS:
            case IOTCL_SR_AUTO_REGISTER:
            case IOTCL_SR_DEVICE_NOT_FOUND:
            case IOTCL_SR_DEVICE_INACTIVE:
            case IOTCL_SR_DEVICE_MOVED:
            case IOTCL_SR_CPID_NOT_FOUND:
                // all fall through
                break;
            default:
                response->ds = IOTCL_SR_UNKNOWN_DEVICE_STATUS;
                break;
        }
    }
    // we have duplicated strings, so we can now free the result
    free(sync_json_root);
    return response;
}

void IOTCL_DiscoveryFreeSyncResponse(IOTCL_SyncResponse *response) {
    if (!response) {
        return;
    }
    free(response->cpid);
    free(response->dtg);
    free(response->broker.host);
    free(response->broker.client_id);
    free(response->broker.user_name);
    free(response->broker.pass);
    free(response->broker.name);
    free(response->broker.sub_topic);
    free(response->broker.pub_topic);
    free(response);
    memset(response, 0, sizeof(IOTCL_SyncResponse));
}

