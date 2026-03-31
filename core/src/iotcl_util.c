/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "iotcl_cfg.h"
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
    int tmp_ret = (int) strftime(buffer, buffer_size, IOTCL_ISO_TIMESTAMP_FORMAT, gmtime(&timestamp));
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


// NOTE: Year must be positive (greater than 1970)
static time_t tm_to_time_t_utc(const struct tm *tm) {
    if (tm->tm_year < 71 || tm->tm_mon < 0 || tm->tm_mon > 11 ||
        tm->tm_mday < 1 || tm->tm_mday > 31 || tm->tm_hour < 0 ||
        tm->tm_hour > 23 || tm->tm_min < 0 || tm->tm_min > 59 ||
        tm->tm_sec < 0 || tm->tm_sec > 60) {
        return (time_t)(-1);
    }

    int y = tm->tm_year + 1900;
    int m = tm->tm_mon + 1;
    if (m < 3) {
        m += 12;
        y--;
    }

    // Zeller's congruence-style calendar calculations
    // Calculate days since Unix epoch (1970-01-01)
    time_t days = (time_t) 365 * (y - 1970)
                   + (y - 1969) / 4
                   - (y - 1901) / 100
                   + (y - 1601) / 400
                   + (153 * m - 457) / 5
                   + tm->tm_mday - 1;

    return days * 86400 + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
}

time_t iotcl_iso8601_basic_to_epoch_utc_time(const char *iso_timestamp_str) {
    struct tm time = {0};
    int sscanf_ret = sscanf(iso_timestamp_str, IOTCL_ISO_TIMESTAMP_BASIC_PARSE_FORMAT,
                            &time.tm_year, &time.tm_mon, &time.tm_mday,
                            &time.tm_hour, &time.tm_min, &time.tm_sec);
    if (sscanf_ret != 6) {
        IOTCL_ERROR(
            IOTCL_ERR_PARSING_ERROR,
            "Failed to parse ISO timestamp string \"%s\"! Expected format is \"%s\"",
            iso_timestamp_str,
            IOTCL_ISO_TIMESTAMP_BASIC_PARSE_FORMAT
        );
        return 0;
    }
    time.tm_year -= 1900; // tm_year is years since 1900
    time.tm_mon -= 1;     // tm_mon is 0-based 
    return tm_to_time_t_utc(&time);    
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