/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef IOTCL_LOG_H
#define IOTCL_LOG_H

#include <stdio.h>

// MBEDTLS config file style - include your own to override the config. See iotcl_example_config.h
#if defined(IOTCL_USER_CONFIG_FILE)
#include IOTCL_USER_CONFIG_FILE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* CRLF seems to be used more than just LF on majority of the boards that we have worked with
   Override this in your config file (to blank) if you are using your own logging facility or need just an LF printed.
 * We are using a macro here instead of a runtime function hook to ensure that the compiler can optimize out strings
 * The err_code passed to the macro also allows for an assert-type hook that can be used to reset the device
 * depending on the error code. The default implementation only prints it.
 * Fatal errors (especially OOM errors) should generally generally trigger the device to reset,
 * hopefully with some delay to allow the underlying error to be printed.
 * The provided error code can be used to determine if the device should actually reset or attempt to resume operation.
 * Warnings are generally issued when it is possible that the problem can still resume cloud messaging. Most commonly,
 * warnings are issued during c2d message processing, for example when parsing errors are encountered in c2d messages.
 */
#ifndef IOTCL_ENDLN
#define IOTCL_ENDLN "\r\n"
#endif

#ifndef IOTCL_ERROR
#define IOTCL_ERROR(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL ERROR [%d]: ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
#endif

#ifndef IOTCL_WARN
#define IOTCL_WARN(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL WARN [%d]: ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
#endif

#ifndef IOTCL_INFO
#define IOTCL_INFO(...) \
    do { \
        printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
#endif


#ifdef __cplusplus
}
#endif

#endif // IOTCL_LOG_H