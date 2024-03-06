/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_CFG_H
#define IOTCL_CFG_H

// MBEDTLS config file style - include your own to override the config. See iotcl_example_config.h
#if defined(IOTCL_USER_CONFIG_FILE)
#include IOTCL_USER_CONFIG_FILE
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Error values for function returns and error codes passed to IOTCL_ERROR and similar macros
// IOTCL_SUCCESS  is used for readability purposes only. The client does not need to compare against IOTCL_SUCCESS.
// "if (status)" and similar comparisons against zero (being success) are safe constructs to use
#define IOTCL_SUCCESS               0
#define IOTCL_ERR_FAILED            1 // Generic Error that doesn't fit into other categories
#define IOTCL_ERR_BAD_VALUE         2 // A value/argument passed to configuration/function is incorrect
#define IOTCL_ERR_MISSING_VALUE     3 // A value/argument passed to configuration/function is missing
#define IOTCL_ERR_OUT_OF_MEMORY     4 // There was not enough memory to allocate
#define IOTCL_ERR_PARSING_ERROR     5 // Not able to parse the jSON data. NOTE: Failure could have happened due to OOM
#define IOTCL_ERR_CONFIG_ERROR      6 // Global config or a required component is not configured correctly.
#define IOTCL_ERR_CONFIG_MISSING    7 // Global config or an optional config item is missing - specific call cannot complete.
#define IOTCL_ERR_OVERFLOW          8 // A buffer or a value would overflow.
#define IOTCL_ERR_IGNORED           9 // A message or an event is ignored. Eg. topic not intended, or command without ack.


// -------  TIME FORMATTING -------
// strftime Format and Size of a buffer that can accommodate a string like "YYYY-MM-DDTHH:MM:SS.000Z" (including null).
#define IOTCL_ISO_TIMESTAMP_FORMAT "%Y-%m-%dT%H:%M:%S.000Z"
#define IOTCL_ISO_TIMESTAMP_STR_LEN (sizeof("2024-01-02T03:04:05.006Z") - 1)

// ------- STRTOK --------
// If using pure c99 standard or if your platform doesn't have reentrant strtok (strtok_r)
// you can route to strtok with a custom function, but ensure that it's called in a thread-safe manner,
// or add your own custom implementation. See an example in iotcl_config.h in tests/unit/
#ifndef IOTCL_STRTOK_R
#define IOTCL_STRTOK_R strtok_r
#endif


// -------  MQTT TOPIC FORMATS AND DEFINES -------
// Always use secure MQTT port
#define IOTCL_MQTT_PORT 8883

// The first %s in topics is the generated client_id, which is "cpid-duid" in case of shared instances.
#define IOTCL_AWS_PUB_RPT_FORMAT "$aws/rules/msg_d2c_rpt/%s/2.1/0"
#define IOTCL_AWS_PUB_ACK_FORMAT "$aws/rules/msg_d2c_ack/%s/2.1/6"
#define IOTCL_AWS_SUB_C2D_FORMAT "iot/%s/cmd"

#define IOTCL_AZURE_PUB_RPT_FORMAT "devices/%s/messages/events/cd=%s&v=2.1&mt=0"
#define IOTCL_AZURE_PUB_ACK_FORMAT "devices/%s/messages/events/cd=%s&v=2.1&mt=6"
#define IOTCL_AZURE_SUB_C2D_FORMAT "devices/%s/messages/devicebound/#"

// Eg. poc-iotconnect-iothub-030-eu2.azure-devices.net/mycpid-myduid/?api-version=2018-06-30
#define IOTCL_AZURE_USERNAME_FORMAT "%s/%s/?api-version=2018-06-30"


// --------------- LIMITS DEFINITIONS ---------------
// These definitions are not currently utilizaed by the library but can be used by the client code
#define IOTCL_CONFIG_DUID_MAX_LEN 64
#define IOTCL_CONFIG_CPID_MAX_LEN 63
#define IOTCL_CONFIG_ENV_MAX_LEN 20

// IoTHub and AWS IoT Core max device id is 128, which is "<CPID>-<DUID>" (with a dash)
#define IOTCL_CONFIG_CLIENTID_MAX_LEN 128

// Useful define: This value is not used in the actual library, but the user code can reference
// it if it needs to store ACKs in flash to be able to send a success/failure after reboot
#define IOTCL_MAX_ACK_LENGTH 36

#ifdef __cplusplus
}
#endif

#endif
