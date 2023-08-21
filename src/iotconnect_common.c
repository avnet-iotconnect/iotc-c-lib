/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#include <stdlib.h>
#include <string.h>
#include "iotconnect_common.h"

static char timebuf[sizeof "2011-10-08T07:07:01.000Z"];

// Obviously, this function is not thread-safe, due to the fixed buffer
const char *iotcl_iso_timestamp_now(void) {
    time_t ts = time(NULL);
    strftime(timebuf, (sizeof timebuf), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&ts));
    return timebuf;
}

unsigned long get_expiry_from_now(time_t expiry_secs)
{
    const time_t expiration = time(NULL) + expiry_secs;
    return (unsigned long) expiration;
}

char *iotcl_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    size_t size = strlen(str) + 1;
    char *p = (char *) malloc(size);
    if (p != NULL) {
        memcpy(p, str, size);
    }
    return p;
}
