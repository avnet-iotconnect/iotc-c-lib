/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

#include "cJSON.h"

#include "iotcl.h"
#include "iotcl_internal.h"
#include "iotcl_log.h"
#include "iotcl_dra_identity.h"

// from https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/reference-table/#hellomsg Hello Message & REST API
static const char* iotcl_dra_ec_error_mapping[] = {
        "OK – No Error"
        "Device not found. Device is not whitelisted to platform."
        "Device is not active."
        "Un-Associated. Device has not any template associated with it."
        "Device is not acquired. Device is created but it is in release state."
        "Device is disabled. It’s disabled from broker by Platform Admin"
        "Company not found as SID is not valid"
        "Subscription is expired."
        "Connection Not Allowed."
        "Invalid Bootstrap Certificate."
        "Invalid Operational Certificate."
};

static void iotcl_dra_clear_and_free_mqtt_config(IotclMqttConfig* c) {
    iotcl_free(c->username);
    iotcl_free(c->host);
    iotcl_free(c->client_id);
    iotcl_free(c->pub_rpt);
    iotcl_free(c->pub_ack);
    iotcl_free(c->sub_c2d);
    c->username = NULL;
    c->host = NULL;
    c->client_id = NULL;
    c->pub_rpt = NULL;
    c->pub_ack = NULL;
    c->sub_c2d = NULL;
    c->pub_rpt_len = 0;
    c->pub_ack_len = 0;
    c->sub_c2d_len = 0;
}
static int iotcl_dra_parse_response_and_configure_iotcl(cJSON *json_root) {
    const char *f;
    IotclMqttConfig* c = NULL;
    if (!json_root) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA Identity: Parsing error or ran out of memory while parsing the response!");
        return IOTCL_ERR_PARSING_ERROR;
    }

    f = "status";
    cJSON *j_status = cJSON_GetObjectItem(json_root, f);
    if (!j_status || !cJSON_IsNumber(j_status)) goto cleanup;

    int response_status = cJSON_GetNumberValue(j_status);
    if (200 != response_status) {
        IOTCL_ERROR(IOTCL_ERR_BAD_VALUE, "DRA Identity: Bad response status %d!", response_status);
        return IOTCL_ERR_BAD_VALUE;
    }

    f = "d";
    cJSON *j_d = cJSON_GetObjectItem(json_root, f);
    if (!j_d || !cJSON_IsObject(j_d)) goto cleanup;


    f = "ec";
    cJSON *j_ec = cJSON_GetObjectItem(j_d, f);
    if (!j_ec || !cJSON_IsNumber(j_ec)) goto cleanup;
    int ec = (int)cJSON_GetNumberValue(j_ec);

    if (0 != ec) {
        const char* ec_message;
        if (ec > 0 && ec < (int) sizeof(iotcl_dra_ec_error_mapping)) {
            ec_message = iotcl_dra_ec_error_mapping[ec];
        } else {
            ec_message = "<Unknown Error>";
        }
        IOTCL_ERROR(IOTCL_ERR_FAILED, "DRA Identity: Identity error received %d. Message: %s", ec, ec_message);
        return IOTCL_ERR_BAD_VALUE;
    }

    f = "p";
    cJSON *j_p = cJSON_GetObjectItem(j_d, f);
    if (!j_p || !cJSON_IsObject(j_p)) goto cleanup;

    f = "topics"; // inside "p"
    cJSON *j_topics = cJSON_GetObjectItem(j_p, f);
    if (!j_topics || !cJSON_IsObject(j_topics)) goto cleanup;

    c = iotcl_mqtt_get_config();
    // in case custom config was not used, free everything up
    iotcl_dra_clear_and_free_mqtt_config(c);

    c->username = iotcl_strdup_json_string(j_p, "un");
    c->host = iotcl_strdup_json_string(j_p, "h");
    c->client_id = iotcl_strdup_json_string(j_p, "id");
    c->pub_rpt = iotcl_strdup_json_string(j_topics, "rpt");
    c->pub_ack = iotcl_strdup_json_string(j_topics, "ack");
    c->sub_c2d = iotcl_strdup_json_string(j_topics, "c2d");

    // NOTE: username should be null for aws, but currently identity returns one
    // We don't know whether this is aws or not just based on identity response
    if (!c->host || !c->client_id || !c->pub_rpt || !c->pub_ack || !c->sub_c2d) {
        iotcl_dra_clear_and_free_mqtt_config(c);
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA Identity: One or more response fields was not found or ran out of memory");
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    return IOTCL_SUCCESS;

    cleanup:
    IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA Identity: Error encountered while parsing the response field \"%s\"", f);
    return IOTCL_ERR_PARSING_ERROR;


}

static int iotcl_dra_identity_validate_config(void) {
    if (!iotcl_get_global_config()->is_valid) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "DRA Identity: The library is not configured. Please configure the library in custom mode first.");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    IotclMqttConfig* c = iotcl_mqtt_get_config();
    if (c->host || c->sub_c2d || c->pub_ack || c->pub_rpt || c->client_id || c->username) {
        IOTCL_WARN(IOTCL_ERR_CONFIG_ERROR, "DRA Identity: The library's MQTT configuration should not be set.");
        iotcl_dra_clear_and_free_mqtt_config(c);
        return IOTCL_ERR_CONFIG_ERROR;
    }
    return IOTCL_SUCCESS;
}

// Formats an input base url URL to use to call identity REST API
int iotcl_dra_identity_build_url(IotclDraUrlContext *base_url_context, const char *duid) {
    if (!base_url_context || !iotcl_dra_url_get_url(base_url_context)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA Identity: Base URL is required.");
        return IOTCL_ERR_MISSING_VALUE;
    }
    if (!duid || 0 == strlen(duid)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA Identity: DUID is required.");
        return IOTCL_ERR_MISSING_VALUE;
    }
    int suffix_size = snprintf(NULL, 0, IOTC_DRA_IDENTITY_FORMAT, duid);
    char* suffix = iotcl_malloc(suffix_size + 1);
    sprintf(suffix, IOTC_DRA_IDENTITY_FORMAT, duid);
    int status = iotcl_dra_url_use_suffix_path(base_url_context, suffix); // the called function will report error
    iotcl_free(suffix);
    return status;
}

// Parse an identity response and configure IoTConnect library mqtt settings with the response result
int iotcl_dra_identity_configure_library_mqtt(const char *response_str) {
    int status = iotcl_dra_identity_validate_config();
    if (IOTCL_SUCCESS != status) {
        return status; // the called function will print the error
    }
    cJSON *root = cJSON_Parse(response_str);
    status = iotcl_dra_parse_response_and_configure_iotcl(root);
    cJSON_Delete(root);
    return status;
}

// Parse an identity response and configure IoTConnect library mqtt settings with the response result
int iotcl_dra_identity_configure_library_mqtt_with_length(const uint8_t *response_data, size_t response_data_size) {
    int status = iotcl_dra_identity_validate_config();
    if (IOTCL_SUCCESS != status) {
        return status; // the called function will print the error
    }
    cJSON *root = cJSON_ParseWithLength((const char *)response_data, response_data_size);
    status = iotcl_dra_parse_response_and_configure_iotcl(root);
    cJSON_Delete(root);
    return status;
}
