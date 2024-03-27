/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file provides function that provide c2d message parsing and ability to send acks back to IoTConnect.
 *
 * OTA USE CASES WITH C2D PROCESSING
 * Example simple use:
 *  o Device receives the OTA event and decides whether it needs to update firmware based on SW and/or HW version.
 *  o Device sends a failure ack if it does not need to update firmware based on SW and/or HW version.
 *  o Device downloads the firmware.
 *  o Device sends OTA success ack or a failure ack in case the firmware download fails.
 *  o Device applies the firmware and loads the new firmware.
 *
 * Example Recommended/ideal fully fledged firmware update process:
 *  o Device receives the OTA event and decides whether it needs to update firmware based on SW and/or HW version.
 *  o Device sends a failure ack if it does not need to update firmware based on SW and/or HW version.
 *  o Device records the ack ID and desired firmware version and stores them into persistent storage.
 *  o Device downloads the firmware and restarts.
 *  o If there is a failure during bring up, the device falls back to the original firmware.
 *  o Device self-tests to ensure that it can bring up the new firmware correctly. Sends a failure ack if not.
 *  o Device or service starts up and compares the stored (in step above) firmware version against the actual running version.
 *  o Device sends the ack according to the the above comparison using the stored ack ID.
 */
#ifndef IOTCL_C2D_H
#define IOTCL_C2D_H

#include <stddef.h>

// MBEDTLS config file style - include your own to override the config. See iotcl_example_config.h
#if defined(IOTCL_USER_CONFIG_FILE)
#include IOTCL_USER_CONFIG_FILE
#endif

// Command statuses

#ifndef IOTCL_C2D_ACK_USES_SPEC
#define IOTCL_C2D_EVT_CMD_SUCCESS           7
#define IOTCL_C2D_EVT_CMD_FAILED            4
#define IOTCL_C2D_EVT_CMD_SUCCESS_WITH_ACK  7 // Should not be used in most cases
#else
/* The specification states different values which differ from the actual values and behavior
 * accepted by the back end. If/when the back end changes to comply with the documentation,
 * define IOTCL_C2D_ACK_USES_SPEC in your iotcl_config.h to use values defined by the documentation.
 */
#define IOTCL_C2D_EVT_CMD_SUCCESS           0
#define IOTCL_C2D_EVT_CMD_FAILED            1
#define IOTCL_C2D_EVT_CMD_SUCCESS_WITH_ACK  2
#endif

// OTA Download statuses
// Best practices for OTA status:
// While the final result of the OTA action should be IOTCL_EVT_OTA_SUCCESS,
// it can only be determined only if we are certain that we downloaded the OTA and are guaranteed to be successful.
// A good way to ensure proper result is to send the ack only when the device is up and running OK with the new version.
// This can be left to the interpretation of the user given the device's capabilities and limitations or the need
// for fine-grained status.
#define IOTCL_C2D_EVT_OTA_SUCCESS           0
#define IOTCL_C2D_EVT_OTA_FAILED            1
#define IOTCL_C2D_EVT_OTA_DOWNLOADING       2
#define IOTCL_C2D_EVT_OTA_DOWNLOAD_DONE     3
#define IOTCL_C2D_EVT_OTA_DOWNLOAD_FAILED   4


