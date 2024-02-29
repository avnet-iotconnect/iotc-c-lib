/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCL_TELEMETRY_H
#define IOTCL_TELEMETRY_H

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct IotclMessageHandleTag *IotclMessageHandle;

/*
 * Create a message handle given IoTConnect configuration.
 * This handle needs to be passed to all function in this module.
 * The handle needs to be be destroyed to free up resources, once the message is sent.
 */
IotclMessageHandle iotcl_telemetry_create(void);

/*
 * Destroys the IoTConnect message handle.
 */
void iotcl_telemetry_destroy(IotclMessageHandle message);

/*
 * Call this optional function to add more than one data set to your message, or use custom timestamps.
 * Example usage would be a case where the user would want to record temperature measurement every hour of the day,
 * but wants to send all data sets once per day.
 * If time is not configured, adding more than one data set would be a questionable thing to do
 * or application specific, as each data set would be timestamped with server receipt time, which would be the same
 * for all data sets.
 * See iotcl_iso_timestamp_now() and iotcl_to_iso_timestamp() in iotcl_util.h.
 * The iso_timestamp argument is optional. If set to NULL, it will use the current time (if configured).
 */
int iotcl_telemetry_add_new_data_set(IotclMessageHandle message, const char *iso_timestamp);

/*
 * Sets a value in the current data set.
 * The path argument is the name of the value the user wants to set.
 * The path argument can be passed in dot notation to set nested values of the IoTConnect OBJECT type values.
 * fore example "accelerometer.x" or "accelerometer.y" set x and y respectively in the accelerometer object.
 * Use iotcl_telemetry_set_number to set DOUBLE, INTEGER, LONG and similar IoTConnect data types.
 */
int iotcl_telemetry_set_number(IotclMessageHandle message, const char *path, double value);

// Use this function to set STRING, DATE, TIME, DATETIME and similar IoTConnect types that use JSON string.
int iotcl_telemetry_set_string(IotclMessageHandle message, const char *path, const char *value);

// Use this function to set BOOLEAN IoTConnect type.
int iotcl_telemetry_set_bool(IotclMessageHandle message, const char *path, bool value);

// Setting a value to null may be desired to indicate that the value is not available.
int iotcl_telemetry_set_null(IotclMessageHandle message, const char *path);

// Generates a JSON string on the heap that the user can send to the reporting topic
// The user must call iotcl_telemetry_destroy_serialized_string() when done.
char *iotcl_telemetry_create_serialized_string(IotclMessageHandle message, bool pretty);

// Frees the JSON string created by iotcl_telemetry_create_serialized_string(). Call this once the data is shipped via MQTT.
void iotcl_telemetry_destroy_serialized_string(char *serialized_string);

#ifdef __cplusplus
}
#endif

#endif /* IOTCL_TELEMETRY_H */
