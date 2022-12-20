/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al & Neerav Parasher <neerav.parasar@softwebsolutions.com>.
 */

#include <stdbool.h>
#include <stdlib.h>

#include <string.h>

#include "cJSON.h"

#include "iotconnect_common.h"
#include "iotconnect_discovery.h"

static char *safe_get_string_and_strdup(cJSON *cjson, const char *value_name) {
    cJSON *value = cJSON_GetObjectItem(cjson, value_name);
    if (!value) {
        return NULL;
    }
    const char *str_value = cJSON_GetStringValue(value);
    if (!str_value) {
        return NULL;
    }
    return iotcl_strdup(str_value);
}

static bool split_url(IotclDiscoveryResponse *response) {
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
    cJSON *base_url_cjson;
    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        return NULL;
    }
    char *root_data = cJSON_GetObjectItem(json_root, "d");//edit
    if (!root_data) {
        cJSON_Delete(json_root);
        return NULL;
    }
    if (cJSON_IsObject(root_data)) {
        base_url_cjson = cJSON_GetObjectItem(root_data, "bu");
    }

    IotclDiscoveryResponse *response = (IotclDiscoveryResponse *) calloc(1, sizeof(IotclDiscoveryResponse));
    if (!response) {
        goto cleanup;
    }

    { // separate the declaration into a block to allow jump without warnings
        char *jsonBaseUrl = base_url_cjson->valuestring;
        if (!jsonBaseUrl) {
            goto cleanup;
        }

        response->url = iotcl_strdup(jsonBaseUrl);
        if (split_url(response)) {
            cJSON_Delete(json_root);
            return response;
        } // else cleanup and return null

    }

    cleanup:
    cJSON_Delete(json_root);
    iotcl_discovery_free_discovery_response(response);
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

IotclSyncResponse *iotcl_discovery_parse_sync_response(const char *response_data) {
    cJSON *tmp_value = NULL;
    IotclSyncResponse *response = (IotclSyncResponse *) calloc(1, sizeof(IotclSyncResponse));
    if (NULL == response) {
        return NULL;
    }

    cJSON *sync_json_root = cJSON_Parse(response_data);
    if (!sync_json_root) {
        response->ec = IOTCL_SR_PARSING_ERROR;
        return response;
    }
    cJSON *sync_res_json = cJSON_GetObjectItemCaseSensitive(sync_json_root, "d");
    if (!sync_res_json) {
        cJSON_Delete(sync_json_root);
        response->ec = IOTCL_SR_PARSING_ERROR;
        return response;
    }
    tmp_value = cJSON_GetObjectItem(sync_res_json, "ec");
    if (!tmp_value) {
        response->ec = IOTCL_SR_PARSING_ERROR;
    }
    else {
        response->ec = cJSON_GetNumberValue(tmp_value);
    }
    if (response->ec == IOTCL_SR_OK) {

        cJSON *meta = cJSON_GetObjectItemCaseSensitive(sync_res_json, "meta");
        if (meta) {
            cJSON *df = cJSON_GetObjectItemCaseSensitive(meta, "df");
            if (df) {
                response->meta.df = cJSON_GetNumberValue(df);
            }
            cJSON *cd = cJSON_GetObjectItemCaseSensitive(meta, "cd");
            if (cd) {
                response->meta.cd = safe_get_string_and_strdup(meta, "cd");
            }
            cJSON *v = cJSON_GetObjectItemCaseSensitive(meta, "v");
            if (v) {
                response->meta.v = cJSON_GetNumberValue(v);
            }
            cJSON *edge = cJSON_GetObjectItemCaseSensitive(meta, "edge");
            if (edge) {
                response->meta.edge = edge->valueint;
            }
            cJSON *gtw = cJSON_GetObjectItemCaseSensitive(meta, "gtw");
            if (safe_get_string_and_strdup(gtw, "tg")) {
                response->meta.tg = safe_get_string_and_strdup(gtw, "tg");//String represents the gateway Tag
                response->meta.g = safe_get_string_and_strdup(gtw, "g");//GUID represents the device GUID
            }
            else {
                response->meta.tg = "";
                response->meta.g = "";
            }
        }
        cJSON *has = cJSON_GetObjectItemCaseSensitive(sync_res_json, "has");
        if (has) {
            cJSON *d = cJSON_GetObjectItemCaseSensitive(has, "d");
            if (d) {
                response->has.d = cJSON_GetNumberValue(d);
            }
            cJSON *attr = cJSON_GetObjectItemCaseSensitive(has, "attr");
            if (attr) {
                response->has.attr = cJSON_GetNumberValue(attr);
            }
            cJSON *set = cJSON_GetObjectItemCaseSensitive(has, "set");
            if (set) {
                response->has.set = cJSON_GetNumberValue(set);
            }
        }
        cJSON *p = cJSON_GetObjectItemCaseSensitive(sync_res_json, "p");
        if (p) {
            response->broker.name = safe_get_string_and_strdup(p, "n");
            response->broker.client_id = safe_get_string_and_strdup(p, "id");
            response->broker.host = safe_get_string_and_strdup(p, "h");
            response->broker.user_name = safe_get_string_and_strdup(p, "un");
            response->broker.pass = safe_get_string_and_strdup(p, "pwd");
            cJSON* topics = cJSON_GetObjectItemCaseSensitive(p, "topics");
            if (topics) {
                response->broker.pub_topic = safe_get_string_and_strdup(topics, "rpt");
                response->broker.ack_pub_topic = safe_get_string_and_strdup(topics, "ack");
                response->broker.di_pub = safe_get_string_and_strdup(topics, "di");
                response->broker.hb_pub = safe_get_string_and_strdup(topics, "hb");
                response->broker.sub_topic = safe_get_string_and_strdup(topics, "c2d");
            }
            if (
                !response->broker.host ||
                !response->broker.name ||
                !response->broker.client_id ||
                !response->broker.user_name ||
                !response->broker.sub_topic ||
                !response->broker.di_pub ||
                !response->broker.ack_pub_topic ||
                !response->broker.hb_pub ||
                // !response->broker.pass || << password may actually be null or empty
                !response->broker.pub_topic
                ) {
                // Assume parsing eror, but it could alo be (unlikely) allocation error
                response->ec = IOTCL_SR_PARSING_ERROR;
            }
        } else {
            response->ec = IOTCL_SR_PARSING_ERROR;
        }
    } else {
        switch (response->ec) {
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
                response->ec = IOTCL_SR_UNKNOWN_DEVICE_STATUS;
                break;
        }
    }
    // we have duplicated strings, so we can now free the result
    cJSON_Delete(sync_json_root);
    return response;
}

void iotcl_discovery_free_sync_response(IotclSyncResponse *response) {
    if (!response) {
        return;
    }
    free(response->broker.host);
    free(response->broker.client_id);
    free(response->broker.user_name);
    free(response->broker.pass);
    free(response->broker.name);
    free(response->broker.sub_topic);
    free(response->broker.pub_topic);
    free(response->broker.ack_pub_topic);
    free(response->broker.hb_pub);
    free(response->broker.di_pub);
    free(response->meta.tg);
    free(response->meta.g);
    free(response->meta.gtw);
    free(response);
}