#ifdef __cplusplus
extern "C" {
#endif

typedef struct IotclC2dEventDataTag *IotclC2dEventData;

typedef void (*IotclOtaCallback)(IotclC2dEventData data);

typedef void (*IotclCommandCallback)(IotclC2dEventData data);

// Callback configuration for the events module.
// NOTE: It is safe to destroy the event data early by calling iotcl_c2d_destroy_event inside the callback
// in order to free up some heap, as long as no other calls other functions in this file are made that depend on event data.
// For more information, see iotcl_c2d_destroy_event().
typedef struct {
    IotclOtaCallback ota_cb;        // callback for OTA events.
    IotclCommandCallback cmd_cb;    // callback for command events.
} IotclEventConfig;

// The user should supply the event received json form the cloud.
// The function will process the received message and will invoke callbacks accordingly.
// Use this function if your data received from the MQTT client is a null terminated string received on the c2d topic.
int iotcl_c2d_process_event(const char *str);

// data is a data buffer received from mQTT.
// Use this function if your data received from the MQTT client is a binary packet buffer with specific length
//  received on the c2d topic. The buffer contents should be a JSON string.
int iotcl_c2d_process_event_with_length(const uint8_t *data, size_t data_len);

// Returns a malloc-ed copy of the command line message parameter.
// The user must manually free the returned string when it is no longer needed.
const char *iotcl_c2d_get_command(IotclC2dEventData data);

// Returns the number of files available for download. The OTA event will always have at least one URL.
int iotcl_c2d_get_ota_url_count(IotclC2dEventData data);

// Returns the OTA download URL with a given zero-based array index.
// NOTE: This is usually a signed URL with limited expiration time. The URL path can be quite long especially in case
// of AWS S3 URLS. In cases where heap is limited, it is recommended to iotcl_strdup or copy it into a buffer
// along with the ACK-id, then call iotcl_c2d_destroy_event()
// immediately before processing the event in order to conserve heap.
const char *iotcl_c2d_get_ota_url(IotclC2dEventData data, int index);

// Extracts the OTA hostname from the OTA download URL with a given zero-based array index.
// This path will include and URL parameters passed. For example, if URL is "https://acme.corp/path?user=me"
// the function will return "acme.corp".
// NOTE: This string will be internally allocated and freed once iotcl_c2d_destroy_event is called.
// The call can NULL is this allocation fails.
const char *iotcl_c2d_get_ota_url_hostname(IotclC2dEventData data, int index);

// Extracts the OTA URL path with leading slash and trailing URL parameters
// from the OTA download URL with a given zero-based array index.
// This path will include and URL parameters passed. For example, if URL is "https://acme.corp/path?user=me"
// the function will return "/path?user=me".
const char *iotcl_c2d_get_ota_url_resource(IotclC2dEventData data, int index);

// Returns the OTA original uploaded filename with a given zero-based array index.
// The the user uploads a file into the OTA Web UI, the name of the file is recorded and can be obtained using this call.
const char *iotcl_c2d_get_ota_original_filename(IotclC2dEventData data, int index);

// Returns the the OTA firmware version that was configured during upload.
const char *iotcl_c2d_get_ota_sw_version(IotclC2dEventData data);

// Returns a malloc-ed copy of the OTA hardware version. It only includes the version number, and not the name.
// The user must manually free the returned string when it is no longer needed.
const char *iotcl_c2d_get_ota_hw_version(IotclC2dEventData data);

// Returns the Acknowledgement ID from the OTA or command (when "receipt required" setting is set in the template).
// If a command tha tis configured in the template without "receipt required" is sent, the return value will be NULL.
// This acknowledgement ID can be used to report the status of OTA or command back to IoTConnect.
// If you need to destroy the event data during the callback processing, you can iotcl_strdup() or copy this ack ID locally.
const char *iotcl_c2d_get_ack_id(IotclC2dEventData data);

// Creates an OTA or a command ack json with optional message (can be NULL).
// The user is responsible to free the returned value with iotcl_c2d_destroy_ack_json().
// Can return NULL if OOM or ack_id is missing
char *iotcl_c2d_create_cmd_ack_json(
        const char *ack_id,   // Required. Received in the command callback, can be obtained with iotcl_c2d_get_ack_id()
        int cmd_status,       // See iotcl_event.h for command status values
        const char *message   // Optional message to be sent along with the ack. Set to NULL or empty if no message.
);

// Creates an OTA ack json with optional message (can be NULL).
// The user is responsible to free the returned value with iotcl_c2d_destroy_ack_json().
// Also see OTA USE CASES WITH C2D PROCESSING section at the comment header of this file.
// Can return NULL if OOM or ack_id is missing
char *iotcl_c2d_create_ota_ack_json(
        const char *ack_id,   // Required. Received in the OTA callback.
        int ota_status,       // See OTA status values in this file
        const char *message   // Optional message to be sent along with the ack. Set to NULL or empty if no message.
);

// Destroy ack returned by iotcl_c2d_create_cmd_ack_json or iotcl_c2d_create_ota_ack_json
// If using the iotcl_mqtt_receive* functions, the user does not need to call this function. It will be done automatically.
void iotcl_c2d_destroy_ack_json(char *ack_json_ptr);

// The iotcl_c2d_process_event() function will set up and automatically destroy the event data.
// The user does not need to call this function unless they want to destroy the event data early
// and free up heap for further processing while still inside the cmd or ota callback.
// If destroying the data early, the user should not call any other functions in this file
// which take event data as argument and should not reference any value obtained by the getters.
void iotcl_c2d_destroy_event(IotclC2dEventData data);


#ifdef __cplusplus
}
#endif

#endif //IOTCL_C2D_H
