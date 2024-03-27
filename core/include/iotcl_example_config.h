/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_EXAMPLE_CONFIG_H
#define IOTCL_EXAMPLE_CONFIG_H

/* The specification states different values which differ from the actual values and behavior
 * accepted by the back end. If/when the back end changes to comply with the documentation,
 * define IOTCL_C2D_ACK_USES_SPEC in your iotcl_config.h to use values defined by the documentation.
 */
// #define IOTCL_C2D_ACK_USES_SPEC

/* Enable this if your discovery response is erroneously reporting that your subscription has expired.
 * Related service ticket https://awspoc.iotconnect.io/support-info/2024031415124727
 */
// #define IOTCL_DRA_DISCOVERY_IGNORE_SUBSCRIPTION_EXPIRED

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
