/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>


#include "iotconnect_common.h"
#include "iotconnect_lib.h"
#include "iotconnect_telemetry.h"


#include <cJSON.h>

/////////////////////////////////////////////////////////
// cJSON implementation

struct IOTCL_MESSAGE_HANDLE_TAG {
    cJSON *root_value;
    cJSON *telemetry_data_array;
    cJSON *current_telemetry_object; // an object inside the "d" array inside the "d" array of the root object
};

cJSON *json_object_dotset_locate(cJSON *search_object, char **leaf_name, const char *path) {
    static const char *DELIM = ".";
    char *mutable_path = IOTCL_Strdup(path);
    char *token = strtok(mutable_path, DELIM);
    *leaf_name = NULL;
    if (NULL == token) {
        // PANIC. Should not happen
        free(mutable_path);
        return NULL;
    }
    cJSON *target_object = search_object;
    do {
        free(*leaf_name);
        *leaf_name = IOTCL_Strdup(token);
        if (!*leaf_name) {
            goto cleanup;
        }
        char *last_token = *leaf_name;
        token = strtok(NULL, DELIM);
        if (NULL != token) {
            // we have more and need to create the nested object
            cJSON *parent_object;
            if (cJSON_HasObjectItem(target_object, last_token)) {
                parent_object = cJSON_GetObjectItem(target_object, last_token);
            } else {
                parent_object = cJSON_AddObjectToObject(target_object, last_token);

                // NOTE: The user should clean up the search object if we return
                // That should free all added objects, so this should be safe to do
                if (!parent_object) goto cleanup;
            }
            target_object = parent_object;
        }
    } while (token != NULL);

    free(mutable_path);
    return target_object;

    cleanup:
    free(mutable_path);
    return NULL;
}

static cJSON *setup_telemetry_object(IOTCL_MESSAGE_HANDLE message) {
    IOTCL_CONFIG *config = IOTCL_GetConfig();
    if (!config) return NULL;
    if (!message) return NULL;

    cJSON *telemetry_object = cJSON_CreateObject();
    if (!telemetry_object) return NULL;
    cJSON_AddStringToObject(telemetry_object, "id", config->device.duid);
    cJSON_AddStringToObject(telemetry_object, "tg", "");
    cJSON *data_array = cJSON_AddArrayToObject(telemetry_object, "d");
    if (!data_array) goto cleanup_to;
    if (!cJSON_AddItemToArray(message->telemetry_data_array, telemetry_object)) goto cleanup_da;

    // setup the actual telemetry object to be used in subsequent calls
    message->current_telemetry_object = cJSON_CreateObject();
    if (!message->current_telemetry_object) goto cleanup_da;

    if (!cJSON_AddItemToArray(data_array, message->current_telemetry_object)) goto cleanup_cto;

    return telemetry_object; // object inside the "d" array of the the root object

    cleanup_cto:
    cJSON_free(message->current_telemetry_object);

    cleanup_da:
    cJSON_free(data_array);

    cleanup_to:
    cJSON_free(telemetry_object);

    return NULL;
}

IOTCL_MESSAGE_HANDLE IOTCL_TelemetryCreate() {
    cJSON *sdk_array = NULL;
    IOTCL_CONFIG *config = IOTCL_GetConfig();
    if (!config) return NULL;
    if (!config->telemetry.dtg) return NULL;
    struct IOTCL_MESSAGE_HANDLE_TAG *msg =
            (struct IOTCL_MESSAGE_HANDLE_TAG *) calloc(sizeof(struct IOTCL_MESSAGE_HANDLE_TAG), 1);

    if (!msg) return NULL;

    msg->root_value = cJSON_CreateObject();

    if (!msg->root_value) goto cleanup;

    if (!cJSON_AddStringToObject(msg->root_value, "cpid", config->device.cpid)) goto cleanup;
    if (!cJSON_AddStringToObject(msg->root_value, "dtg", config->telemetry.dtg)) goto cleanup;
    if (!cJSON_AddNumberToObject(msg->root_value, "mt", 0)) goto cleanup; // telemetry message type (zero)
    sdk_array = cJSON_AddObjectToObject(msg->root_value, "sdk");
    if (!sdk_array)  goto cleanup;
    if (!cJSON_AddStringToObject(sdk_array, "l", CONFIG_IOTCONNECT_SDK_NAME)) goto cleanup_array;
    if (!cJSON_AddStringToObject(sdk_array, "v", CONFIG_IOTCONNECT_SDK_VERSION)) goto cleanup_array;
    if (!cJSON_AddStringToObject(sdk_array, "e", config->device.env)) goto cleanup_array;

    msg->telemetry_data_array = cJSON_AddArrayToObject(msg->root_value, "d");

    if (!msg->telemetry_data_array) goto cleanup_array;

    return msg;

    cleanup_array:
    cJSON_free(sdk_array);
    cleanup:
    cJSON_free(msg);
    return NULL;
}

