/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_LIB_H
#define IOTCL_LIB_H

/* ---- LIBRARY GENERAL RULES AND CONCEPTS ----
 * This library's functions generally return error codes defined in iotcl_constants.h as int,
 * unless otherwise noted or obvious.
 *
 * The library requires dynamic memory allocation. Lifetimes of values allocated on heap are specified in corresponding
 * functions and where the user may or may not be responsible for freeing those values with noted functions.
 * Functions which return values that need to be freed are generally prefixed with "create" or "clone".
 * Otherwise they are prefixed with "get" or other general prefixes.
 *
 * ---- THREAD SAFETY AND REENTRANCY ----
 * The library's functions implementations are implemented with thread safety and reentrancy in mind,
 * unless otherwise noted.
 * iotcl_init() and iotcl_deinit() are making modifications to a global configuration instance and are not thread safe,
 * so they should be handled with care and stage before and after device connections are started/terminated or only once
 * at application startup (no need to deinit).
 *
 * ---- DEVICE CONFIGURATION GUIDE ----
 * To determine how to set the IotclDeviceConfigType value, one needs to understand the difference between the
 * two the IoTConnect instance types and how to supply appropriate parameters:
 * - Accounts may be on shared or dedicated instances.
 * - In addition to being needed for MQTT connect, the MQTT Client ID is a part of the topics format
 *    and the MQTT username in case of Azure IoTHub connections. The Client ID can be found in the Connection Info
 *    tab at your device's Info panel at the IoTConnect web site.
 * - If on dedicated instance, the MQTT Client ID is the same as the device ID.
 * - If on shared instance (trial account etc.), the client ID in the info panel will be prefixed with your CPID.
 *   In this case the library will need the CPID value configured in order to construct the MQTT Client ID string.
 * - Azure accounts require the "cd" value. The "cd" value is a unique 7-8 character code
 *   that corresponds to your device ID.
 *   For Azure connections we recommend using the IOTCL_DCT_CUSTOM configuration in conjunction with
 *   the identity response (discovery REST API) to provide the configuration values,
 *   not all devices can easily do HTTPS so we provide the two Azure instance configurations for convenience.
 *   in hopes that this behavior (the cd value) will change in the future(as of 8th of February 2024).
 *  - While not recommended for Azure instances, for single device testing or demos, you may decide to use
 *    IOTCL_DCT_AZURE_SHARED and OTCL_DCT_AZURE_DEDICATED configuration methods along with your device's CD value.
 *
 * Azure accounts also require the MQTT endpoint hostname in order to construct the username to be used
 * for the mqtt connection. See IOTCL_AZURE_USERNAME_FORMAT in iotcl_constants.h. The host string passed will be cloned
 * and permanently kept in MQTT config when passed -- or if passed in case of AWS.
 *
 * To determine if you are on a dedicated or shared instance you can examine the Connection Info panel at the
 * IoTConnect web site and follow this simple guide:
 *  - If your account is on a shared instance, device's clientID will only contain the device ID.
 *  - If your account is on a shared instance, device's clientID will be prefixed by your CPID.
 *
 *
 * Here are some example reporting topics that will be generated depending on which back end cloud solution is used
 * (AWS/Azure) and which IoTConnect instance type is used:
 *  IOTCL_DCT_AWS_DEDICATED:
 *      Reporting Topic: $aws/rules/msg_d2c_rpt/mydevice/2.1/0
 *  IOTCL_DCT_AWS_SHARED:
 *      Reporting Topic: $aws/rules/msg_d2c_rpt/mycpid-mydevice/2.1/0
 *  IOTCL_DCT_AZURE_DEDICATED:
 *      $devices/mydevice/messages/events/cd=MYCD123&v=2.1&mt=0
 *  IOTCL_DCT_AZURE_SHARED:
 *      "devices/mycpid-mydevice/messages/events/cd=MYCD123&v=2.1&mt=0",
 *      "devices/mycpid-mydevice/messages/events/cd=MYCD123&v=2.1&mt=6",
 *      "devices/mycpid-mydevice/messages/devicebound/#");
 *  Reporting Topic: devices/mycpid-mydevice/messages/events/cd=MYCD123&v=2.1&mt=0
 *
 * ---- TIME CONFIGURATION GUIDE ----
 *
 * Provide the optional time function that should return GMT date and time of day defined as
 * a timestamp compatible with the time() function from time.h. Internally, the library will use the function to obtain
 * time_t value and convert it to ISO format using "struct tm" and in order to timestamp the messages.
 *
 * Please be aware of the Year 2038 problem and provide a portable way to convert struct timeval
 * into the appropriate time_t value. We also recommend that you test your device's
 * standard library (typically newlib) compatibility with dates after 2038.
 *
 * Even if your device has the ability to get time, you may want to NOT set this in order to preserve bandwidth
 * by not sending the timestamp and let the back end timestamp the events as they arrive.
 * If your device has NTP support, or a battery backed clock, you can set this function.
 * If you do not provide the time function, telemetry messages will be timestamped as they received by the server.
 * Also note that if no time function is provided, attempting to add more than one telemetry values set into a single
 * telemetry message may not make much sense unless unless each is timestamped individually.
 *
 * If your device can directly utilize the time() function from <time.h> to obtain time of day at GMT,
 * simply pass iotcl_default_time() from iotcl_util.h.
 *
 * Also see https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages/#Device
 * and https://en.wikipedia.org/wiki/Year_2038_problem
 */

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "iotcl_cfg.h"
#include "iotcl_c2d.h"
#include "iotcl_telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*IoTclMallocFunction)(size_t size);

