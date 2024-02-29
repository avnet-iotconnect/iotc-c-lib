/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_UTIL_H
#define IOTCL_UTIL_H

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// you can use this function to configure time_fn in IotclClientConfig
time_t iotcl_default_time(void);

// Clones a string if string not null. Returns NULL otherwise.
char *iotcl_strdup(const char *str);

// Convert unix/epoch timestamp to ISO 8601 timestamp. Provide a buffer into which to place a timestamp.
// Buffer size should be IOTCL_ISO_TIMESTAMP_STR_LEN, but argument s added for safety checking.
int iotcl_to_iso_timestamp(time_t timestamp, char *buffer, size_t buffer_size);

// Call the configured time_fn and return current timestamp.
// Buffer size should be IOTCL_ISO_TIMESTAMP_STR_LEN, but argument s added for safety checking.
int iotcl_iso_timestamp_now(char *buffer, size_t buffer_size);

// Checks if str is printable up to given length. Prints an error with "what" as message prefix if not printable.
// Length should not include the null string terminator.
bool iotcl_is_printable(const char* what, const char* str, size_t length);


#ifdef __cplusplus
}
#endif
#endif //IOTCL_UTIL_H
