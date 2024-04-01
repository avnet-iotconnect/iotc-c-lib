/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <string.h>

#include "cJSON.h"

#include "iotcl_internal.h"
#include "iotcl_cfg.h"
#include "iotcl_log.h"
#include "iotcl_dra_url.h"
#include "iotcl_dra_discovery.h"

// NOTE: We assume that v2.1 in the API is not directly tied to the protocol version
#define IOTCL_DRA_DISCOVERY_URL_FORMAT "https://%s/api/v2.1/dsdk/cpId/%s/env/%s"

static int iotcl_dra_parse_discovery_json(IotclDraUrlContext *base_url_context, size_t base_url_slack, cJSON *json_root) {
    const char *f;

    if (!json_root) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA Discovery: Parsing error or ran out of memory while parsing response!");
        return IOTCL_ERR_PARSING_ERROR;
    }

    f = "status";
    cJSON *j_status = cJSON_GetObjectItem(json_root, f);
    if (!j_status || !cJSON_IsNumber(j_status)) goto cleanup;

    int response_status = (int) cJSON_GetNumberValue(j_status);
    if (200 != response_status) {
        IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "DRA Discovery: Received status %d! Incorrect environment name?", response_status);
        return IOTCL_ERR_BAD_VALUE;
    }

    char *message; // in case message cannot be parsed...
    f = "message";
    cJSON *j_message = cJSON_GetObjectItem(json_root, f);
    if (!j_message || !cJSON_IsString(j_message)) {
        goto cleanup;
    } else {
        message = cJSON_GetStringValue(j_message);
    }

    f = "d";
    cJSON *j_d = cJSON_GetObjectItem(json_root, f);
    if (!j_d || !cJSON_IsObject(j_d)) goto cleanup;

    f = "ec";
    cJSON *j_ec = cJSON_GetObjectItem(j_d, f);
    if (!j_ec || !cJSON_IsNumber(j_ec)) goto cleanup;
    int ec = (int) cJSON_GetNumberValue(j_ec);

#ifdef IOTCL_DRA_DISCOVERY_IGNORE_SUBSCRIPTION_EXPIRED
    // Related service ticket https://awspoc.iotconnect.io/support-info/2024031415124727
    if (3 == ec) {
        IOTCL_WARN(IOTCL_ERR_FAILED, "DRA Discovery: Received error %d! Server message was: \"%s\". Ignoring...", ec, message);
        ec = 0; // ignore this error
    }
#endif

    if (0 != ec) {
        IOTCL_ERROR(IOTCL_ERR_FAILED, "DRA Discovery: Received error %d! Server message was: \"%s\"", ec, message);
        return IOTCL_ERR_BAD_VALUE;
    }

    f = "bu";
    cJSON *j_bu = cJSON_GetObjectItem(j_d, f);
    if (!j_bu || !cJSON_IsString(j_bu)) goto cleanup;

    int status = iotcl_dra_url_init_with_slack(base_url_context, base_url_slack, cJSON_GetStringValue(j_bu));
    if (IOTCL_SUCCESS != status) {
        // the called function will print the error, but we need to be more specific, though return the original cause
        IOTCL_ERROR(IOTCL_ERR_FAILED, "DRA: Unable to initialize base URL from discovery response!");
        return status;
    }

    return IOTCL_SUCCESS;

    cleanup:
    IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA: Error encountered while parsing the discovery response field \"%s\"", f);
    return IOTCL_ERR_PARSING_ERROR;
}

int iotcl_dra_discovery_init_url_with_host(IotclDraUrlContext *c, char *host, const char *cpid, const char *env) {
    if (!host || !cpid || !env || 0 == strlen(host) || 0 == strlen(cpid) || 0 == strlen(env)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA: Host, cpid and env arguments are required.");
        return IOTCL_ERR_MISSING_VALUE;
    }
    size_t size = (size_t) snprintf(NULL, 0, IOTCL_DRA_DISCOVERY_URL_FORMAT, host, cpid, env);
    char *disc_url = iotcl_malloc(size + 1);
    if (!disc_url) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA: Out of memory while allocating the URL buffer!");
        return IOTCL_ERR_OUT_OF_MEMORY;
    }
    sprintf(disc_url, IOTCL_DRA_DISCOVERY_URL_FORMAT, host, cpid, env);
    iotcl_dra_url_init(c, disc_url);
    iotcl_free(disc_url);
    return IOTCL_SUCCESS;
}

int iotcl_dra_discovery_init_url_aws(IotclDraUrlContext *c, const char *cpid, const char *env) {
    return iotcl_dra_discovery_init_url_with_host(c, IOTCL_DRA_DEFAULT_DISCOVERY_HOST_AWS, cpid, env);
}

int iotcl_dra_discovery_init_url_azure(IotclDraUrlContext *c, const char *cpid, const char *env) {
    return iotcl_dra_discovery_init_url_with_host(c, IOTCL_DRA_DEFAULT_DISCOVERY_HOST_AZURE, cpid, env);
}

int iotcl_dra_discovery_parse(IotclDraUrlContext *c, int base_url_slack, const char *response_str) {
    cJSON *root = cJSON_Parse(response_str);
    int status = iotcl_dra_parse_discovery_json(c, (size_t) base_url_slack, root);
    cJSON_Delete(root);
    return status;
}

int iotcl_dra_discovery_parse_with_length(IotclDraUrlContext *c, int base_url_slack, const uint8_t *response_data, size_t response_data_size) {
    cJSON *root = cJSON_ParseWithLength((const char *)response_data, response_data_size);
    int status = iotcl_dra_parse_discovery_json(c, (size_t) base_url_slack, root);
    cJSON_Delete(root);
    return status;
}
