/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
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
