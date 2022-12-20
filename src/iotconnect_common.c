/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al & Neerav Parasher <neerav.parasar@softwebsolutions.com>.
 */
#include <stdlib.h>
#include <string.h>
#include "iotconnect_common.h"

static char timebuf[sizeof "0000-00-00T00:00:00.000Z"];

static const char *to_iso_timestamp(time_t *timestamp) {
    time_t ts = timestamp ? *timestamp : time(NULL);
    strftime(timebuf, (sizeof timebuf), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&ts));
    return timebuf;
}

const char *iotcl_to_iso_timestamp(time_t timestamp) {
    return to_iso_timestamp(&timestamp);
}

const char *iotcl_iso_timestamp_now(void) {
    return to_iso_timestamp(NULL);
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

int get_time_difference(char newT[25], char oldT[25]) {
    time_t NEW, OLD;
    struct tm new_date;
    struct tm old_date;
    unsigned int DYear, DDay, DHour, DMin, DSec;

    new_date.tm_mday = ((newT[8] - '0') * 10 + (newT[9] - '0'));    new_date.tm_mon = ((newT[5] - '0') * 10 + (newT[6] - '0'));
    new_date.tm_year = ((newT[3] - '0') * 1000 + (newT[2] - '0') * 100 + (newT[1] - '0') * 10 + (newT[0] - '0'));
    new_date.tm_hour = ((newT[11] - '0') * 10 + (newT[12] - '0'));  new_date.tm_min = ((newT[14] - '0') * 10 + (newT[15] - '0'));
    new_date.tm_sec = ((newT[17] - '0') * 10 + (newT[18] - '0'));

    old_date.tm_mday = ((oldT[8] - '0') * 10 + (oldT[9] - '0'));    old_date.tm_mon = ((oldT[5] - '0') * 10 + (oldT[6] - '0'));
    old_date.tm_year = ((oldT[3] - '0') * 1000 + (oldT[2] - '0') * 100 + (oldT[1] - '0') * 10 + (oldT[0] - '0'));
    old_date.tm_hour = ((oldT[11] - '0') * 10 + (oldT[12] - '0'));  old_date.tm_min = ((oldT[14] - '0') * 10 + (oldT[15] - '0'));
    old_date.tm_sec = ((oldT[17] - '0') * 10 + (oldT[18] - '0'));

    NEW = mktime(&new_date);
    OLD = mktime(&old_date);

    char* NewT = ctime(&new_date);
    char* lastT = ctime(&old_date);

    while (old_date.tm_sec > new_date.tm_sec) {
        --new_date.tm_min;
        new_date.tm_sec += 60;
    }
    DSec = new_date.tm_sec - old_date.tm_sec;

    while (old_date.tm_min > new_date.tm_min) {
        --new_date.tm_hour;
        new_date.tm_min += 60;
    }
    DMin = new_date.tm_min - old_date.tm_min;

    while (old_date.tm_hour > new_date.tm_hour) {
        --new_date.tm_mday;
        new_date.tm_hour += 24;
    }
    DHour = new_date.tm_hour - old_date.tm_hour;

    DDay = new_date.tm_mday - old_date.tm_mday;
    DYear = new_date.tm_year - old_date.tm_year;

    unsigned int TIMEDIFF = (DYear * 365 * 24 * 60 * 60) + (DDay * 24 * 60 * 60) + (DHour * 60 * 60) + (DMin * 60) + (DSec);
    return TIMEDIFF;
}