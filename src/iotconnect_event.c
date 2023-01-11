/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al & Neerav Parasher <neerav.parasar@softwebsolutions.com>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "iotconnect_common.h"
#include "iotconnect_lib.h"

#define CJSON_ADD_ITEM_HAS_RETURN \
    (CJSON_VERSION_MAJOR * 10000 + CJSON_VERSION_MINOR * 100 + CJSON_VERSION_PATCH >= 10713)

#if !CJSON_ADD_ITEM_HAS_RETURN
#error "cJSON version must be 1.7.13 or newer"
#endif

struct IotclEventDataTag {
    cJSON *data;
    cJSON *root;
    IotConnectEventType type;
};

/*
Conversion of boolean to IOT 2.0 specification messages:

For commands:
Table 15 [Possible values for st]
4 Command Failed with some reason
6 Executed successfully

For OTA:
Table 16 [Possible values for st]
4 Firmware command Failed with some reason
7 Firmware command executed successfully
*/
static int to_ack_status(bool success, IotConnectEventType type) {
    int status = 4; // default is "failure"
    if (success == true) {
        switch (type) {
            case DEVICE_COMMAND:
                status = 6;
                break;
            case DEVICE_OTA:
                status = 0;
                break;
            default:; // Can't do more than assume failure if unknown type is used.
        }
    }
    return status;
}


static bool iotc_process_callback(struct IotclEventDataTag *eventData) {
    if (!eventData) return false;

    IotclConfig *config = iotcl_get_config();
    if (!config) return false;

    switch (eventData->type) {
        case DEVICE_COMMAND:
            if (config->event_functions.cmd_cb) {
                config->event_functions.cmd_cb(eventData);
            }
            break;
        case DEVICE_OTA:
            if (config->event_functions.ota_cb) {
                config->event_functions.ota_cb(eventData);
            }
            break;
        case ON_CLOSE: case DEVICE_DELETE: case DEVICE_DISABLE: case STOP_OPERATION:
            if (config->event_functions.msg_cb) {
                config->event_functions.msg_cb(eventData, eventData->type);
            }
            break;
        default:
            break;
    }

    return true;
}


/************************ CJSON IMPLEMENTATION **************************/
static inline bool is_valid_string(const cJSON *json) {
    return (NULL != json && cJSON_IsString(json) && json->valuestring != NULL);
}

bool iotcl_process_event(const char *event) {
    int cmd_type;
    cJSON* cmd = NULL;
    cJSON* j_ack_id = NULL;
    cJSON* root = cJSON_Parse(event);

    if (!root){
        return false;
    }  
    { // scope out the on-the fly varialble declarations for cleanup jump
        // root object should only have cmdType
        cJSON *j_type = cJSON_GetObjectItemCaseSensitive(root, "ct");
        if (j_type) {
            cmd_type = j_type->valueint;
            switch (cmd_type) {
            case DEVICE_COMMAND://ct :0
                cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
                if (!cmd) {
                    goto cleanup;
                }
                j_ack_id = cJSON_GetObjectItemCaseSensitive(root, "ack");
                if (!is_valid_string(j_ack_id)) {
                    goto cleanup;
                }
                break;

            case DEVICE_OTA://ct : 1
                cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
                if (!cmd) {
                    goto cleanup;
                }

                j_ack_id = cJSON_GetObjectItemCaseSensitive(root, "ack");
                if (!is_valid_string(j_ack_id)) {
                    goto cleanup;
                }
                break;

            case MODULE_UPDATE_COMMAND://ct : 2
                break;

            case ON_CHANGE_ATTRIBUTE://ct :101
                // FIXME TODO CHECK unused -- int get_attr = 201;
                break;

            case REFRESH_SETTING_TWIN://ct :102
                break;

            case REFRESH_CHILD_DEVICE://ct :104
                break;

            case DF_CHANGE://ct :105  
                break;

            case DEVICE_DELETE://ct :106 
                break;

            case DEVICE_DISABLE://ct :107
                break;

            case ON_CLOSE://ct :108 
                break;

            case STOP_OPERATION://ct :109 
                break;

            case HEARTBEAT_COMMAND://ct :110
                break;

            case STOP_HEARTBEAT://ct :111
                break;

            default:; // Can't do more than assume failure if unknown type is used.
            }
            IotConnectEventType type = cmd_type;
            struct IotclEventDataTag *eventData = (struct IotclEventDataTag *) calloc(sizeof(struct IotclEventDataTag), 1);
            if (NULL == eventData) goto cleanup;
            eventData->root = root;
            // FIXME TODO CHECK -- eventData->data should be assigned or removed?;
            eventData->type = type;
            // FIXME TODO CHECK -- memory leak?
            bool result = iotc_process_callback(eventData);
            free(eventData);
            return result;
        }
        cJSON* dProperties = cJSON_GetObjectItemCaseSensitive(root, "d");
        if (dProperties) {
            cJSON* ct = cJSON_GetObjectItemCaseSensitive(dProperties, "ct");
            cmd_type = ct->valueint;

            switch (cmd_type) {
            case 201://ct :201
                printf("Attributes Received ");
                break;
            case 202://ct :202
                printf("Twin Received");
                break;
            case 204://ct :204
                printf("Child Received\n");
                break;
            default:;
            }
            IotConnectEventType type = cmd_type;
            struct IotclEventDataTag* eventData = (struct IotclEventDataTag*)calloc(sizeof(struct IotclEventDataTag), 1);
            if (NULL == eventData) goto cleanup;
            eventData->root = root;
            // FIXME TODO CHECK -- eventData->data should be assigned or removed?;
            eventData->type = type;
            // FIXME TODO CHECK -- memory leak?
            bool result = iotc_process_callback(eventData);
            free(eventData);
            return result;
        }
    }
    cleanup:
    if (root) {
        cJSON_free(root);
    }
    return false;
}

