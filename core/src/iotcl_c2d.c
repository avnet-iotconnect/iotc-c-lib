/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

#include "cJSON.h"
#include "iotcl_log.h"
#include "iotcl_internal.h"
#include "iotcl_util.h"
#include "iotcl.h"
#include "iotcl_c2d.h"

#define HTTPS_PREFIX "https://"

bool is_protocol_version_warning_printed = false;

// Per https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/c2d-messages/#Other
typedef enum {
    IOTCL_C2D_ET_DEVICE_COMMAND = 0,
    IOTCL_C2D_ET_DEVICE_OTA = 1,

    // Other types that are not currently supported:
    IOTCL_C2D_ET_MODULE_COMMAND = 2,
    IOTCL_C2D_ET_REFRESH_ATTRIBUTE = 101,
    IOTCL_C2D_ET_REFRESH_SETTING = 102, // or Twin
    IOTCL_C2D_ET_REFRESH_EDGE_RULE = 103,
    IOTCL_C2D_ET_REFRESH_CHILD_DEVICE = 104,
    IOTCL_C2D_ET_DATA_FREQUENCY_CHANGE = 105,
    IOTCL_C2D_ET_DEVICE_DELETED = 106,
    IOTCL_C2D_ET_DEVICE_DISABLED = 107,
    IOTCL_C2D_ET_DEVICE_RELEASED = 108,
    IOTCL_C2D_ET_STOP_OPERATION = 109,
    IOTCL_C2D_ET_START_HEARTBEAT = 110,
    IOTCL_C2D_ET_STOP_HEARTBEAT = 111,
} IotclC2dEventType;

struct IotclC2dEventDataTag {
    cJSON *root;
    IotclC2dEventType type;
    char *hostname; // May ore may not be allocated. Temporary storage for parsed hostname string.
};

static int iotcl_c2d_process_callback(struct IotclC2dEventDataTag *event_data) {
    IotclGlobalConfig *config = iotcl_get_global_config();
    if (!config->is_valid) {
        // an error will be printed by the called function
        return IOTCL_ERR_CONFIG_MISSING;
    }

    switch (event_data->type) {
        case IOTCL_C2D_ET_DEVICE_COMMAND:
            if (config->event_functions.cmd_cb) {
                config->event_functions.cmd_cb(event_data);
            }
            break;
        case IOTCL_C2D_ET_DEVICE_OTA:
            if (config->event_functions.ota_cb) {
                config->event_functions.ota_cb(event_data);
            }
            break;
        default:
            // should be pre-checked and never happen
            break;
    }

    return IOTCL_SUCCESS;
}

static bool is_valid_string(const cJSON *json) {
    return (NULL != json && cJSON_IsString(json) && json->valuestring != NULL);
}

static int iotcl_c2d_parse_json(cJSON *root) {
    int status; // unknown in case someone forgot to set it

    // parse version
    cJSON *j_v = cJSON_GetObjectItem(root, "v");
    if (!is_valid_string(j_v)) {
        status = IOTCL_ERR_PARSING_ERROR;
        IOTCL_ERROR(status, "Unable to parse protocol version from the message");
        goto cleanup;
    }
    char *version = cJSON_GetStringValue(j_v);
    if (strcmp(version, "2.1") != 0 && !is_protocol_version_warning_printed) {
        is_protocol_version_warning_printed = true;
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "Encountered potentially unsupported protocol version %s!", version);
    }

    // parse event type
    cJSON *j_ct = cJSON_GetObjectItem(root, "ct");
    if (!j_ct || !cJSON_IsNumber(j_ct)) {
        status = IOTCL_ERR_PARSING_ERROR;
        IOTCL_ERROR(status, "Unable to parse message type (\"ct\")");
        goto cleanup;
    }
    int type = (int) cJSON_GetNumberValue(j_ct);

    if (type != IOTCL_C2D_ET_DEVICE_COMMAND && type != IOTCL_C2D_ET_DEVICE_OTA) {
        status = IOTCL_ERR_PARSING_ERROR;
        IOTCL_WARN(IOTCL_ERR_PARSING_ERROR, "Received unsupported message type %d", type);
        goto cleanup;
    }

    struct IotclC2dEventDataTag event_data = {0};
    event_data.root = root;
    event_data.type = type;

    root = NULL; // Clear this pointer to avoid a double free in case some other step that can fail is added below

    status = iotcl_c2d_process_callback(&event_data);
    iotcl_c2d_destroy_event(&event_data);
    return status;

    cleanup:
    cJSON_Delete(root);
    return status;
}

static const char *iotcl_c2d_get_string_value(cJSON *target_object, bool is_required, const char *name) {
    const char *ret = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(target_object, name));
    if (is_required && !ret) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "\"%s\" was not found in c2d response", name);
    }
    return ret;
}

