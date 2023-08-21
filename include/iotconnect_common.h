/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_COMMON_H
#define IOTCONNECT_COMMON_H


#include <time.h>

// No mutexes
// Don't implement if a particular level isn't required.
// Could specify as functions, if required.
#include <stdio.h>
#define IOTC_LOG(...) do{ printf("LOG: ");printf(__VA_ARGS__);}while(0)
#define IOTC_DEBUG(...) do{ printf("DEBUG: ");printf(__VA_ARGS__);}while(0)
#define IOTC_WARN(...) do{ printf("WARN: ");printf(__VA_ARGS__);}while(0)
#define IOTC_ERROR(...) do{ printf("ERROR: ");printf(__VA_ARGS__);}while(0)

#ifdef __cplusplus
extern "C" {
#endif

char *iotcl_strdup(const char *str);

// NOTE: This function is not thread-safe
const char *iotcl_iso_timestamp_now(void);

unsigned long get_expiry_from_now(time_t expiry_secs);

#ifdef __cplusplus
}
#endif
#endif //IOTCONNECT_LIB_H
