/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <string.h>
#include "cJSON.h"
#include "iotcl_util.h"
#include "iotcl_internal.h"

char *iotcl_strdup_json_string(cJSON *cjson, const char *value_name) {
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

int iotcl_strdup_json_string_if_exists(char** result, cJSON *cjson, const char *value_name) {
    *result = NULL;
    cJSON *value = cJSON_GetObjectItem(cjson, value_name);
    if (!value) {
        *result = NULL;
        return IOTCL_SUCCESS;
    }
    const char *str_value = cJSON_GetStringValue(value);
    if (!str_value) {
        *result = NULL;
        return IOTCL_SUCCESS;
    }
    *result = iotcl_strdup(str_value);
    if (!*result) {
        return IOTCL_ERR_OUT_OF_MEMORY;
    }
    return IOTCL_SUCCESS;
}

// Helper: get non-empty string from JSON or return NULL
char *get_json_string(cJSON *root, const char *field) {
    cJSON *item = cJSON_GetObjectItem(root, field);
    if (!item || !cJSON_IsString(item)) return NULL;
    char *val = cJSON_GetStringValue(item);
    return (val && strlen(val) > 0) ? val : NULL;
}

