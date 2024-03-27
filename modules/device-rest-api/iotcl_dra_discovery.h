/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 */

#ifndef ITOCL_DRA_DISCOVERY_H
#define ITOCL_DRA_DISCOVERY_H

#include <stdint.h>
#include <stddef.h>
#include "iotcl_dra_url.h"

#ifdef __cplusplus
extern "C" {
#endif

// Default values for discovery hosts for each cloud end type
// Not used in the library, but useful defines for the client
// Use iotcl_dra_create_discovery_url_with_host if using a different discovery endpoint
#define IOTCL_DRA_DEFAULT_DISCOVERY_HOST_AWS "awsdiscovery.iotconnect.io"
#define IOTCL_DRA_DEFAULT_DISCOVERY_HOST_AZURE "discovery.iotconnect.io"

// Formats a URL context with a discovery URL to use for a given cpid and environment for default AWS discovery URL
// Free the returned URL with iotcl_dra_url_deinit() if the function returns success
int iotcl_dra_discovery_init_url_aws(IotclDraUrlContext *discovery_url, const char *cpid, const char *env);

// Formats a URL context with a discovery URL to use for a given cpid and environment for default Azure discovery URL
// Free the returned URL with iotcl_dra_url_deinit() if the function returns success
int iotcl_dra_discovery_init_url_azure(IotclDraUrlContext *discovery_url, const char *cpid, const char *env);

// Creates a URL context with a discovery URL to use for a given cpid and environment for a specific discovery host
// Free the returned URL with iotcl_dra_url_deinit() if the function returns success
int iotcl_dra_discovery_init_url_with_host(IotclDraUrlContext *discovery_url, char *host, const char *cpid, const char *env);

// Parse a discovery json response and return a base url to use with iotcl_dra_url.h
// Free the returned URL with iotcl_dra_url_deinit() when done with it if the function returns success.
// Pass non-zero slack if you want to prevent base URL buffer reallocations when calling other API from base URL
// by appending a suffix to base URL.
// If only calling the identity API, you can add strlen(IOTCL_DRA_IDENTITY_PREFIX) + strlen(duid).
// If unsure, pass zero. See iotcl_dra_url_use_suffix_path() for more info.
int iotcl_dra_discovery_parse(IotclDraUrlContext *base_url, int base_url_slack, const char *response_str);

// same as iotcl_dra_discovery_parse(), but with data and data length
int iotcl_dra_discovery_parse_with_length(
        IotclDraUrlContext *base_url,
        int base_url_slack,
        const uint8_t *response_data,
        size_t response_data_size
);


#ifdef __cplusplus
}
#endif

#endif // ITOCL_DRA_DISCOVERY_H
