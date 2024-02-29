/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "cJSON.h"
#include "iotcl_constants.h"
#include "iotcl_log.h"
#include "iotcl_internal.h"
#include "iotcl_util.h"

time_t iotcl_default_time(void) {
    return time(NULL);
}

char *iotcl_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    size_t size = strlen(str) + 1;
    char *p = (char *) iotcl_malloc(size);
    if (p) {
        memcpy(p, str, size);
    }
    return p;
}

int iotcl_to_iso_timestamp(time_t timestamp, char *buffer, size_t buffer_size) {
    if (!buffer) {
        IOTCL_WARN(IOTCL_ERR_BAD_VALUE, "iotcl_to_iso_timestamp: Buffer is NULL! Timestamp not created.");
        return IOTCL_ERR_BAD_VALUE;
    }
    if (buffer_size > 1) {
        buffer[0] = 0; // Clear the buffer so it's clean in case of an error
    }
    if (buffer_size < (IOTCL_ISO_TIMESTAMP_STR_LEN + 1)) {
        IOTCL_WARN(IOTCL_ERR_OVERFLOW, "iotcl_to_iso_timestamp: Buffer too small! Timestamp not created.");
        return IOTCL_ERR_OVERFLOW;
    }
    int tmp_ret = strftime(buffer, buffer_size, IOTCL_ISO_TIMESTAMP_FORMAT, gmtime(&timestamp));
    if (IOTCL_ISO_TIMESTAMP_STR_LEN != tmp_ret) {
        IOTCL_WARN(
                IOTCL_ERR_CONFIG_ERROR,
                "iotcl_to_iso_timestamp: Expected strftime to return %d, but got %d!",
                (int) IOTCL_ISO_TIMESTAMP_STR_LEN,
                tmp_ret
        );
        return IOTCL_ERR_CONFIG_ERROR;
    }
    // we could check timestamp < IOTCL_TIME_2024_START, but technically it could be incorrect
    // in case some platform decices to do something weird with time_t.
    // So we will just ensure that they year starts with 2024 or higher by using strncmp alphanumeric ordering.
    if (strncmp(buffer, "2024", 4) == -1) {
        IOTCL_WARN(
                IOTCL_ERR_CONFIG_ERROR,
                "iotcl_to_iso_timestamp: Expected timestamp newer than January 2024, but got %s!",
                buffer
        );
        return IOTCL_ERR_CONFIG_ERROR;
    }
    return IOTCL_SUCCESS;
}

int iotcl_iso_timestamp_now(char *buffer, size_t buffer_size) {
    if (buffer && buffer_size > 1) {
        // Clear the buffer so it's clean in case of an error
        // More error handling is delegated to iotcl_to_iso_timestamp
        buffer[0] = 0;
    }
    if (!iotcl_get_global_config()->is_valid) {
        return IOTCL_ERR_CONFIG_MISSING; // called function will print the error
    }
    if (iotcl_get_global_config()->time_fn) {
        return iotcl_to_iso_timestamp(iotcl_get_global_config()->time_fn(), buffer, buffer_size);
    } else {
        IOTCL_ERROR(IOTCL_ERR_CONFIG_ERROR, "iotcl_iso_timestamp_now called, but time function is not configured");
        return IOTCL_ERR_CONFIG_ERROR;
    }
}

bool iotcl_is_printable(const char *what, const char *str, size_t length) {
    // if we disabled this, then ignore checking
    if (iotcl_get_global_config()->is_valid && iotcl_get_global_config()->disable_printable_check) {
        return true;
    }

    for (int i = 0; i < (int) length; i++) {
        char ch = str[i];
        if (!isprint(ch) && !isspace(ch) && ch != '\r' && ch != '\n') {
            IOTCL_ERROR(
                    IOTCL_ERR_PARSING_ERROR,
                    "%s: Encountered a non-printable character 0x%x at position %d",
                    what, ch, i
            );
            return false;
        }
    }
    return true;
}