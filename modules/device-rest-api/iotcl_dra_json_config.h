/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 */

#ifndef ITOCL_DRA_JSON_CONFIG_H
#define ITOCL_DRA_JSON_CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *pf;               // Platform: "aws" or "az"
    char *cpid;             // Company ID
    char *env;              // Environment
    char *duid;             // Device Unique ID (from "uid")
    char *client_id;        // Client ID (from "did")
    bool dedicated_instance; // true if strlen(uid) == strlen(did)
} IotclDraJsonConfigResult;

// Parse the iotcDeviceConfig.json and populate the result structure.
// Returns IOTCL_SUCCESS on success, or an error code on failure.
// On success, the caller is responsible for freeing the result with iotcl_dra_json_config_free().
int iotcl_dra_json_config_parse(IotclDraJsonConfigResult *json_config, const char *json_str);

// Free all allocated strings in the result structure.
void iotcl_dra_json_config_free(IotclDraJsonConfigResult *json_config);

#ifdef __cplusplus
}
#endif

#endif // ITOCL_DRA_JSON_CONFIG_H
