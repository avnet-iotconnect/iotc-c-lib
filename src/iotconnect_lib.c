/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "iotconnect_common.h"
#include "iotconnect_lib.h"

static IotclConfig config;
static bool config_is_valid = false;

bool iotcl_init(IotclConfig *c) {
    iotcl_deinit();
    if (
            !c || !c->device.env || !c->device.cpid || !c->device.duid ||
            0 == strlen(c->device.env) || 0 == strlen(c->device.env) || 0 == strlen(c->device.env)
            ) {
        IOTCL_LOG ("IotConnectLib_Configure: configuration parameters missing" IOTCL_NL);
        return false;
    }
    if (strlen(c->device.cpid) + 1 /* dash, separator */  + strlen(c->device.duid) > MAX_DEVICE_COMBINED_NAME) {
        IOTCL_LOG ("IotConnectLib_Configure: combined name (cpid + uuid) exceeded maximum value" IOTCL_NL);
        return false;
    }
    // FIXME this is a shallow copy -- have potential dangling pointers from c here?
    // Only works if c is static, or allocated -- not if its from some local variable?
    memcpy(&config, c, sizeof(config));

    // FIXME this check looks superfluous? already checked c before it was copied -- maybe it's needed for a deep copy
    // if duid/cpid/env are allocated?
    if (!config.device.duid || !config.device.cpid || !config.device.env) {
        // allocation failure
        IOTCL_LOG ("IotConnectLib_Configure: malloc failure" IOTCL_NL);
        iotcl_deinit();
        return false;
    }
    config_is_valid = true;
    return true;
}


IotclConfig *iotcl_get_config() {
    if (!config_is_valid) {
        return NULL;
    }
    return &config;
}

void iotcl_deinit() {
    config_is_valid = false;

    memset(&config, 0, sizeof(config));
}