static int iotcl_c2d_validate_data_and_type(
        IotclC2dEventData data,
        IotclC2dEventType expected_type,
        const char *what
) {
    if (!data) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "c2d event data null while attempting to get the \"%s\" value", what);
        return IOTCL_ERR_MISSING_VALUE;
    }
    if (data->type != expected_type) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "Incorrect c2d event type %d! Expected %d.", data->type, expected_type);
        return IOTCL_ERR_BAD_VALUE;
    }
    return IOTCL_SUCCESS;
}

static cJSON *iotcl_c2d_get_ota_url_array_item(IotclC2dEventData data, int index) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "download URL")) {
        return NULL;
    }
    cJSON *urls = cJSON_GetObjectItemCaseSensitive(data->root, "urls");
    if (NULL == urls || !cJSON_IsArray(urls)) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "the \"urls\" array is not found in c2d response");
        return NULL;
    }
    if (index < 0 || cJSON_GetArraySize(urls) > index) {
        // this URL should never be null...
        return cJSON_GetArrayItem(urls, (int) index);
    } else {
        IOTCL_ERROR(
                IOTCL_ERR_BAD_VALUE,
                "Attempting to access the OTA url item at index %d, but there are only %d items in the array",
                (int) index,
                cJSON_GetArraySize(urls)
        );
        return NULL;
    }
}

static char *iotcl_c2d_create_ack(IotclC2dEventType type, const char *ack_id, int status, const char *message) {
    char *result = NULL;

    cJSON *ack_json = cJSON_CreateObject();
    if (!ack_json) goto cleanup;
#ifdef IOTCL_C2D_LEGACY_ACK_SUPPORT
    if (!cJSON_AddStringToObject(ack_json, "t", "2024-00-00T00:00:00.000Z")) goto cleanup;
#endif
    cJSON *ack_d = cJSON_AddObjectToObject(ack_json, "d");
    if (!ack_d) goto cleanup;

    if (!cJSON_AddStringToObject(ack_d, "ack", ack_id)) goto cleanup;
    if (!cJSON_AddNumberToObject(ack_d, "st", status)) goto cleanup;
    if (!cJSON_AddNumberToObject(ack_d, "type", type)) goto cleanup;
    if (message && strlen(message) > 0) {
        if (!cJSON_AddStringToObject(ack_d, "msg", message)) goto cleanup;
    }
    result = cJSON_PrintUnformatted(ack_json);
    if (!result) goto cleanup;

    cJSON_Delete(ack_json);

    return result;

    cleanup:
    cJSON_free(result);
    cJSON_Delete(ack_json);

    IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "Out of memory while creating the ack JSON!");
    return NULL;
}

int iotcl_c2d_process_event(const char *str) {
    cJSON *root = cJSON_Parse(str);
    if (!root) {
        IOTCL_ERROR(
                IOTCL_ERR_PARSING_ERROR,
                "JSON parsing error or possible out of memory error while parsing: \"%s\"",
                str
        );
        return IOTCL_ERR_PARSING_ERROR;
    }
    return iotcl_c2d_parse_json(root);
}

int iotcl_c2d_process_event_with_length(const uint8_t *data, size_t data_len) {
    cJSON *root = cJSON_ParseWithLength((const char *) data, data_len);
    if (!root) {
        IOTCL_ERROR(
                IOTCL_ERR_PARSING_ERROR,
                "jSON parsing error or possible out of memory error while parsing: \"%.*s\"",
                (int) data_len,
                data
        );
        return IOTCL_ERR_PARSING_ERROR;
    }
    return iotcl_c2d_parse_json(root);
}

const char *iotcl_c2d_get_ota_url(IotclC2dEventData data, int index) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "OTA URL")) {
        return NULL;
    }
    cJSON *url_array_item = iotcl_c2d_get_ota_url_array_item(data, index);
    if (!url_array_item) {
        // called function logs the error
        return NULL;
    }
    return iotcl_c2d_get_string_value(url_array_item, true, "url");
}

