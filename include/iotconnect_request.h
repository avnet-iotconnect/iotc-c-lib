/* Copyright (C) 2021 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_REQUEST_H
#define IOTCONNECT_REQUEST_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // Can be copied at the device page. Can be obtained via REST Sync call.
    // SDK ID from Web UI KeyVault->Settings Your SDK pulldown
    const char *sid;
} IotclRequestConfig;


// Call this to request create the JSON hello message and start communicating to IoTConnect in v2.0 format,
// once it isent to IoTConnect.
// Call iotcl_request_destroy() to destroy the returned string once it's no longer needed.
// Call iotcl_process_event() to process and receive the response.
char* iotcl_request_create_hello(void);

// Call this function to destroy any returned string from iotcl_request_create* calls.
void iotcl_request_destroy(char *serialized_string);

#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_REQUEST_H
