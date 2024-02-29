/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.

 * The user should not be typically including this file, unless they intend to provide some custom handling
 * or can make use of some utility functions provided.
 */
#ifndef IOTCL_INTERNAL_H
#define IOTCL_INTERNAL_H

#include <stdbool.h>
#include <time.h>
#include "cJSON.h"
#include "iotcl.h"

#ifdef __cplusplus
extern "C" {
#endif

// IMPORTANT: Always include this header file into your iotcl_*.c sources so that we compare the version
// BEFORE we failing compilation so that the user can be better informed.
// Some projects already have older conflicting cJSON, and for those we need to ensure that we don't include or compile
// the old cJSON version.
// Version 1.7.13 introduces some return values for certain function calls, and we need those.
#define CJSON_VERSION_COMPOUND  (CJSON_VERSION_MAJOR * 10000 + CJSON_VERSION_MINOR * 100 + CJSON_VERSION_PATCH)
#define CJSON_MIN_VERSION 10713

#if (CJSON_VERSION_COMPOUND < CJSON_MIN_VERSION)
#error "cJSON version must be 1.7.13 or newer"
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
char *iotcl_strdup_json_string(cJSON *cjson, const char *value_name);

// Provides means to locate a path in the form "a.v.c" within cJSON structure.
// See the implementation for more details.
int iotcl_cjson_dot_path_locate(cJSON *root_object, cJSON **parent_object, const char **leaf_name, const char *path);

#ifdef __cplusplus
}
#endif

#endif // IOTCL_INTERNAL_H