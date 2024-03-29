/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdio.h>

#include "iotcl.h"
#include "iotcl_util.h"
#include "iotcl_telemetry.h"
#include "heap_tracker.h"

static void my_transport_send(const char *topic, const char *json_str) {
    printf("Sending on topic %s:\n%s\n", topic, json_str);
}

static void telemetry_test(bool use_time) {
    int err_cnt = 0;
    IotclClientConfig config;

    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_AWS_DEDICATED;
    config.device.duid = "mydevice";
    config.mqtt_send_cb = my_transport_send;
    if (use_time) {
        config.time_fn = iotcl_default_time;
    }
    err_cnt += iotcl_init_and_print_config(&config) ? 1 : 0;
    iotcl_mqtt_print_config();

    IotclMessageHandle msg = iotcl_telemetry_create();

    err_cnt += iotcl_telemetry_set_number(msg, "mytemp", 123) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_string(msg, "str_abc", "abc") ? 1 : 0;

    // Create a new entry with different time and values
    err_cnt += iotcl_telemetry_add_new_data_set(msg, "2024-01-02T03:04.000Z") ? 1 : 0;
    err_cnt += iotcl_telemetry_set_number(msg, "parent.num_111", 111) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_string(msg, "str_456", "456") ? 1 : 0;
    err_cnt += iotcl_telemetry_set_number(msg, "num-123_55", 123.55) ? 1 : 0;

    err_cnt += iotcl_telemetry_add_new_data_set(msg, "2024-01-02T03:05.000Z") ? 1 : 0;
    err_cnt += iotcl_telemetry_set_null(msg, "nulltest") ? 1 : 0;
    err_cnt += iotcl_telemetry_set_bool(msg, "booltest", true) ? 1 : 0;


    const int EXPECTED_CNT = 5;
    printf("START INVALID VALUE TESTING. Expecting %d errors:\n", EXPECTED_CNT);
    printf("---------------------------\n");
    err_cnt += iotcl_telemetry_add_new_data_set(msg, NULL) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_number(msg, "too.many.dots", 1) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_number(msg, ".starts_with_dot", 1) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_bool(msg, "ends_with_dot.", false) ? 1 : 0;
    err_cnt += iotcl_telemetry_set_bool(msg, NULL, true) ? 1 : 0;
    printf("---------------------------\n");

    err_cnt += iotcl_mqtt_send_telemetry(msg, true) ? 1 : 0;
    iotcl_telemetry_destroy(msg);

    if (EXPECTED_CNT != err_cnt) {
        printf("Total error count of %d is INCORRECT!\n", err_cnt);
    } else {
        printf("Total error count is correct.\n");
    }
    printf("END INVALID VALUE TESTING.\n");

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
