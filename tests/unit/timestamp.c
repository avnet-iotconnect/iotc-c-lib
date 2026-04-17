/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

// Defaulting log endlines for appropriate OS for tests only
#ifndef IOTCL_ENDLN
    #ifdef _WIN32
        #define IOTCL_ENDLN "\r\n"
    #else
        #define IOTCL_ENDLN "\n"
    #endif
#endif


#include "iotcl.h"
#include "iotcl_log.h"
#include "iotcl_util.h"
#include "heap_tracker.h"

static int validate_timestamp(const char* iso_timestamp_in) {
    time_t timestamp = iotcl_iso8601_basic_to_epoch_utc_time(iso_timestamp_in);
    if (timestamp == 0) {
        IOTCL_INFO("Failed to parse timestamp: %s", iso_timestamp_in);
        return -1;
    }

    char iso_timestamp_out[IOTCL_ISO_TIMESTAMP_STR_LEN + 1]; // Enough for "YYYY-MM-DDTHH:MM:SSZ"
    int result = iotcl_to_iso_timestamp(timestamp, iso_timestamp_out, sizeof(iso_timestamp_out));
    if (result != IOTCL_SUCCESS) {
        IOTCL_INFO("Failed to format timestamp back to ISO8601: %s. Error was: %d", iso_timestamp_in, result);
        return -2;
    }
    IOTCL_INFO("Original ISO8601: %s, Parsed and formatted back: %s", iso_timestamp_in, iso_timestamp_out);
    
    // Slight difference in /ITOCONNECT formatted timestamp and AWS formatted (input) timestamp
    // so only compare seconds.
    if (strncmp(iso_timestamp_in, iso_timestamp_out, IOTCL_ISO_TIMESTAMP_STR_LEN - (strlen(".000Z"))) != 0) {
        IOTCL_INFO("Mismatch! Original: %s, After parsing and formatting back: %s", iso_timestamp_in, iso_timestamp_out);
        return -3;
    }
    return 0;
}

static void timestamp_test() {

}

int main(void) {
    ht_reset_config();
    ht_init();
    iotcl_configure_dynamic_memory(ht_malloc, ht_free);

    int test_result = 0; // until proven otherwise
    test_result |= validate_timestamp("2020-01-01T00:00:00Z");
    test_result |= validate_timestamp("2020-12-31T23:59:59Z");
    test_result |= validate_timestamp("2023-02-28T23:59:59Z"); // Laast leap second
    test_result |= validate_timestamp("2023-03-01T00:00:00Z"); // Day after leap second
    test_result |= validate_timestamp("1971-01-01T00:00:00Z"); // Earliest valid date
    test_result |= validate_timestamp("2024-02-29T12:34:56Z"); // Leap year
    test_result |= validate_timestamp("2024-03-01T00:00:00Z"); // Day after leap day

    ht_print_summary();
    if (ht_get_num_current_allocations() != 0) {
        return 2;
    }
    if (test_result) {
        IOTCL_INFO("Some timestamp tests failed!");
    } else {
        IOTCL_INFO("All timestamp tests passed!");
    }
    return (test_result ? 1 : 0);
}
