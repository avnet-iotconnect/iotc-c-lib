/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
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

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {

} IotclDraJsonConfigResult;

int iotcl_dra_json_config_parse(IotclDraJsonConfigResult* json_confg, const char *json_str);

#ifdef __cplusplus
}
#endif

#endif // ITOCL_DRA_JSON_CONFIG_H
