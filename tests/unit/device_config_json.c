/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdio.h>
#include <string.h>


// Defaulting log endlines for appropriate OS for tests only
#ifndef IOTCL_ENDLN
    #ifdef _WIN32
        #define IOTCL_ENDLN "\r\n"
    #else
        #define IOTCL_ENDLN "\n"
    #endif
#endif

#include "iotcl.h"
#include "iotcl_dra_json_config.h"
#include "heap_tracker.h"

#define TEST_SHORT_FORM_JSON \
    "{\"ver\":\"2.1\",\"pf\":\"aws\",\"cpid\":\"48b14f8b0cb24d029c1573e36ee31e12\",\"env\":\"prod\",\"uid\":\"TestDUID\",\"did\":\"48b14f8b0cb24d029c1573e36ee31e12-TestDUID\",\"at\":3,\"disc\":\"https://discovery.iotconnect.io\"}"

static int json_config_test(void) {
    int errors = 0;

    IotclClientConfig config;
    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_CUSTOM;
    iotcl_init(&config);

    IotclDraJsonConfigResult result = {0};
    int status = iotcl_dra_json_config_parse(&result, TEST_SHORT_FORM_JSON);

    if (status != IOTCL_SUCCESS) {
        printf("ERROR: Failed to parse JSON config, status=%d\n", status);
        errors++;
    } else {
        printf("Parsed JSON config successfully:\n");
        printf("  pf:                 %s\n", result.pf);
        printf("  cpid:               %s\n", result.cpid);
        printf("  env:                %s\n", result.env);
        printf("  duid:               %s\n", result.duid);
        printf("  client_id:          %s\n", result.client_id);
        printf("  dedicated_instance: %s\n", result.dedicated_instance ? "true" : "false");

        // Validate expected values
        if (strcmp(result.pf, "aws") != 0) {
            printf("ERROR: Expected pf='aws', got '%s'\n", result.pf);
            errors++;
        }
        if (strcmp(result.cpid, "48b14f8b0cb24d029c1573e36ee31e12") != 0) {
            printf("ERROR: Expected cpid='48b14f8b0cb24d029c1573e36ee31e12', got '%s'\n", result.cpid);
            errors++;
        }
        if (strcmp(result.env, "prod") != 0) {
            printf("ERROR: Expected env='prod', got '%s'\n", result.env);
            errors++;
        }
        if (strcmp(result.duid, "TestDUID") != 0) {
            printf("ERROR: Expected duid='TestDUID', got '%s'\n", result.duid);
            errors++;
        }
        if (strcmp(result.client_id, "48b14f8b0cb24d029c1573e36ee31e12-TestDUID") != 0) {
            printf("ERROR: Expected client_id='48b14f8b0cb24d029c1573e36ee31e12-TestDUID', got '%s'\n", result.client_id);
            errors++;
        }
        // dedicated_instance should be false in this case
        if (result.dedicated_instance != false) {
            printf("ERROR: Expected dedicated_instance=false, got true\n");
            errors++;
        }

        iotcl_dra_json_config_free(&result);
    }

    iotcl_deinit();
    return errors;
}

int main(void) {
    ht_reset_config();
    ht_init();
    iotcl_configure_dynamic_memory(ht_malloc, ht_free);

    int errors = json_config_test();

    ht_print_summary();

    if (ht_get_num_current_allocations() != 0) {
        printf("ERROR: Memory leak detected!\n");
        return 2;
    }

    if (errors > 0) {
        printf("FAILED: %d error(s)\n", errors);
        return 1;
    }

    printf("All tests passed.\n");
    return 0;
}
