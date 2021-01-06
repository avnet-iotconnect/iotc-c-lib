/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_TELEMETRY_H
#define IOTCONNECT_TELEMETRY_H

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // Can be copied at the device page. Can be obtained via REST Sync call.
    // Device template GUID required to send telemetry data.
    const char *dtg;
} IOTCL_TELEMETRY_CONFIG;


typedef struct IOTCL_MESSAGE_HANDLE_TAG *IOTCL_MESSAGE_HANDLE;

/*
 * Create a message handle given IoTConnect configuration.
 * This handle can be used to add data to the message.
 * The handle cannot be re-used and should be destroyed to free up resources, once the message is sent.
 */
IOTCL_MESSAGE_HANDLE IOTCL_TelemetryCreate();

/*
 * Destroys the IoTConnect message handle.
 */
void IOTCL_TelemetryDestroy(IOTCL_MESSAGE_HANDLE message);

/*
 * Creates a new telemetry data set with a given timestamp in ISO 8601 Date and time UTC format:
 * eg. "2020-03-31T21:18:05.000Z"
 * The user can omit calling this function if single data point would be sent.
 * In that case, the current system timestamp will be used
 * @see IOTCL_IsoTimestampNow, IOTCL_ToIsoTimestamp
 */
bool IOTCL_TelemetryAddWithIsoTime(IOTCL_MESSAGE_HANDLE message, const char *time);

/*
 * Creates a new telemetry data set with a given timestamp in unix time format.
 * The user can omit calling this function if single data point would be sent.
 * In that case, the current system timestamp will be used.
 * NOTE: IoTConnect DOES NOT support epoch/unix timestamps with 2.0 specification. Reserved for future use.
 * @see IOTCL_IsoTimestampNow, IOTCL_ToIsoTimestamp
 */
bool IOTCL_TelemetryAddWithEpochTime(IOTCL_MESSAGE_HANDLE message, time_t time);

/*
 * Sets a number value in in the last created data set. Creates one with current time if none were created
 * previously with TelemetryAddWith*Time() call.
 * Path is an (optional) dotted notation that can be used to set values in nested objects.
 */
bool IOTCL_TelemetrySetNumber(IOTCL_MESSAGE_HANDLE message, const char *path, double value);

/*
 * Sets a string value in in the last created data set. Creates one with current time if none were created
 * previously with TelemetryAddWith*Time() call.
 * Path is an (optional) dotted notation that can be used to set values in nested objects.
 */
bool IOTCL_TelemetrySetString(IOTCL_MESSAGE_HANDLE message, const char *path, const char *value);

/*
 * Sets a boolean value in in the last created data set. Creates one with current time if none were created
 * previously with TelemetryAddWith*Time() call.
 * Path is an (optional) dotted notation that can be used to set values in nested objects.
 */
bool IOTCL_TelemetrySetBool(IOTCL_MESSAGE_HANDLE message, const char *path, bool value);

/*
 * Sets the field value to null in the last created data set. Creates one with current time if none were created
 * previously with TelemetryAddWith*Time() call.
 * Null values can be used to indicate lack of avaialable data to the cloud.
 * Path is an (optional) dotted notation that can be used to set values in nested objects.
 */
bool IOTCL_TelemetrySetNull(IOTCL_MESSAGE_HANDLE message, const char *path);

const char *IOTCL_CreateSerializedString(IOTCL_MESSAGE_HANDLE message, bool pretty);

void IOTCL_DestroySerialized(const char *serialized_string);

#ifdef __cplusplus
}
#endif

#endif /* IOTCONNECT_TELEMETRY_H */
