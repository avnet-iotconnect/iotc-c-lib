
/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#ifndef IOTCONNECT_LIB_CONFIG_H
#define IOTCONNECT_LIB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_IOTCONNECT_ENV_MAX_LEN 20

#define CONFIG_IOTCONNECT_CLIENTID_MAX_LEN 128

#define CONFIG_IOTCONNECT_DUID_MAX_LEN 64

// IoTHub max device id is 128, which is "<CPID>-<DUID>" (with a dash)
#define CONFIG_IOTCONNECT_CPID_MAX_LEN (CONFIG_IOTCONNECT_CLIENTID_MAX_LEN - 1 - CONFIG_IOTCONNECT_DUID_MAX_LEN)

#ifndef CONFIG_IOTCONNECT_SDK_NAME
#define CONFIG_IOTCONNECT_SDK_NAME "M_C"
#endif

#ifndef CONFIG_IOTCONNECT_SDK_VERSION
#define CONFIG_IOTCONNECT_SDK_VERSION "2.0"
#endif

#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_LIB_CONFIG_H
