/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#include "iotcl_log.h"
#include "iotcl_internal.h"
#include "iotcl_util.h"
#include "iotcl.h"

static IotclGlobalConfig config = {0};

static IoTclMallocFunction cfg_malloc_fn = malloc;
static IoTclFreeFunction cfg_free_fn = free;

static bool iotcl_topics_match_cfg(const char *cfg_topic, const char *topic, size_t topic_length) {
    if (!topic || 0 == topic_length || !cfg_topic) {
        return false;
    }
    int part_match = strncmp((const char *) topic, cfg_topic, topic_length);
    if (0 != part_match) {
        return false;
    }

    // then check if cfg_topic is equal the non-null terminated in size also
    return (cfg_topic[topic_length] == 0);
}

static void print_value_if_not_null(const char* heading, const char* value) {
    if (value) IOTCL_INFO("%s: %s", heading, value);
}

void *iotcl_malloc(size_t size) {
    return cfg_malloc_fn(size);
}

void iotcl_free(void *ptr) {
    // do an extra null check to make sure the behavior is defined for NULL
    // in case some odd custom implementation does not handle it.
    if (ptr) {
        cfg_free_fn(ptr);
    }
}

void iotcl_configure_dynamic_memory(IoTclMallocFunction malloc_fn, IoTclFreeFunction free_fn) {
    if (NULL == malloc_fn || NULL == free_fn) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "Both malloc and free functions must be provided");
        return;
    }
    cfg_malloc_fn = malloc_fn;
    cfg_free_fn = free_fn;
    cJSON_Hooks cjson_hooks;
    cjson_hooks.malloc_fn = malloc_fn;
    cjson_hooks.free_fn = free_fn;
    cJSON_InitHooks(&cjson_hooks);
}

void iotcl_init_client_config(IotclClientConfig *c) {
    memset(c, 0, sizeof(IotclClientConfig));
}

int iotcl_init(IotclClientConfig *c) {
    int ret;
    iotcl_deinit();

    if (NULL == c) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_init: Client config is NULL");
        return IOTCL_ERR_MISSING_VALUE;
    }

    config.disable_printable_check = c->disable_printable_check;


    // Shortcuts for shorter conditions
    const IotclDeviceConfigType itype = c->device.instance_type;

    if (itype <= IOTCL_DCT_UNDEFINED || itype >= IOTCL_DCT_MAX) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_ERROR, "iotcl_init: Invalid instance config type %d!", itype);
        return IOTCL_ERR_CONFIG_ERROR;
    }

    const bool is_custom = (itype == IOTCL_DCT_CUSTOM);
    const bool is_azure = (itype == IOTCL_DCT_AZURE_DEDICATED || itype == IOTCL_DCT_AZURE_SHARED);
    const bool is_shared = (itype == IOTCL_DCT_AWS_SHARED || itype == IOTCL_DCT_AZURE_SHARED);

    if (!is_custom && (NULL == c->device.duid || 0 == strlen(c->device.duid))) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_init: DUID is required");
        return IOTCL_ERR_MISSING_VALUE;
    }

    if (is_shared && (NULL == c->device.cpid || 0 == strlen(c->device.cpid))) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_init: CPID is required for shared instance configuration");
        return IOTCL_ERR_MISSING_VALUE;
    }

    if (is_azure && (NULL == c->device.cd || 0 == strlen(c->device.cd))) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_init: CD is required for azure instance configuration");
        return IOTCL_ERR_MISSING_VALUE;
    }

    memcpy(&config.event_functions, &c->events, sizeof(config.event_functions));
    config.time_fn = c->time_fn;
    config.mqtt_send_cb = c->mqtt_send_cb;

    // MQTT configuration is not processed for custom configs, so skip it altogether to simplify the logic below
    if (is_custom) {
        config.is_valid = true;
        return IOTCL_SUCCESS;
    }

    char *p; // "that last string allocated" - shortcut for OOM checks and sprintfs
    if (is_shared) {
        p = config.mqtt_config.client_id = iotcl_malloc(strlen(c->device.duid) + strlen(c->device.cpid) + 1);
        if (!p) goto cleanup_print_oom;
        strcpy(p, c->device.cpid);
        strcat(p, "-");
        strcat(p, c->device.duid);
    } else {
        p = config.mqtt_config.client_id = iotcl_strdup(c->device.duid);
        if (!p) goto cleanup_print_oom;
    }
    if (is_azure) {
        // we use snprintf with null to calculate buffer size
        p = config.mqtt_config.username = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AZURE_USERNAME_FORMAT,
                             config.mqtt_config.host,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AZURE_USERNAME_FORMAT, config.mqtt_config.host, config.mqtt_config.client_id);

        p = config.mqtt_config.pub_rpt = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AZURE_PUB_RPT_FORMAT,
                             config.mqtt_config.host,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AZURE_PUB_RPT_FORMAT, config.mqtt_config.client_id, c->device.cd);
        config.mqtt_config.pub_rpt_len = strlen(p);

        p = config.mqtt_config.pub_ack = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AZURE_PUB_ACK_FORMAT,
                             config.mqtt_config.client_id,
                             c->device.cd
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AZURE_PUB_ACK_FORMAT, config.mqtt_config.client_id, c->device.cd);
        config.mqtt_config.pub_ack_len = strlen(p);

        p = config.mqtt_config.sub_c2d = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AZURE_SUB_C2D_FORMAT,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AZURE_SUB_C2D_FORMAT, config.mqtt_config.client_id);
        config.mqtt_config.sub_c2d_len = strlen(p);

    } else {
        p = config.mqtt_config.pub_rpt = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AWS_PUB_RPT_FORMAT,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AWS_PUB_RPT_FORMAT, config.mqtt_config.client_id);
        config.mqtt_config.pub_rpt_len = strlen(p);

        p = config.mqtt_config.pub_ack = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0,IOTCL_AWS_PUB_ACK_FORMAT,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AWS_PUB_ACK_FORMAT, config.mqtt_config.client_id);
        config.mqtt_config.pub_ack_len = strlen(p);

        p = config.mqtt_config.sub_c2d = iotcl_malloc(
                1 + (size_t) snprintf(NULL, 0, IOTCL_AWS_SUB_C2D_FORMAT,
                             config.mqtt_config.client_id
                )
        );
        if (!p) goto cleanup_print_oom;
        sprintf(p, IOTCL_AWS_SUB_C2D_FORMAT, config.mqtt_config.client_id);
        config.mqtt_config.sub_c2d_len = strlen(p);
    }

    config.is_valid = true;
    return IOTCL_SUCCESS;

    cleanup_print_oom:
    IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "iotcl_init: Out of memory error while allocating topic strings!");
    ret = IOTCL_ERR_OUT_OF_MEMORY;

    // free up everything and invalidate the config
    iotcl_deinit();

    return ret; // default IOTCL_ERR_OUT_OF_MEMORY, if not set
}

