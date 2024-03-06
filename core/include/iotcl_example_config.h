/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_EXAMPLE_CONFIG_H
#define IOTCL_EXAMPLE_CONFIG_H

// If using pure c99 standard or if your platform doesn't have reentrant strtok (strtok_r)
// you can route to strtok with a custom function, but ensure that it's called in a thread-safe manner,
// or add your own custom implementation
// #define IOTCL_STRTOK_R strtok_r

// See iotcl.h for more information about silently ignoring messages arriving on unknown topics

/* Enable this for the time being if you need command ack functionality enabled until the fix is available on the back end
#define IOTCL_C2D_LEGACY_ACK_SUPPORT
*/

// See iotc_log.h for more information about configuring logging

/*
#define IOTCL_ENDLN "\r\n"

#define IOTCL_FATAL(err_code, ...) \
    do { \
        printf("IOTCL FATAL (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
#endif

#define IOTCL_ERROR(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL ERROR (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)

#define IOTCL_WARN(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL WARN (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
*/

#endif // IOTCL_EXAMPLE_CONFIG_H
