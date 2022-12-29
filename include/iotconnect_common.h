/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al & Neerav Parasher <neerav.parasar@softwebsolutions.com>.
 */

#ifndef IOTCONNECT_COMMON_H
#define IOTCONNECT_COMMON_H


#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

char *iotcl_strdup(const char *str);

// NOTE: This function is not thread-safe
const char *iotcl_to_iso_timestamp(time_t timestamp);

// NOTE: This function is not thread-safe
const char *iotcl_iso_timestamp_now(void);

// Internal function
void iotcl_oom_error(void);

//The internal function converts the Timestamp difference into a number of seconds.
//The number of seconds will be used to compare with data frequency(df) to send data at precise intervals.
//Return int value of two - time difference.
int iotcl_get_time_difference(char newT[25], char oldT[25]);

#ifdef __cplusplus
}
#endif
#endif //IOTCONNECT_LIB_H