const char *iotcl_c2d_get_ota_url_hostname(IotclC2dEventData data, int index) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "OTA URL hostname")) {
        return NULL;
    }
    // shortcut... we've already done the parsing and allocation
    if (data->hostname) {
        return data->hostname;
    }
    cJSON *url_array_item = iotcl_c2d_get_ota_url_array_item(data, index);
    if (!url_array_item) {
        // called function logs the error
        return NULL;
    }
    const char *url = iotcl_c2d_get_string_value(url_array_item, true, "url");
    if (!url) {
        // called function logs the error
        return NULL;
    }
    if (!strstr(url, HTTPS_PREFIX)) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "The download URL is missing the leading \"https://\"");
        return NULL;
    }
    const char *host_start = &url[strlen(HTTPS_PREFIX)]; // cannot be null because strstr(url, HTTPS_PREFIX) check
    const char *resource_start = strstr(host_start, "/");
    if (!resource_start) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "The download URL does not appear to be correctly formatted");
        return NULL;
    }
    size_t hostname_str_len = (size_t) (resource_start - host_start);
    char *hostname = iotcl_malloc(hostname_str_len + 1 /* for null terminator */);
    if (!hostname) {
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "Out of memory while allocating the OTA hostname string");
        return NULL;
    }
    strncpy(hostname, host_start, hostname_str_len);
    hostname[hostname_str_len] = '\0'; // just to be sure
    data->hostname = hostname; // record it so we can free it and shortcut it
    return hostname;
}

const char *iotcl_c2d_get_ota_url_resource(IotclC2dEventData data, int index) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "OTA URL resource")) {
        return NULL;
    }
    cJSON *url_array_item = iotcl_c2d_get_ota_url_array_item(data, index);
    if (!url_array_item) {
        // called function logs the error
        return NULL;
    }

    const char *url = iotcl_c2d_get_string_value(url_array_item, true, "url");
    if (!url) {
        // called function logs the error
        return NULL;
    }
    if (!strstr(url, HTTPS_PREFIX)) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "The download URL is missing the leading \"https://\"");
        return NULL;
    }
    const char *host_start = &url[strlen(HTTPS_PREFIX)];
    char *resource_start = strstr(host_start, "/");
    if (!resource_start) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "The download URL does not appear to be correctly formatted");
        return NULL;
    }
    return resource_start;
}

const char *iotcl_c2d_get_ota_original_filename(IotclC2dEventData data, int index) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "OTA original filename")) {
        return NULL;
    }
    cJSON *url_array_item = iotcl_c2d_get_ota_url_array_item(data, index);
    if (!url_array_item) {
        // called function logs the error
        return NULL;
    }
    return iotcl_c2d_get_string_value(url_array_item, true, "fileName");
}

const char *iotcl_c2d_get_command(IotclC2dEventData data) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_COMMAND, "command")) {
        return NULL;
    }
    return iotcl_c2d_get_string_value(data->root, true, "cmd");
}

int iotcl_c2d_get_ota_url_count(IotclC2dEventData data) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "URL count")) {
        return 0;
    }
    cJSON *urls = cJSON_GetObjectItemCaseSensitive(data->root, "urls");
    if (NULL == urls || !cJSON_IsArray(urls)) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "the \"urls\" array is not found in c2d response");
        return 0;
    }
    return cJSON_GetArraySize(urls);
}

const char *iotcl_c2d_get_ota_sw_version(IotclC2dEventData data) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "sw version")) {
        return NULL;
    }
    return iotcl_c2d_get_string_value(data->root, true, "sw");
}

const char *iotcl_c2d_get_ota_hw_version(IotclC2dEventData data) {
    if (IOTCL_SUCCESS != iotcl_c2d_validate_data_and_type(data, IOTCL_C2D_ET_DEVICE_OTA, "hw version")) {
        return NULL;
    }
    return iotcl_c2d_get_string_value(data->root, true, "hw");
}

const char *iotcl_c2d_get_ack_id(IotclC2dEventData data) {
    if (!data) {
        // a bit of string re-use here at a cost of CPU time and stack
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "c2d event data null while attempting to get the \"%s\" value", "ack ID");
        return NULL;
    }
    return iotcl_c2d_get_string_value(data->root, false, "ack");
}


char *iotcl_c2d_create_cmd_ack_json(const char *ack_id, int cmd_status, const char *message) {
    if (!ack_id|| 0 == strlen(ack_id)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_c2d_create_cmd_ack_json: ack_id is required!");
        return NULL;
    }
    // not checking for possible values status yet until we resolve back end issues
    return iotcl_c2d_create_ack(IOTCL_C2D_ET_DEVICE_COMMAND, ack_id, cmd_status, message);
}

char *iotcl_c2d_create_ota_ack_json(const char *ack_id, int ota_status, const char *message) {
    if (!ack_id || 0 == strlen(ack_id)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_c2d_create_ota_ack_json: ack_id is required!");
        return NULL;
    }
    // not checking for possible values status yet until we resolve back end issues
    return iotcl_c2d_create_ack(IOTCL_C2D_ET_DEVICE_OTA, ack_id, ota_status, message);
}

void iotcl_c2d_destroy_ack_json(char *ack_json_ptr) {
    cJSON_free(ack_json_ptr);
}

void iotcl_c2d_destroy_event(IotclC2dEventData data) {
    cJSON_Delete(data->root);
    data->root = NULL;
    iotcl_free(data->hostname); // in case it was created
    data->hostname = NULL;
}