int iotcl_init_and_print_config(IotclClientConfig *c) {
    int status = iotcl_init(c);
    if (IOTCL_SUCCESS != status) {
        return status;
    }
    IOTCL_INFO("-- IOTCL Device Config --");
    print_value_if_not_null("DUID", c->device.duid);
    print_value_if_not_null("CPID", c->device.cpid);
    print_value_if_not_null("CD  ", c->device.cd);
    print_value_if_not_null("Host", c->device.host);
    return IOTCL_SUCCESS;
}

void iotcl_deinit(void) {

    iotcl_free(config.mqtt_config.username);
    iotcl_free(config.mqtt_config.client_id);
    iotcl_free(config.mqtt_config.host);
    iotcl_free(config.mqtt_config.pub_rpt);
    iotcl_free(config.mqtt_config.pub_ack);
    iotcl_free(config.mqtt_config.sub_c2d);

    // config.is_valid = false; after memset
    memset(&config, 0, sizeof(config));
}

IotclGlobalConfig *iotcl_get_global_config(void) {
    if (!config.is_valid) {
        // if the user intended to configure the library topics etc. manually, they can init with "custom" instance type.
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_get_global_config: IotConnect Library is not configured!");
    }
    // still return config, even if error
    return &config;
}

IotclMqttConfig *iotcl_mqtt_get_config(void) {
    if (!config.is_valid) {
        // if the user intended to configure the library topics etc. manually, they can init with "custom" instance type.
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_get_global_config: IotConnect Library is not configured!");
        return NULL;
    }
    return &config.mqtt_config;
}
void iotcl_mqtt_print_config(void) {
    if (!config.is_valid) {
        // if the user intended to configure the library topics etc. manually, they can init with "custom" instance type.
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_get_global_config: IotConnect Library is not configured!");
        return;
    }
    IotclMqttConfig* mc = &config.mqtt_config;
    IOTCL_INFO("-- IOTCL MQTT Config --");
    print_value_if_not_null("Client ID", mc->client_id);
    print_value_if_not_null("Username ", mc->username);
    print_value_if_not_null("Host     ", mc->host);
    print_value_if_not_null("Pub RPT  ", mc->pub_rpt);
    print_value_if_not_null("Pub ACK  ", mc->pub_ack);
    print_value_if_not_null("Sub C2D  ", mc->sub_c2d);
}