typedef void (*IoTclFreeFunction)(void *ptr);

typedef void (*IotclMqttTransportSend)(const char *topic, const char *json_str);

typedef time_t (*IotclTimeFunction)(void);

// This structure's instance is a part of IoTConnect library's global configuration and is
// permanently kept by the library after iotcl_init() is called, and until iotcl_deinit().
// The client can use provided values in order to configure their mqtt client.
// The client can also manually provide this structure's values (along wth RAM storage) when using IOTCL_MQTT_CFG_CUSTOM
// via Identity REST API module or other means. In this case, initially all values in this structure will be NULL.
// When using Azure IoT C Device SDK or Azure RTOS, the client has no control over the topics, so
// certain message "properties" (like "cd" and "version" need to be configured with every message).
typedef struct {
    char *client_id;    // Value is "DUID" for all dedicated instances, "CPID-DUID" for shared.
    char *username;     // For AWS, username will be NULL. It is not required for the MQTT connection. Ignore this value for AWS Custom config with Identity API.
    char *host;         // Only available in custom configuration (via Identity REST API module or other means).
    char *pub_rpt;      // MQTT topic for reporting (telemetry) publishing.
    char *pub_ack;      // MQTT topic for acknowledgement publishing.
    char *sub_c2d;      // MQTT topic for receiving C2D commands.
    char *cd;           // The "CD" value that can be used with AzureRTOS and similar to configure main topic "properties" (Azure concept)
    char *version;      // The "protocol ver" value that can be used with AzureRTOS and similar to configure main topic "properties" (Azure concept)
} IotclMqttConfig;

// See DEVICE CONFIGURATION GUIDE in the header of this file.
typedef enum {
    IOTCL_DCT_UNDEFINED = 0, // was not set and will cause an error
    IOTCL_DCT_AWS_SHARED,
    IOTCL_DCT_AWS_DEDICATED,
    IOTCL_DCT_AZURE_SHARED,
    IOTCL_DCT_AZURE_DEDICATED,

    // Use this for custom or with Discovery/Identity Rest API module.
    // When using this option, the configuration can be obtained by calling iotcl_get_global_config() in iotcl_internal.h
    IOTCL_DCT_CUSTOM,

    // used for error/sanity checking
    IOTCL_DCT_MAX

} IotclDeviceConfigType;

// Specify device values that will be used to construct topics, username, client_id etc.
// See DEVICE CONFIGURATION GUIDE in the header of this file.
// For MQTT port, always use IOTCL_MQTT_PORT
// For MQTT password, when using X509, set to NULL/empty, depending on how your MQTT client deals with this.
// If using Symmetric Key authentication on Azure, the MQTT password computation based on the key
// is outside the scope of this library (for now) since it requires some complex algorithms to compute it.
typedef struct {
    IotclDeviceConfigType instance_type;
    const char *duid; // Always required, unless using IOTCL_MQTT_CFG_CUSTOM
    const char *cpid; // Set this value if using shared instances (IOTCL_DCT_AWS_SHARED or IOTCL_DCT_AZURE_SHARED)
    const char *cd;   // Set this value if using Azure (IOTCL_DCT_AZURE_SHARED or IOTCL_DCT_AZURE_DEDICATED)
    const char *host; // Set this value if using Azure (IOTCL_DCT_AZURE_SHARED or IOTCL_DCT_AZURE_DEDICATED)
} IotclDeviceConfig;

typedef struct {

    // See DEVICE CONFIGURATION GUIDE at the header of this file.
    IotclDeviceConfig device;

    // Event callbacks for c2d message (events) processing. See iotcl_event.h.
    // Optional, but required for event handling - commands or OTA respectively.
    IotclEventConfig events;

    // Optional callback function that will be called when you call send functions like iotcl_mqtt_send_telemetry().
    // If this is not, you can manually convert and process data using available functions in iotcl_telemetry.h.
    // Most MQTT clients will accept a unit8_t* pointer and length rather than a null-terminated string.
    // Simply cast the pointer and run strlen() on the received string before forwarding.
    IotclMqttTransportSend mqtt_send_cb;

    // Optional. See TIME CONFIGURATION GUIDE at the header of this file.
    IotclTimeFunction time_fn;

    // This QOL check can be disabled in case of some special requirements.
    // Received string characters from MQTT are checked against isprint(), isspace() and newline and warning is printed
    // if they are not printable, but could fail on some untested locales.
    // This check can detect garbled strings, but could be a deterrent for some cases.
    bool disable_printable_check;
} IotclClientConfig;

