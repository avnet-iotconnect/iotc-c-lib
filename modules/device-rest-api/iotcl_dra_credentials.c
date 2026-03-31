/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

#include "cJSON.h"


#include "iotcl_cfg.h"
#include "iotcl_log.h"
#include "iotcl_util.h"
#include "iotcl_internal.h"
#include "iotcl_dra_credentials.h"
#include "iotcl.h"

static int iotcl_dra_parse_credentials_json(IotclDraCredentialsResult *result, cJSON *json_root) {
    if (!json_root) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA JSON Credentials: Parsing error or ran out of memory while parsing!");
        return IOTCL_ERR_PARSING_ERROR;
    }

    char *f = "message";
    cJSON *j_d = cJSON_GetObjectItem(json_root, f);
    if (j_d) {
        char *message = cJSON_GetStringValue(j_d);
        IOTCL_ERROR(IOTCL_ERR_FAILED, "DRA JSON Credentials: Received error message from server: \"%s\"", message ? message : "(null)");
        return IOTCL_ERR_FAILED;
    }

    f = "credentials";
    cJSON *j_credentials = cJSON_GetObjectItem(json_root, f);
    if (!j_credentials || !cJSON_IsObject(j_credentials)) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA JSON Credentials: Missing or invalid \"credentials\" object in JSON!");
        return IOTCL_ERR_PARSING_ERROR;
    }

    char *access_key_id = get_json_string(j_credentials, "accessKeyId");
    char *secret_access_key = get_json_string(j_credentials, "secretAccessKey");
    char *session_token = get_json_string(j_credentials, "sessionToken");
    char *expiration_str = get_json_string(j_credentials, "expiration");
    if (!expiration_str) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA JSON Credentials: Missing or invalid \"expiration\" field in credentials!");
        return IOTCL_ERR_PARSING_ERROR;
    }
    // Parse the expiration string like "2016-03-15T00:05:07Z"
    result->expiration = iotcl_iso8601_basic_to_epoch_utc_time(expiration_str);
    if (result->expiration == 0) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA JSON Credentials: Failed to parse \"expiration\" field in credentials!");
        return IOTCL_ERR_PARSING_ERROR;
    }

    if (!access_key_id || !secret_access_key || !session_token) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA JSON Credentials: accessKeyId, secretAccessKey, sessionToken, and expiration are required and cannot be empty");
        return IOTCL_ERR_MISSING_VALUE;
    }

    result->access_key_id = iotcl_strdup(access_key_id);
    result->secret_access_key = iotcl_strdup(secret_access_key);
    result->session_token = iotcl_strdup(session_token);
    result->expiration_str = iotcl_strdup(expiration_str);
    if (!result->access_key_id || !result->secret_access_key || !result->session_token || !result->expiration_str) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA JSON Credentials: Out of memory while allocating result strings");
        iotcl_dra_json_credentials_free(result);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}

int iotcl_dra_json_credentials_parse(IotclDraCredentialsResult *credentials, const char *json_str) {
    if (!credentials || !json_str) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA JSON Credentials: credentials and json_str arguments are required");
        return IOTCL_ERR_MISSING_VALUE;
    }

    memset(credentials, 0, sizeof(IotclDraCredentialsResult));

    cJSON *root = cJSON_Parse(json_str);
    int status = iotcl_dra_parse_credentials_json(credentials, root);
    cJSON_Delete(root);
    return status;
}

void iotcl_dra_json_credentials_free(IotclDraCredentialsResult *json_config) {
    if (!json_config) {
        return;
    }
    if (json_config->access_key_id) { iotcl_free(json_config->access_key_id); json_config->access_key_id = NULL; }
    if (json_config->secret_access_key) { iotcl_free(json_config->secret_access_key); json_config->secret_access_key = NULL; }
    if (json_config->session_token) { iotcl_free(json_config->session_token); json_config->session_token = NULL; }
    if (json_config->expiration_str) { iotcl_free(json_config->expiration_str); json_config->expiration_str = NULL; }
    json_config->expiration = 0 ;
}
