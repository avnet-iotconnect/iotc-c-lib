/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stddef.h>
#include <string.h>

#include "cJSON.h"

#include "iotcl_util.h"
#include "iotcl_internal.h"
#include "iotcl_log.h"
#include "iotcl.h"
#include "iotcl_telemetry.h"

struct IotclMessageHandleTag {
    cJSON *root_value;       // The root of the message. Only this one needs to be JSON_Delete-d
    cJSON *data_set_array;   // Convenience: The "d" array of data points.
    cJSON *current_data_set; // Convenience: Current data set object inside the "d" array containing current data values.
};

static int setup_data_set_object(const char *function_name, IotclMessageHandle message, const char *iso_timestamp) {
    cJSON *current_data_set = NULL;
    cJSON *array_item = cJSON_CreateObject();

    if (!array_item) goto oom_error;

    // used if time_fn is configured
    char time_str_buffer[IOTCL_ISO_TIMESTAMP_STR_LEN + 1] = {0};

    // If the user didn't pass the timestamp and time function is configured
    if (!iso_timestamp && iotcl_get_global_config()->time_fn) {
        int status = iotcl_iso_timestamp_now(time_str_buffer, sizeof(time_str_buffer));
        if (IOTCL_SUCCESS == status) {
            iso_timestamp = time_str_buffer;
        } else {
            // The called function will print the error.
            return status;
        }
    }

    if (iso_timestamp) {
        if (NULL == cJSON_AddStringToObject(array_item, "dt", iso_timestamp)) {
            // don't clean up current_data_set to make it worse than it is. At least we can send the measage without "ts".
            goto oom_error;
        }
    }

    current_data_set = cJSON_AddObjectToObject(array_item, "d");
    if (!current_data_set) goto oom_error;

    // This needs to be the last potential failure to avoid potential double free from deleting the array_item chain
    if (!cJSON_AddItemToArray(message->data_set_array, array_item)) goto oom_error;

    // and set this up at last, as it cannot fail
    message->current_data_set = current_data_set;

    return IOTCL_SUCCESS; // object inside the "d" array of the the root object

    oom_error:
    cJSON_Delete(array_item);
    cJSON_Delete(current_data_set);
    IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory!", function_name);
    return IOTCL_ERR_OUT_OF_MEMORY;
}

// Common functionality for all set functions.
// Lazy creates message->current_data_set and sets it up with timestamp (if available).
// Prints common errors and returns the error if one is encountered.
static int iotcl_telemetry_set_functions_common(
        const char *function_name,
        cJSON **parent_object,
        const char **leaf_name,
        IotclMessageHandle message,
        const char *path
) {
    const int DOT_INDEX_NONE = -1;
    int status;

    *parent_object = NULL;
    *leaf_name = NULL;

    if (NULL == message) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message handle argument is required!", function_name);
        return IOTCL_ERR_MISSING_VALUE;
    }

    if (NULL == path || 0 == strlen(path)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The path argument is required!", function_name);
        return IOTCL_ERR_MISSING_VALUE;
    }
    if (NULL == parent_object || NULL == leaf_name) { // internal error
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: parent_object and leaf_name are required!", function_name);
        return IOTCL_ERR_MISSING_VALUE;
    }
    if (NULL == message->current_data_set) {
        status = setup_data_set_object(function_name, message, NULL);
        if (status) {
            // the called function will print the error and clean up
            return status;
        }
    }
    *parent_object = message->current_data_set;

    int dot_index = DOT_INDEX_NONE;
    size_t path_len = strlen(path);
    // walk the string and look for dots
    for (size_t i = 0; i < path_len; i++) {
        char ch = path[i];
        if (ch == '.') {
            if (i == 0) {
                IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "%s: Path \"%s\" cannot start with \".\"!", function_name, path);
                return IOTCL_ERR_BAD_VALUE;
            } else if (dot_index == DOT_INDEX_NONE) {
                dot_index = (int) i;
            } else {
                IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "%s: Path \"%s\" cannot cannot have more than one \".\"!", function_name, path);
                return IOTCL_ERR_BAD_VALUE;
            }
        }
    }

    if (dot_index == DOT_INDEX_NONE) {
        *parent_object = message->current_data_set;
        *leaf_name = path;
        return  IOTCL_SUCCESS;
    } else {
        const char *leaf_name_str = &path[dot_index + 1];
        if (*leaf_name_str == '\0') {
            IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "%s: Path \"%s\" cannot end with \".\"!", function_name, path);
            return IOTCL_ERR_BAD_VALUE;
        }
        char *object_name = iotcl_strdup(path);
        if (!object_name) {
            IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory!", function_name);
            return IOTCL_ERR_BAD_VALUE;
        }
        // else we need to break down the path string in two on the dot...

        // truncate the duplicate here so we can nest with the object name
        object_name[dot_index] = '\0';
        *parent_object = cJSON_AddObjectToObject(message->current_data_set, object_name);
        iotcl_free(object_name);
        if (!*parent_object) {
            IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory!", function_name);
            return IOTCL_ERR_BAD_VALUE;
        }
        *leaf_name = leaf_name_str;
    }
    return IOTCL_SUCCESS;
}