int iotcl_mqtt_send_telemetry(IotclMessageHandle msg, bool pretty) {
    if (!config.is_valid) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_telemetry: Library not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_config.pub_rpt) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_telemetry: pub_rpt topic is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_send_cb) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_telemetry: mqtt_send_cb callback is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    char * json_str = iotcl_telemetry_create_serialized_string(msg, pretty);
    if (!json_str) {
        return IOTCL_ERR_FAILED; // called function will print the error
    }
    config.mqtt_send_cb(config.mqtt_config.pub_rpt, config.mqtt_config.pub_rpt_len, json_str);
    iotcl_telemetry_destroy_serialized_string(json_str);
    return IOTCL_SUCCESS;
}

int iotcl_mqtt_send_ota_ack(const char *ack_id, int ota_status, const char *message) {
    if (!config.is_valid) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_ota_ack: Library not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_config.pub_ack) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_ota_ack: pub_ack topic is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_send_cb) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_ota_ack: mqtt_send_cb callback is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    char *json_str = iotcl_c2d_create_ota_ack_json(ack_id, ota_status, message);
    if (!json_str) {
        return IOTCL_ERR_FAILED; // called function will print the error
    }
    config.mqtt_send_cb(config.mqtt_config.pub_ack, config.mqtt_config.pub_ack_len, json_str);
    iotcl_c2d_destroy_ack_json(json_str);
    return IOTCL_SUCCESS;
}

int iotcl_mqtt_send_cmd_ack(const char *ack_id, int cmd_status, const char *message) {
    if (!config.is_valid) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_cmd_ack: Library not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_config.pub_ack) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_cmd_ack: pub_rpt topic is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!config.mqtt_send_cb) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_send_cmd_ack: mqtt_send_cb callback is not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    char *json_str = iotcl_c2d_create_cmd_ack_json(ack_id, cmd_status, message);
    if (!json_str) {
        return IOTCL_ERR_FAILED; // called function will print the error
    }
    config.mqtt_send_cb(config.mqtt_config.pub_ack, config.mqtt_config.pub_ack_len, json_str);
    iotcl_c2d_destroy_ack_json(json_str);
    return IOTCL_SUCCESS;
}

int iotcl_mqtt_receive(const char *topic_name, size_t topic_len, const char *str) {
    if (!config.is_valid) {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_MISSING, "iotcl_mqtt_receive: Library not configured!");
        return IOTCL_ERR_CONFIG_MISSING;
    }
    if (!iotcl_is_printable("iotcl_mqtt_receive_with_length: topic_name", topic_name, topic_len)) {
        return IOTCL_ERR_BAD_VALUE;
    }
    if (!iotcl_topics_match_cfg(config.mqtt_config.sub_c2d, topic_name, topic_len)) {
        return IOTCL_ERR_IGNORED;
    }
    return iotcl_mqtt_receive_c2d(str);
}

int iotcl_mqtt_receive_with_length(const char *topic_name, size_t topic_len, const uint8_t *data, size_t data_len) {
    if (!config.is_valid) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "iotcl_mqtt_receive_with_length: Library not configured!");
        return IOTCL_ERR_CONFIG_ERROR;
    }
    if (!iotcl_is_printable("iotcl_mqtt_receive_with_length: topic_name", topic_name, topic_len)) {
        return IOTCL_ERR_BAD_VALUE;
    }
    if (!iotcl_topics_match_cfg(config.mqtt_config.sub_c2d, topic_name, topic_len)) {
        return IOTCL_ERR_IGNORED;
    }
    return iotcl_mqtt_receive_c2d_with_length(data, data_len);
}

int iotcl_mqtt_receive_c2d(const char *str) {
    if (!iotcl_is_printable("iotcl_mqtt_receive: str", str, strlen(str))) {
        return IOTCL_ERR_BAD_VALUE;
    }
    return iotcl_c2d_process_event(str);
}

int iotcl_mqtt_receive_c2d_with_length(const uint8_t *data, size_t data_len) {
    if (!iotcl_is_printable("iotcl_mqtt_receive_with_length: str", (const char *) data, data_len)) {
        return IOTCL_ERR_BAD_VALUE;
    }
    return iotcl_c2d_process_event_with_length(data, data_len);
}