char *iotcl_clone_command(IotclEventData data) {

    cJSON *command = cJSON_GetObjectItem(data->root, "cmd");
    if (NULL == command || !is_valid_string(command)) {
        return NULL;
    }

    return iotcl_strdup(command->valuestring);
}

char *iotcl_clone_download_url(IotclEventData data, size_t index) {
    cJSON *urls = cJSON_GetObjectItemCaseSensitive(data->root, "urls");
    if (NULL == urls || !cJSON_IsArray(urls)) {
        return NULL;
    }
    if ((size_t) cJSON_GetArraySize(urls) > index) {
        cJSON *url = cJSON_GetArrayItem(urls, index);
        if (is_valid_string(url)) {
            return iotcl_strdup(url->valuestring);
        } else if (cJSON_IsObject(url)) {
            cJSON *url_str = cJSON_GetObjectItem(url, "url");
            if (is_valid_string(url_str)) {
                return iotcl_strdup(url_str->valuestring);
            }
        }
    }
    return NULL;
}


char *iotcl_clone_sw_version(IotclEventData data) {

    cJSON *sw = cJSON_GetObjectItem(data->root, "sw");
    if (is_valid_string(sw)) {
        return iotcl_strdup(sw->valuestring);
    }
    return NULL;
}

char *iotcl_clone_hw_version(IotclEventData data) {
    cJSON *ver = cJSON_GetObjectItemCaseSensitive(data->data, "ver");
    if (cJSON_IsObject(ver)) {
        cJSON *sw = cJSON_GetObjectItem(ver, "hw");
        if (is_valid_string(sw)) {
            return iotcl_strdup(sw->valuestring);
        }
    }
    return NULL;
}

char *iotcl_clone_ack_id(IotclEventData data) {
    cJSON *ackid = cJSON_GetObjectItemCaseSensitive(data->data, "ackId");
    if (is_valid_string(ackid)) {
        return iotcl_strdup(ackid->valuestring);
    }
    return NULL;
}

static char *create_ack(
        bool success,
        const char *message,
        IotConnectEventType message_type,
        const char *ack_id) {

    char *result = NULL;

    IotclConfig *config = iotcl_get_config();

    if (!config) {
        return NULL;
    }

    cJSON *ack_json = cJSON_CreateObject();

    if (ack_json == NULL) {
        return NULL;
    }
    /*{
        "dt": "", //Current time stamp in format YYYY-MM-DDTHH:MM:SS.SSSZ
        "d": {
            "ack": "", //Acknowledgement GUID
            "type": 0, //Fixed ? Acknowledgement of Command
            "st": 0, //Status of Acknowledgement Ref
            "msg": "", //Your custom message for acknowledgement
        }
    }*/
    if (!cJSON_AddStringToObject(ack_json, "dt", iotcl_iso_timestamp_now())) goto cleanup;
    {
        cJSON *ack_data = cJSON_CreateObject();
        if (NULL == ack_data) goto cleanup;
        if (!cJSON_AddItemToObject(ack_json, "d", ack_data)) {
            cJSON_Delete(ack_data);
            goto cleanup;
        }
        if (!cJSON_AddStringToObject(ack_data, "ack", ack_id)) goto cleanup;
        if (!cJSON_AddNumberToObject(ack_data, "type", message_type)) goto cleanup;
        if (!cJSON_AddNumberToObject(ack_data, "st", to_ack_status(success, message_type))) goto cleanup;
        if (!cJSON_AddStringToObject(ack_data, "msg", message ? message : "")) goto cleanup;
    }

    result = cJSON_PrintUnformatted(ack_json);

    // fall through
    cleanup:
    cJSON_Delete(ack_json);
    return result;
}

char *iotcl_create_ack_string_and_destroy_event(
        IotclEventData data,
        bool success,
        const char *message
) {
    if (!data) return NULL;
    // already checked that ack ID is valid in the messages
    char *ack_id = cJSON_GetObjectItemCaseSensitive(data->root, "ack")->valuestring;
    char *ret = create_ack(success, message, data->type, ack_id);
    iotcl_destroy_event(data);
    return ret;
}

char *iotcl_create_ota_ack_response(
        const char *ota_ack_id,
        bool success,
        const char *message
) {
    char *ret = create_ack(success, message, DEVICE_OTA, ota_ack_id);
    return ret;
}

void iotcl_destroy_event(IotclEventData data) {
    cJSON_Delete(data->root);
    free(data);
}