IotclMessageHandle iotcl_telemetry_create(void) {
    const char * FUNCTION_NAME = "iotcl_telemetry_set_null";

    // check early in the call sequence that the config is valid, so it is safe to assume it is configured
    // in subsequent calls to other iotcl_telemetry_* functions.
    if (!iotcl_get_global_config()->is_valid) {
        return NULL; // called function will print the error
    }

    struct IotclMessageHandleTag *message = iotcl_malloc(sizeof(struct IotclMessageHandleTag))
    ;

    if (!message) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "iotcl_telemetry_create: Out of memory error while allocating message handle!");
        return NULL;
    }
    memset(message, 0, sizeof(struct IotclMessageHandleTag));

    message->root_value = cJSON_CreateObject();
    if (!message->root_value) goto cleanup;

    message->data_set_array = cJSON_AddArrayToObject(message->root_value, "d");
    if (!message->data_set_array) goto cleanup;

    return message;

    cleanup:
    IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
    cJSON_Delete(message->root_value);
    message->root_value = NULL;
    message->data_set_array = NULL;
    return NULL;
}

int iotcl_telemetry_add_new_data_set(IotclMessageHandle message, const char *iso_timestamp) {
    const char *FUNCTION_NAME = "iotcl_telemetry_add_new_data_set";
    if (NULL == message) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message handle argument is required!", FUNCTION_NAME);
        return IOTCL_ERR_MISSING_VALUE;
    }
    if (NULL == iso_timestamp) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The iso_timestamp argument is required!", FUNCTION_NAME);
        return IOTCL_ERR_MISSING_VALUE;
    }
    int status = setup_data_set_object("iotcl_telemetry_add_with_iso_time", message, iso_timestamp);
    if (status) {
        // called function should print the error message
        return status;
    }
    return IOTCL_SUCCESS;
}

int iotcl_telemetry_set_number(IotclMessageHandle message, const char *path, double value) {
    const char *FUNCTION_NAME = "iotcl_telemetry_set_number";

    if (NULL == message) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message handle argument is required!", FUNCTION_NAME);
        return IOTCL_ERR_MISSING_VALUE;
    }

    cJSON *parent_object = NULL;
    const char *leaf_name = NULL;
    int status = iotcl_telemetry_set_functions_common(
            FUNCTION_NAME,
            &parent_object,
            &leaf_name,
            message,
            path
    );
    if (status) {
        // called function will print the error
        return status;
    }

    if (!cJSON_AddNumberToObject(parent_object, leaf_name, value)) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}

int iotcl_telemetry_set_string(IotclMessageHandle message, const char *path, const char *value) {
    const char *FUNCTION_NAME = "iotcl_telemetry_set_string";
    if (NULL == message) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message handle argument is required!", FUNCTION_NAME);
        return IOTCL_ERR_MISSING_VALUE;
    }

    const char *leaf_name = NULL;
    cJSON *parent_object = NULL;

    int status = iotcl_telemetry_set_functions_common(
            FUNCTION_NAME,
            &parent_object,
            &leaf_name,
            message,
            path
    );
    if (status) {
        // called function will print the error
        return status;
    }

    if (!cJSON_AddStringToObject(parent_object, leaf_name, value)) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}

int iotcl_telemetry_set_bool(IotclMessageHandle message, const char *path, bool value) {
    const char *FUNCTION_NAME = FUNCTION_NAME;
    const char *leaf_name = NULL;
    cJSON *parent_object = NULL;
    int status = iotcl_telemetry_set_functions_common(
            "iotcl_telemetry_set_bool",
            &parent_object,
            &leaf_name,
            message,
            path
    );
    if (status) {
        // called function will print the error
        return status;
    }

    if (!cJSON_AddBoolToObject(parent_object, leaf_name, value)) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}


int iotcl_telemetry_set_null(IotclMessageHandle message, const char *path) {
    const char *FUNCTION_NAME = "iotcl_telemetry_set_null";
    const char *leaf_name = NULL;
    cJSON *parent_object = NULL;
    int status = iotcl_telemetry_set_functions_common(
            FUNCTION_NAME,
            &parent_object,
            &leaf_name,
            message,
            path
    );
    if (status) {
        // called function will print the error
        return status;
    }

    if (!cJSON_AddNullToObject(parent_object, leaf_name)) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;
}

char *iotcl_telemetry_create_serialized_string(IotclMessageHandle message, bool pretty) {
    const char *FUNCTION_NAME = "iotcl_create_serialized_string";

    if (NULL == message) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message handle argument is required!", FUNCTION_NAME);
        return NULL;
    }

    if (NULL == message->root_value) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "%s: The message is empty!", FUNCTION_NAME);
        return NULL;
    }

    char *serialized_string = (pretty) ?
                              cJSON_Print(message->root_value) : cJSON_PrintUnformatted(message->root_value);

    if (!serialized_string) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "%s: Out of memory error!", FUNCTION_NAME);
        return NULL;
    }
    return serialized_string;
}

void iotcl_telemetry_destroy_serialized_string(char *serialized_string) {
    cJSON_free(serialized_string);
}

void iotcl_telemetry_destroy(IotclMessageHandle message) {
    if (message) {
        cJSON_Delete(message->root_value);
        iotcl_free(message);
    }
}
