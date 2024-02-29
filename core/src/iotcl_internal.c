/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "iotcl_log.h"
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


/*
 * The function can be used to either locate an object along with the parent or create parents along the
 * way and return a suitable leaf name.
 *
 * Explaining the complex algorithm:
 * make a mutable string to work with because path will be mangled otherwise
 * Example path = "a.b.c"
 *  the first strtok returns token = "a"
 *  parent_object stays - it's pointing at json root
 *  enter loop - token should not be null ever when entering the first time
 *   record previous_token = token = "a"
 *   strtok returns token = "b"
 *   token is not null, so don't break loop
 *   create an object inside parent_object with name "a" (previous_token), so we can eventually add "b"
 *   parent_object = create result (object "a")
 *  iterate loop - token is not null, it is "b"
 *   record previous_token = token = "c"
 *   strtok returns token = "c"
 *   token is not null, so don't break loop
 *   create/get an object inside parent_object with name "b" (previous_token), so we can eventually add "b"
 *   parent_object = create/get result (object "b")
 *  iterate loop - token is not null, it is "c"
 *   strtok returns token = NULL;
 *   token is null, so break the loop
 *
 *  previous_token = "c", so we will return that by pointing to the right most part of the path argument after the dot,
 *  which is path_end minus the length of previous_token.
 *  parent_object is the last one created under "b"
 *
 *  Example path "foo":
 *   the first strtok returns token = "foo"
 *   parent_object stays - it's pointing at json root
 *   enter loop - token should not be null ever when entering the first time
 *    record previous_token = token = "foo"
 *    strtok returns token = NULL
 *    token is not null, so don't break loop
 *  previous_token = "foo", so we will return that by pointing to the right most part of the path argument after the now
 *  unmodified path string. path_end minus the length of previous_token logic works still.
 *  parent_object is the last one passed in
 */
int iotcl_cjson_dot_path_locate(
        cJSON *root_object,
        cJSON **parent_object,
        const char **leaf_name,
        const char *path
) {
    static const char *DELIM = ".";
    char *strtok_saveptr = NULL;

    *parent_object = root_object;
    // we must clone the string so it can be manipulated with strtok so we can use it
    char *mutable_path = iotcl_strdup(path);
    if (NULL == mutable_path) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "iotcl_cjson_dot_path_locate: Out of memory!");
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    char *token = strtok_r(mutable_path, DELIM, &strtok_saveptr);
    char *previous_token;

    if (NULL == token) {
        // PANIC. Should not happen on the first token
        iotcl_free(mutable_path);
        IOTCL_ERROR(IOTCL_ERR_FAILED, "iotcl_cjson_dot_path_locate: Unexpected strtok() NULL return!");
        return IOTCL_ERR_FAILED;
    }

    do {
        previous_token = token;
        token = strtok_r(NULL, DELIM, &strtok_saveptr);
        if (NULL == token) {
            break;
        }
        // double check first.. We could be looking at a pre-existing json structure
        if (cJSON_HasObjectItem(*parent_object, previous_token)) {
            *parent_object = cJSON_GetObjectItem(*parent_object, previous_token);
        } else {
            cJSON *obj = cJSON_AddObjectToObject(*parent_object, previous_token);
            if (!obj) {
                iotcl_free(mutable_path);
                IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "iotcl_cjson_dot_path_locate: Out of memory!");
                return IOTCL_ERR_OUT_OF_MEMORY;
            }
            *parent_object = obj;
        }
    } while (token != NULL);

    *leaf_name = &path[strlen(path) - strlen(previous_token)];
    iotcl_free(mutable_path);
    return IOTCL_SUCCESS;
}
