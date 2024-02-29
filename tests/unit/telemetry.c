/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdlib.h>
#include <stdio.h>

#include "iotcl.h"
#include "iotcl_util.h"
#include "iotcl_telemetry.h"
#include "heap_tracker.h"

static void my_transport_send(const char *topic, size_t topic_len, const char *json_str) {
    (void) topic_len;
    printf("Sending on topic %s:\n%s\n", topic, json_str);
}

static void telemetry_test(bool use_time) {
    IotclClientConfig config;

    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_AWS_DEDICATED;
    config.device.duid = "mydevice";
    config.mqtt_send_cb = my_transport_send;
    if (use_time) {
        config.time_fn = iotcl_default_time;
    }
    iotcl_init_and_print_config(&config);
    iotcl_mqtt_print_config();
    iotcl_init(&config);

    IotclMessageHandle msg = iotcl_telemetry_create();

    //iotcl_telemetry_add_new_data_set(msg, "2024-01-02T03:04.000Z");
    iotcl_telemetry_set_number(msg, "mytemp", 123);
    iotcl_telemetry_set_string(msg, "str-prs", "prs");

    // Create a new entry with different time and values
    iotcl_telemetry_add_new_data_set(msg, NULL);
    iotcl_telemetry_set_number(msg, "parent.num-111", 111);
    iotcl_telemetry_set_string(msg, "str-456", "456");
    iotcl_telemetry_set_number(msg, "num-123,55", 123.55);

    iotcl_telemetry_add_new_data_set(msg, "2024-01-02T03:04.000Z");
    iotcl_telemetry_set_null(msg, "nulltest");
    iotcl_telemetry_set_bool(msg, "booltest", true);

    iotcl_mqtt_send_telemetry(msg, true);
    iotcl_telemetry_destroy(msg);

    iotcl_deinit();
}

int main(void) {
    ht_reset_config();
    ht_init();
    iotcl_configure_dynamic_memory(ht_malloc, ht_free);

    telemetry_test(true);
    telemetry_test(false);

    ht_print_summary();
}
