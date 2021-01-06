/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_EVENT_H
#define IOTCONNECT_EVENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNKNOWN_EVENT = 0,
    DEVICE_COMMAND = 0x01,
    DEVICE_OTA = 0x02,
    MODULE_UPDATE_COMMAND = 0x03,
    ON_CHANGE_ATTRIBUTE = 0x10,
    ON_CHANGE_SETTING = 0x11,
    ON_FORCE_SYNC = 0x12,
    ON_ADD_REMOVE_DEVICE = 0x13,
    ON_ADD_REMOVE_RULE = 0x15,
    ON_CLOSE = 0x99
} IotConnectEventType;

typedef struct IOTCL_EVENT_DATA_TAG *IOTCL_EVENT_DATA;

typedef void (*IOTCL_MESSAGE_CALLBACK)(IOTCL_EVENT_DATA data, IotConnectEventType type);

typedef void (*IOTCL_OTA_CALLBACK)(IOTCL_EVENT_DATA data);

typedef void (*IOTCL_COMMAND_CALLBACK)(IOTCL_EVENT_DATA data);

//callback configuration for the events module
typedef struct {
    IOTCL_OTA_CALLBACK ota_cb; // callback for OTA events.
    IOTCL_COMMAND_CALLBACK cmd_cb; // callback for command events.
    IOTCL_MESSAGE_CALLBACK msg_cb; // callback for ALL messages, including the specific ones like cmd or ota callback.
    IOTCL_COMMAND_CALLBACK unsupported_cb;   // callback when event that cannot be decoded by the library is received.
} IOTCL_EVENT_FUNCTIONS;


// The user should supply the event received json form the cloud.
// The function will process the received message and will invoke callbacks accordingly.
// If return value is false, there was an error during processing.
bool IOTCL_ProcessEvent(const char *event);

// Returns a malloc-ed copy of the command line message parameter.
// The user must manually free the returned string when it is no longer needed.
char *IOTCL_CloneCommand(IOTCL_EVENT_DATA data);

// Returns a malloc-ed copy of the OTA download URL with a given zero-based index.
// The user must manually free the returned string when it is no longer needed.
char *IOTCL_CloneDownloadUrl(IOTCL_EVENT_DATA data, size_t index);

// Returns a malloc-ed copy of the OTA firmware version.
// The user must manually free the returned string when it is no longer needed.
char *IOTCL_CloneSwVersion(IOTCL_EVENT_DATA data);

// Returns a malloc-ed copy of the OTA hardware version. It only includes the version number, and not the name.
// The user must manually free the returned string when it is no longer needed.
char *IOTCL_CloneHwVersion(IOTCL_EVENT_DATA data);

// Returns a malloc-ed copy of the Ack ID of the OTA message or a command.
// The user must manually free the returned string when it is no longer needed.
char *IOTCL_CloneAckId(IOTCL_EVENT_DATA data);

// Creates an OTA or a command ack json with optional message (can be NULL).
// The user is responsible to free the returned string.
// This function also frees up all resources taken by the message.
char *IOTCL_CreateAckStringAndDestroyEvent(
        IOTCL_EVENT_DATA data,
        bool success,
        const char *message
);

/*
 Creates an OTA ack json with optional message (can be NULL).
 The user is responsible to free the returned string.
 This function should not be generally used to send the ACK in response to an immediate event.
   use IOTCL_CreateAckStringAndDestroyEvent instead.
 This function is intended to be used for the following firmware update use case:
 o Device an OTA and decides that it needs to update software
 o Device does not send an ack. Device should call IOTCL_DestroyEvent
 o Device records the ack ID and desired firmware version and stores them.
 o Device downloads the firmware and restarts.
 o If there is a failure during bring up, the device falls back to the original firmware.
 o Device or service starts up and compares the stored desired firmware version against the actual running version
 o Device sends the ack according to the the above comparison using the stored ack ID.

*/
char *IOTCL_CreateOtaAckResponse(
        const char *ota_ack_id,
        bool success,
        const char *message
);

// Call this is no ack is sent.
// This function frees up all resources taken by the message.
void IOTCL_DestroyEvent(IOTCL_EVENT_DATA data);

#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_EVENT_H
