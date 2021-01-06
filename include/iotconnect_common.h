/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_COMMON_H
#define IOTCONNECT_COMMON_H


#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

char *IOTCL_Strdup(const char *str);

// NOTE: This function is not thread-safe
const char *IOTCL_ToIsoTimestamp(time_t timestamp);

// NOTE: This function is not thread-safe
const char *IOTCL_IsoTimestampNow();

// Internal function
void IOTCL_OomError();

#ifdef __cplusplus
}
#endif
#endif //IOTCONNECT_LIB_H
