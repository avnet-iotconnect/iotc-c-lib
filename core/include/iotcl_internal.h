/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.

 * The user should not be typically including this file, unless they intend to provide some custom handling
 * or can make use of some utility functions provided.
 */
#ifndef IOTCL_INTERNAL_H
#define IOTCL_INTERNAL_H

#include <stdbool.h>
#include "iotcl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool is_valid;
    IotclMqttConfig mqtt_config;
    IotclMqttTransportSend mqtt_send_cb;
    IotclEventConfig event_functions;
    IotclTimeFunction time_fn;
    bool disable_printable_check;
} IotclGlobalConfig;

// Generally intended for internal use only where other modules will access the lib's global config instance
// Also intended for IOTCL_DCT_CUSTOM config option.
// The value is guaranteed non-null, but the user should check IotclGlobalConfig.is_valid;
IotclGlobalConfig *iotcl_get_global_config(void);

// A helper function to clone a string from cJSON structure and return NULL if type is invalid etc.
// This simplified version can be used when the field is required.
struct cJSON;
char *iotcl_strdup_json_string(struct cJSON *cjson, const char *value_name);

// A helper function to clone a string from cJSON structure and return NULL if the field does not exist.
// This version can be used when the field is optional
// It returns IOTCL_ERR_OUT_OF_MEMORY if malloc fails.
int iotcl_strdup_json_string_if_exists(char** result, struct cJSON *cjson, const char *value_name);

// Helper: get non-empty string from JSON or return NULL
char *get_json_string(struct cJSON *root, const char *field);

#ifdef __cplusplus
}
#endif

#endif // IOTCL_INTERNAL_H