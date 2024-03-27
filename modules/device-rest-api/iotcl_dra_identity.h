/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 */

#ifndef ITOCL_DRA_IDENTITY_H
#define ITOCL_DRA_IDENTITY_H

#include <stdint.h>
#include "iotcl_dra_url.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOTCL_DRA_IDENTITY_PREFIX "/uid/"
#define IOTC_DRA_IDENTITY_FORMAT IOTCL_DRA_IDENTITY_PREFIX"%s"

// Formats an input base url URL to use to call identity REST API
int iotcl_dra_identity_build_url(IotclDraUrlContext *base_url_context, const char *duid);

// Parse an identity response and configure IoTConnect library mqtt settings with the response result
int iotcl_dra_identity_configure_library_mqtt(const char *response_str);

// Parse an identity response and configure IoTConnect library mqtt settings with the response result
int iotcl_dra_identity_configure_library_mqtt_with_length(const uint8_t *response_data, size_t response_data_size);



#ifdef __cplusplus
}
#endif

#endif // ITOCL_DRA_IDENTITY_H
