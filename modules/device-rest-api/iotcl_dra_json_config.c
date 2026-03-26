/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

#include "cJSON.h"


#include "iotcl_cfg.h"
#include "iotcl_log.h"
#include "iotcl_util.h"
#include "iotcl_dra_json_config.h"
#include "iotcl.h"

// Helper: get non-empty string from JSON or return NULL
static char *get_json_string(cJSON *root, const char *field) {
    cJSON *item = cJSON_GetObjectItem(root, field);
    if (!item || !cJSON_IsString(item)) return NULL;
    char *val = cJSON_GetStringValue(item);
    return (val && strlen(val) > 0) ? val : NULL;
}

static int iotcl_dra_parse_json_config(IotclDraJsonConfigResult *result, cJSON *json_root) {
    if (!json_root) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA JSON Config: Parsing error or ran out of memory while parsing!");
        return IOTCL_ERR_PARSING_ERROR;
    }


    // Parse and validate platform
    char *pf = get_json_string(json_root, "pf");
    if (!pf || (strcmp(pf, IOTCL_PF_AWS_STR) != 0 && strcmp(pf, IOTCL_PF_AZURE_STR) != 0)) {
        IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "DRA JSON Config: Invalid platform \"%s\". Expected \"%s\" or \"%s\"", 
                    pf ? pf : "(null)", IOTCL_PF_AWS_STR, IOTCL_PF_AZURE_STR);
        return IOTCL_ERR_BAD_VALUE;
    }

    char *ver = get_json_string(json_root, "ver");
    char *cpid = get_json_string(json_root, "cpid");
    char *env = get_json_string(json_root, "env");
    char *uid = get_json_string(json_root, "uid");
    char *did = get_json_string(json_root, "did");

    if (!cpid || !env || !uid || !did) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA JSON Config: cpid, env, uid, and did are required and cannot be empty");
        return IOTCL_ERR_MISSING_VALUE;
    }

    // ver is only used for sanity checks
    if (ver && strcmp(ver, "2.1") != 0) {
        IOTCL_WARN(IOTCL_ERR_BAD_VALUE, "DRA JSON Config: Expected protocol version \"%s\", got \"%s\"", "2.1", ver ? ver : "(null)");
    }

    // Allocate and copy values into result
    result->pf = iotcl_strdup(pf);
    result->cpid = iotcl_strdup(cpid);
    result->env = iotcl_strdup(env);
    result->duid = iotcl_strdup(uid);
    result->client_id = iotcl_strdup(did);
    result->dedicated_instance = (strlen(uid) == strlen(did));

    if (!result->pf || !result->cpid || !result->env || !result->duid || !result->client_id) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA JSON Config: Out of memory while allocating result strings");
        iotcl_dra_json_config_free(result);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}

int iotcl_dra_json_config_parse(IotclDraJsonConfigResult *json_config, const char *json_str) {
    if (!json_config || !json_str) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA JSON Config: json_config and json_str arguments are required");
        return IOTCL_ERR_MISSING_VALUE;
    }

    memset(json_config, 0, sizeof(IotclDraJsonConfigResult));

    cJSON *root = cJSON_Parse(json_str);
    int status = iotcl_dra_parse_json_config(json_config, root);
    cJSON_Delete(root);
    return status;
}

void iotcl_dra_json_config_free(IotclDraJsonConfigResult *json_config) {
    if (!json_config) {
        return;
    }
    if (json_config->pf) { iotcl_free(json_config->pf); json_config->pf = NULL; }
    if (json_config->cpid) { iotcl_free(json_config->cpid); json_config->cpid = NULL; }
    if (json_config->env) { iotcl_free(json_config->env); json_config->env = NULL; }
    if (json_config->duid) { iotcl_free(json_config->duid); json_config->duid = NULL; }
    if (json_config->client_id) { iotcl_free(json_config->client_id); json_config->client_id = NULL; }
    json_config->dedicated_instance = false;
}