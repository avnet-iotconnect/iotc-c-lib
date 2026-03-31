/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains the functions that can be used to parse the credentials
 * response by using /IOTCONNECT's mutual TLS authentication.
 * The REST endpoints are availabe in the c-lib's config from Device Identity response.
 * Pass your device private key and certificate to the endpoint to get the credentials for MQTT connection.
 */

#ifndef IOTCL_DRA_CREDENTIALS_H
#define IOTCL_DRA_CREDENTIALS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Not used by the library itself, but useful for the user:
// When requesting credentials with mutual TLS, pass this header
// with value of the client_id (AKA thing name) from c-lib's config.
#define IOTCL_DRA_THING_NAME_HEADER "x-amzn-iot-thingname"

// Structure to hold the credentials result from the credentials request
// and expiration,
// NOTE: The original expiration string is provided in case the caller wants to print the value or
// wants to use "proper" ISO 8601 conversion rather than our own low-level algorithm
typedef struct {
    char *access_key_id;
    char *secret_access_key;
    char *session_token;
    char *expiration_str; // Original expiration string from JSON
    time_t expiration;    // Expiration time in epoch seconds
} IotclDraCredentialsResult;

// Parse the credentials request result and populate the result structure.
// Returns IOTCL_SUCCESS on success, or an error code on failure.
// On success, the caller is responsible for freeing the result with iotcl_dra_json_credentials_free().
int iotcl_dra_json_credentials_parse(IotclDraCredentialsResult *credentials, const char *json_str);

// Free all allocated strings in the result structure.
void iotcl_dra_json_credentials_free(IotclDraCredentialsResult *credentials);

#ifdef __cplusplus
}
#endif

#endif // IOTCL_DRA_CREDENTIALS_H
