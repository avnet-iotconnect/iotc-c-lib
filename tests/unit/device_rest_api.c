/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdio.h>
#include <string.h>

#include "iotcl.h"
#include "iotcl_dra_url.h"
#include "iotcl_dra_discovery.h"
#include "iotcl_dra_identity.h"
#include "heap_tracker.h"


#define EXAMPLE_DISCOVERY_RESPONSE \
 "{\"d\":{\"ec\":0,\"bu\":\"https://diavnet.iotconnect.io/api/2.1/agent/device-identity/cg/b892c353-e375-4cc3-8841-32e271e26122\",\"log:mqtt\":{\"hn\":\"\",\"un\":\"\",\"pwd\":\"\",\"topic\":\"\"},\"pf\":\"az\"},\"status\":200,\"message\":\"Success\"}"

#define EXAMPLE_IDENTITY_RESPONSE \
    "{\"d\":{\"ec\":0,\"ct\":200,\"meta\":{\"at\":3,\"df\":60,\"cd\":\"XG4E2CA\",\"gtw\":null,\"edge\":0,\"pf\":0,\"hwv\":\"\",\"swv\":\"\",\"v\":2.1},\"has\":{\"d\":0,\"attr\":1,\"set\":0,\"r\":0,\"ota\":0},\"p\":{\"n\":\"mqtt\",\"h\":\"a3etk4e19usyja-ats.iot.us-east-1.amazonaws.com\",\"p\":8883,\"id\":\"abcde\",\"topics\":{\"rpt\":\"$aws/rules/msg_d2c_rpt/abcde/2.1/0\",\"flt\":\"$aws/rules/msg_d2c_flt/abcde/2.1/3\",\"od\":\"$aws/rules/msg_d2c_od/abcde/2.1/4\",\"hb\":\"$aws/rules/msg_d2c_hb/abcde/2.1/5\",\"ack\":\"$aws/rules/msg_d2c_ack/abcde/2.1/6\",\"dl\":\"$aws/rules/msg_d2c_dl/abcde/2.1/7\",\"di\":\"$aws/rules/msg_d2c_di/abcde/2.1/1\",\"c2d\":\"iot/abcde/cmd\",\"set\":{\"pub\":\"$aws/things/abcde/shadow/name/setting_info/report\",\"sub\":\"$aws/things/abcde/shadow/name/setting_info/property-shadow\",\"pubForAll\":\"$aws/things/abcde/shadow/name/setting_info/get\",\"subForAll\":\"$aws/things/abcde/shadow/name/setting_info/get/all\"}}},\"dt\":\"2024-03-06T16:29:56.745Z\"},\"status\":200,\"message\":\"Device info loaded successfully.\"}"

static void discovery_test(void) {

    IotclClientConfig config;

    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_CUSTOM;
    iotcl_init_and_print_config(&config);

    IotclDraUrlContext c = {0};

    iotcl_dra_url_init(&c, "https://diavnet.iotconnect.io/api/2.1/agent/device-identity/cg/b892c358-e375-4cc3-8841-32e271e26122");
    printf("Example URL: %s\n", iotcl_dra_url_get_url(&c));
    printf("Hostname: %s\n", iotcl_dra_url_get_hostname(&c));
    printf("Resource: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_use_suffix_path(&c, "/test/suffix");
    printf("Resource with sufix: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_use_suffix_path(&c, "/longer/suffix");
    printf("Resource with sufix: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_use_suffix_path(&c, "/short/suffix"); // should not reallocate. check with a breakpoint
    printf("Resource with sufix: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_deinit(&c);

    iotcl_dra_discovery_init_url_aws(&c, "mycpid", "myenv");
    printf("Discovery URL: %s\n", iotcl_dra_url_get_url(&c));
    iotcl_dra_url_deinit(&c);

    // allocate exact slack. can check with breakpoint that we did not reallocate:
    iotcl_dra_discovery_parse(&c, strlen(IOTCL_DRA_IDENTITY_PREFIX) + strlen("abcde"), EXAMPLE_DISCOVERY_RESPONSE);
    printf("Base URL: %s\n", iotcl_dra_url_get_url(&c));
    iotcl_dra_identity_build_url(&c, "abcde");
    printf("Identity URL: %s\n", iotcl_dra_url_get_url(&c));
    iotcl_dra_identity_configure_library_mqtt(EXAMPLE_IDENTITY_RESPONSE);
    iotcl_mqtt_print_config();
    iotcl_dra_url_deinit(&c);
    iotcl_deinit();

    printf(" --  TESTING ERROR AND EDGE CASES -- \n");

    iotcl_dra_url_init(&c, "no https");
    printf("Resource: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_deinit(&c);

    iotcl_dra_url_init(&c, "https://host");
    printf("Resource: %s\n", iotcl_dra_url_get_resource(&c));
    iotcl_dra_url_deinit(&c);

    iotcl_dra_url_init(&c, "http://host/");
    printf("Resource: %s\n", iotcl_dra_url_get_resource(&c));
    printf("Is HTTPS: %s\n", iotcl_dra_url_is_https(&c) ? "yes" : "no");
    iotcl_dra_url_deinit(&c);


    /// END
    iotcl_deinit();
}

int main(void) {
    ht_reset_config();
    ht_init();
    iotcl_configure_dynamic_memory(ht_malloc, ht_free);

    discovery_test();

    ht_print_summary();
}
