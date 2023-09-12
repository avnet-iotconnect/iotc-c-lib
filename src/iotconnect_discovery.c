/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "cJSON.h"
#include "iotconnect_common.h"
#include "iotconnect_discovery.h"

static bool split_url(IotclDiscoveryResponse *response) {
    printf("%s ->\n", __func__);

    size_t base_url_len = strlen(response->url);

    // mutable version that will allow us to modify the url string
    char *base_url_copy = iotcl_strdup(response->url);
    if (!base_url_copy) {
        return false;
    }
    int num_found = 0;
    char *host = NULL;
    for (size_t i = 0; i < base_url_len; i++) {
        if (base_url_copy[i] == '/') {
            num_found++;
            if (num_found == 2) {
                host = &base_url_copy[i + 1];
                // host will be terminated below
            } else if (num_found == 3) {
                response->path = iotcl_strdup(&base_url_copy[i]); // first make a copy
                base_url_copy[i] = 0; // then terminate host so that it can be duped below
                break;
            }
        }
    }
    response->host = iotcl_strdup(host);
    free(base_url_copy);

    return (response->host && response->path);
}

IotclDiscoveryResponse *iotcl_discovery_parse_discovery_response(const char *response_data) {
    printf("%s ->\n", __func__);

    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        printf("%s: !json_root\n", __func__);
        return NULL;
    }

    cJSON *base_url_cjson = cJSON_GetObjectItem(json_root, "baseUrl");
    if (!base_url_cjson) {
        cJSON *d_cjson = cJSON_GetObjectItem(json_root, "d");
        if (!d_cjson) {
            // neither exist
            printf("%s: !d_cjson\n", __func__);
            cJSON_Delete(json_root);
            return NULL;
        }
        cJSON *bu_cjson = cJSON_GetObjectItem(d_cjson, "bu");
        if (!bu_cjson) {
            // neither exist
            printf("%s: !base_url_cjson && !bu_cjson\n", __func__);
            cJSON_Delete(json_root);
            return NULL;
        }

        // swap
        base_url_cjson = bu_cjson;
        bu_cjson = NULL;
    } 

    IotclDiscoveryResponse *response = (IotclDiscoveryResponse *) calloc(1, sizeof(IotclDiscoveryResponse));
    if (!response) {
        printf("%s: !response\n", __func__);
        goto cleanup;
    }

    { // separate the declaration into a block to allow jump without warnings
        char *jsonBaseUrl = base_url_cjson->valuestring;
        if (!jsonBaseUrl) {
            printf("%s: !jsonBaseUrl\n", __func__);
            goto cleanup;
        }

        response->url = iotcl_strdup(jsonBaseUrl);
        if (split_url(response)) {
            printf("%s: split_url ok\n", __func__);
            cJSON_Delete(json_root);
            return response;
        } // else cleanup and return null

        printf("%s: split_url failed\n", __func__);
    }

    cleanup:
    printf("%s: failed\n", __func__);

    cJSON_Delete(json_root);
    iotcl_discovery_free_discovery_response(response);

    printf("%s <-\n", __func__);

    return NULL;
}

void iotcl_discovery_free_discovery_response(IotclDiscoveryResponse *response) {
    if (response) {
        free(response->url);
        free(response->host);
        free(response->path);
        free(response);
    }
}

int iotcl_discovery_parse_sync_response(const char *response_data, IotclSyncResponse **presponse)
{
    IotclSyncResponse *response = NULL;
    cJSON *sync_json_root = NULL;
    *presponse = NULL;

    response = (IotclSyncResponse *) calloc(1, sizeof(IotclSyncResponse));
    if (NULL == response) {
        printf("%s: NULL == response\n", __func__);
        goto parsing_error;
    }

    sync_json_root = cJSON_Parse(response_data);
    if (!sync_json_root) {
        printf("%s: !sync_json_root\n", __func__);
        goto parsing_error;
    }
    cJSON *sync_res_json = cJSON_GetObjectItemCaseSensitive(sync_json_root, "d");
    if (!sync_res_json) {
        printf("%s: !sync_res_json\n", __func__);
        goto parsing_error;
    }

    if(safe_get_integer(sync_res_json, "ec", &response->ec) != 0) {
        goto parsing_error;
    }

#if 0
    switch (response->ec) {
        case IOTCL_SR_OK:
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
#endif

    if(safe_get_integer(sync_res_json, "ct", &response->ct) != 0) {
        goto parsing_error;
    }

    cJSON *meta = cJSON_GetObjectItemCaseSensitive(sync_res_json, "meta");
    if (!meta) {
            goto parsing_error;
    }
    if(safe_get_integer(meta, "at", &response->meta.at) != 0) {
        goto parsing_error;
    }
    response->meta.cd = safe_get_string_and_strdup(meta, "cd");

    cJSON *p = cJSON_GetObjectItemCaseSensitive(sync_res_json, "p");
    if (!p) {
            goto parsing_error;
    }
    response->broker.name = safe_get_string_and_strdup(p, "n");
    response->broker.client_id = safe_get_string_and_strdup(p, "id");
    response->broker.host = safe_get_string_and_strdup(p, "h");
    response->broker.user_name = safe_get_string_and_strdup(p, "un");
    response->broker.pass = safe_get_string_and_strdup(p, "pwd");
    // Note: !response->broker.pass || << password may actually be null or empty
    if ( !response->meta.cd || !response->broker.host || !response->broker.client_id || !response->broker.user_name ) {
            goto parsing_error;
    }
    cJSON *topics = cJSON_GetObjectItemCaseSensitive(p, "topics");
    if (!topics) {
            goto parsing_error;
    }
    response->broker.topics.sub_topic = safe_get_string_and_strdup(topics, "c2d");
    response->broker.topics.pub_topic = safe_get_string_and_strdup(topics, "rpt");
    if ( !response->broker.topics.sub_topic || !response->broker.topics.pub_topic) {
            goto parsing_error;
    }

    // we have duplicated strings, so we can now free the result
    cJSON_Delete(sync_json_root);
    *presponse = response;

    return 0;

parsing_error:
    // Assume parsing eror, but it could alo be (unlikely) allocation error
    if(sync_json_root) {
        cJSON_Delete(sync_json_root);
    }
    iotcl_discovery_free_sync_response(response);
    return -1;
}

void iotcl_discovery_free_sync_response(IotclSyncResponse *response) {
    if (!response) {
        return;
    }
    free(response->meta.cd);
    free(response->broker.host);
    free(response->broker.client_id);
    free(response->broker.user_name);
    free(response->broker.pass);
    free(response->broker.name);
    free(response->broker.topics.sub_topic);
    free(response->broker.topics.pub_topic);
    free(response);
}

