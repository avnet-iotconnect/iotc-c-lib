/* Copyright (C) 2022 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 */

#ifndef IOTCONNECT_DISCOVERY_V3_H
#define IOTCONNECT_DISCOVERY_V3_H

#define DISCOVERY_V3_HOST_NAME "discovery.iotconnect.io"
#define DISCOVERY_V3_RESOURCE_FORMAT "/api/v3.1/dsdk/cpId/%s/env/%s"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IotclDiscoveryV3Response {
	int ec; // error code. 0 = success.
	char *bu; // Agent Host base URL. Append resource path to this returned URL
	char *message; // message or error message
} IotclDiscoveryV3Response;

IotclDiscoveryV3Response * iotcl_parse_discovery_v3_response(const char* response_data);

void iotcl_free_discovery_v3_response(IotclDiscoveryV3Response *response);

#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_DISCOVERY_V3_H
