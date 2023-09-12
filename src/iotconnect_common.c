/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iotconnect_common.h"

static char timebuf[sizeof "2011-10-08T07:07:01.000Z"];

static const char *to_iso_timestamp(time_t *timestamp) {
    time_t ts = timestamp ? *timestamp : time(NULL);
    strftime(timebuf, (sizeof timebuf), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&ts));
    return timebuf;
}

const char *iotcl_to_iso_timestamp(time_t timestamp) {
    return to_iso_timestamp(&timestamp);
}

const char *iotcl_iso_timestamp_now(void) {
    return to_iso_timestamp(NULL);
}

char *iotcl_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    size_t size = strlen(str) + 1;
    char *p = (char *) malloc(size);
    if (p != NULL) {
        memcpy(p, str, size);
    }
    return p;
}

int safe_get_integer(cJSON *cjson, const char *value_name, int *value) {
    printf("%s: parsing \"%s\"\n", __func__, value_name);
    cJSON *tmp_value = cJSON_GetObjectItem(cjson, value_name);
    if (!tmp_value || !cJSON_IsNumber(tmp_value)) {
        return -1;
    }
    *value = cJSON_GetNumberValue(tmp_value);
    return 0;
}

int get_numeric_value_or_default(cJSON *cjson, const char *value_name, int default_value) {
    int value = default_value;
    if(safe_get_integer(cjson, value_name, &value) == 0)
    {
        return value;
    }
    return default_value;
}

char *safe_get_string_and_strdup(cJSON *cjson, const char *value_name) {
    printf("%s: parsing \"%s\"\n", __func__, value_name);
    cJSON *value = cJSON_GetObjectItem(cjson, value_name);
    if (!value) {
        return NULL;
    }
    const char *str_value = cJSON_GetStringValue(value);
    if (!str_value) {
        return NULL;
    }

    printf("%s -> %s = %s\n", __func__, value_name, str_value);

    return iotcl_strdup(str_value);
}