bool IOTCL_TelemetryAddWithEpochTime(IOTCL_MESSAGE_HANDLE message, time_t time) {
    if (!message) return false;
    cJSON *telemetry_object = setup_telemetry_object(message);
    cJSON_AddNumberToObject(telemetry_object, "ts", time);
    if (!cJSON_HasObjectItem(message->root_value, "ts")) {
        cJSON_AddNumberToObject(message->root_value, "ts", time);
    }
    return true;
}

bool IOTCL_TelemetryAddWithIsoTime(IOTCL_MESSAGE_HANDLE message, const char *time) {
    if (!message) return false;
    cJSON *const telemetry_object = setup_telemetry_object(message);
    if (!telemetry_object)  return false;
    if (!cJSON_AddStringToObject(telemetry_object, "dt", time)) return false;
    if (!cJSON_HasObjectItem(message->root_value, "t")) {
        if (!cJSON_AddStringToObject(message->root_value, "t", time)) return false;
    }
    return true;
}

bool IOTCL_TelemetrySetNumber(IOTCL_MESSAGE_HANDLE message, const char *path, double value) {
    if (!message) return false;
    if (NULL == message->current_telemetry_object) {
        if (!IOTCL_TelemetryAddWithIsoTime(message, IOTCL_IsoTimestampNow())) return false;
    }
    char *leaf_name = NULL;
    cJSON *target = json_object_dotset_locate(message->current_telemetry_object, &leaf_name, path);
    if (!target) goto cleanup; // out of memory

    if (!cJSON_AddNumberToObject(target, leaf_name, value)) goto cleanup_tgt;
    free(leaf_name);
    return true;

    cleanup_tgt:
    cJSON_free(target);

    cleanup:
    free(leaf_name);
    return false;
}

bool IOTCL_TelemetrySetBool(IOTCL_MESSAGE_HANDLE message, const char *path, bool value) {
    if (!message) return false;
    if (NULL == message->current_telemetry_object) {
        if (!IOTCL_TelemetryAddWithIsoTime(message, IOTCL_IsoTimestampNow())) return false;
    }
    char *leaf_name = NULL;
    cJSON *target = json_object_dotset_locate(message->current_telemetry_object, &leaf_name, path);
    if (!target) goto cleanup; // out of memory

    if (!cJSON_AddBoolToObject(target, leaf_name, value)) goto cleanup_tgt;
    free(leaf_name);
    return true;

    cleanup_tgt:
    cJSON_free(target);

    cleanup:
    free(leaf_name);
    return false;
}

bool IOTCL_TelemetrySetString(IOTCL_MESSAGE_HANDLE message, const char *path, const char *value) {
    if (!message) return false;
    if (NULL == message->current_telemetry_object) {
        IOTCL_TelemetryAddWithIsoTime(message, IOTCL_IsoTimestampNow());
    }
    char *leaf_name = NULL;
    cJSON *const target = json_object_dotset_locate(message->current_telemetry_object, &leaf_name, path);
    if (!target) goto cleanup; // out of memory

    if (!cJSON_AddStringToObject(target, leaf_name, value)) goto cleanup_tgt;
    free(leaf_name);
    return true;

    cleanup_tgt:
    cJSON_free(target);

    cleanup:
    free(leaf_name);
    return false;
}

bool IOTCL_TelemetrySetNull(IOTCL_MESSAGE_HANDLE message, const char *path) {
    if (!message) return false;
    if (NULL == message->current_telemetry_object) {
        IOTCL_TelemetryAddWithIsoTime(message, IOTCL_IsoTimestampNow());
    }
    char *leaf_name = NULL;
    cJSON *const target = json_object_dotset_locate(message->current_telemetry_object, &leaf_name, path);
    if (!target) goto cleanup; // out of memory

    if (!cJSON_AddNullToObject(target, leaf_name)) goto cleanup_tgt;
    free(leaf_name);
    return true;

    cleanup_tgt:
    cJSON_free(target);

    cleanup:
    free(leaf_name);
    return false;
}

const char *IOTCL_CreateSerializedString(IOTCL_MESSAGE_HANDLE message, bool pretty) {
    if (!message) return NULL;
    if (!message->root_value) return NULL;
    if (pretty) {
        return cJSON_Print(message->root_value);
    } else {
        return cJSON_PrintUnformatted(message->root_value);
    }
}

void IOTCL_DestroySerialized(const char *serialized_string) {
    cJSON_free((char *) serialized_string);
}

void IOTCL_TelemetryDestroy(IOTCL_MESSAGE_HANDLE message) {
    if (message) {
        cJSON_Delete(message->root_value);
    }
    free(message);
}