/* Optional malloc and free alternatives.
 * Call this function at runtime BEFORE calling any other IoTConnect library functions - before iotcl_init
 * or any other module's function - to redirect malloc to your own custom implementation.
 * The configuration will be forwarded to the cJSON dependency as well.
 * If not supplied, malloc() and free() will be used.
 *  FreeRTOS: Should use pvPortMalloc and pvPortFree.
 *  AzureRTOS: Should supply your own interface making use of use tx_byte_allocate and tx_byte_release
 *      See https://embeddedartistry.com/blog/2017/02/17/implementing-malloc-with-threadx/ for an example
 */
void iotcl_configure_dynamic_memory(IoTclMallocFunction malloc_fn, IoTclFreeFunction free_fn);

// Initializes a local reference to config with defaults.
// Call this function before calling iotcl_configure().
void iotcl_init_client_config(IotclClientConfig *c);

// Sets up the library's global configuration instance per passed local configuration instance.
// User is not responsible for maintaining memory references to any of the provided values in the IotclClientConfig object.
int iotcl_init(IotclClientConfig *c);

// Same as iotcl_init(), but prints a device config summary to help troubleshoot issue.
int iotcl_init_and_print_config(IotclClientConfig *c);

// Call this to release any memory references maintained by the library (including topics etc)
void iotcl_deinit(void);

// Returns the MQTT topics for this device. NULL, if not configured.
IotclMqttConfig *iotcl_mqtt_get_config(void);

// Prints the current IoTConnect mqtt config if the library is configured. Could be useful for troubleshooting.
// If value is null, it is not printed.
void iotcl_mqtt_print_config(void);

// Send a telemetry message constructed with iotcl_telemetry_create()
// Call this only if mqtt_send_cb is configured. Otherwise parse the messages manually using the iotcl_event.h functions.
int iotcl_mqtt_send_telemetry(IotclMessageHandle msg, bool pretty);

// Call this only if mqtt_send_cb is configured. Otherwise parse the messages manually using the iotcl_event.h functions.
int iotcl_mqtt_send_ota_ack(
        const char *ack_id, // Required. Received in the OTA callback.
        int ota_status,     // See iotcl_event.h for OTA status values
        const char *message // Optional message to be sent along with the ack. Set to NULL or empty if no message.
);

// Call this only if mqtt_send_cb is configured. Otherwise parse the messages manually using the iotcl_event.h functions.
int iotcl_mqtt_send_cmd_ack(
        const char *ack_id, // Required. Received in the command callback.
        int cmd_status,     // See iotcl_event.h for command status values
        const char *message // Optional message to be sent along with the ack. Set to NULL or empty if no message.
);

// iotcl_mqtt_receive* functions are a safe way to route the inbound messages to appropriate subsystems,
// or ignore the message based on the topic supplied, in case a common inbound MQTT message entry point is used.
// As opposed to processing the messages directly with functions in iotcl_c2d.h (or future shadow/twin implementations)
// these functions offer a single entry point for any such uses and provide an optional topic check.
// Function flavors:
//   iotcl_mqtt_receive              - Process a null terminated string data.
//   iotcl_mqtt_receive_with_length  - Process a byte buffer with length (most common use case with MQTT clients)
// The library will attempt to parse the message and route the call to the appropriate subsystem
// depending on the topic name, or silently ignore it and return IOTCL_ERR_IGNORED.
// The functions will also validate data and topic names to ensure they contain only printable characters.
int iotcl_mqtt_receive(const char *topic_name, const char *str);

int iotcl_mqtt_receive_with_length(const char *topic_name, const uint8_t *data, size_t data_len);

// If if your client does not pass the topic name or you have dedicated callbacks for each subscribed topic, then use the
// iotcl_mqtt_receive_c2d* functions. The functions will also validate data to ensure they contain only printable characters.
int iotcl_mqtt_receive_c2d(const char* str);

int iotcl_mqtt_receive_c2d_with_length(const uint8_t *data, size_t data_len);


///////////////////////////////////////////////////////////////////////////////////////////////////////////

// The user should not generally call this function, but it is provided for convenience, and for internal use,
// or custom configuration memory allocation.
// This function will redirect to iotcl_configure_dynamic_memory() configured values, if provided.
void *iotcl_malloc(size_t size);

// The user should not generally call this function, but it is provided for convenience, and for internal use,
// or custom configuration memory allocation.
// This function will redirect to iotcl_configure_dynamic_memory() configured values, if provided.
void iotcl_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif //IOTCL_LIB_H
