/* SPDX-License-Identifier: MIT
 * Copyright (C) 2020 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotcl.h"
#include "iotcl_c2d.h"
#include "heap_tracker.h"

static const char *const TEST_STR_COMMAND = "{\"v\":\"2.1\",\"ct\":0,\"cmd\":\"set-led-green off\",\"ack\":\"4d99ed07-0ea0-43c6-97ba-53780faddc5c\"}";
static const char *const TEST_STR_OTA = "{\"v\":\"2.1\",\"ct\":1,\"cmd\":\"ota\",\"ack\":\"c6c90df6-d27f-44e5-8eb0-e278dc73ad4f\",\"sw\":\"1.5\",\"hw\":\"1\",\"urls\":[{\"url\":\"https://iotc-260030673750.s3.amazonaws.com/584af730-2854-4a77-8f3b-ca1696401e08/firmware/415443e4-8bad-4375-a124-9734d6cc7fdc/64b18fa3-3aa2-43bc-b65b-71371aca72cb.bin?AWSAccessKeyId=ASIATZCYJGNLASDPEL4H&Expires=1706815818&x-amz-security-token=FwoGZXIvYXdzEJT%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaDP0cBexs1rTHqcBxZyKuASD9RoOOeHUIC0pN4AsL%2FA1aXOkEJwSvdI317PwQuKZF%2FRqnBUM3Fxqie7qVtaWIOaWrYXXW9BlJrlZyxJbEsrZ0TWtYB%2FJUqPT300Ioe7Z3E8bswpVFe%2FVw4HscmKDNcHiF54e1ldDRQisiuNiCA4SgCXHMMXvj4%2FFZ1CvlDj2IK3I1m%2FsF9BIoq12q4%2FvgHhb4IG5jG6GtSALb0r5OpcaC0Epy3lXvEj6P5%2BoxqSj91O%2BtBjIti7EkqDYqK%2BI4QAJbPDb2YbpDbXR35ZkihiRXQ6uAWpCuX0Af%2Buw4%2BGIOcd8c&Signature=X1ecCh7I9zojlj%2BCjtwAHhzl9Rg%3D\",\"fileName\":\"my_firmware.bin\"}]}";

static void my_transport_send(const char *topic, const char *json_str) {
    printf("Sending on topic %s:\n%s\n", topic, json_str);
}

static void on_cmd(IotclC2dEventData data) {
    const char *ack_id = iotcl_c2d_get_ack_id(
            data); // we are sending command with "Require Acknowledgement" option in IoTConnect
    const char *command = iotcl_c2d_get_command(data);
    if (!ack_id || !command) {
        return; // by default, the library will print the error in the log if there was an OOM
    }
    printf("Ack ID:  %s\n", ack_id);
    printf("Command: %s\n", command);

    iotcl_mqtt_send_cmd_ack(ack_id, IOTCL_C2D_EVT_CMD_SUCCESS_WITH_ACK, NULL);

    // generate a failure too
    iotcl_mqtt_send_cmd_ack(ack_id, IOTCL_C2D_EVT_CMD_FAILED, "Test generated failure");
}

static void on_ota(IotclC2dEventData data) {
    const char *ack_id = iotcl_c2d_get_ack_id(data);
    const char *url = iotcl_c2d_get_ota_url(data, 0);
    const char *hostname = iotcl_c2d_get_ota_url_hostname(data, 0);
    const char *resource = iotcl_c2d_get_ota_url_resource(data, 0);
    const char *filename = iotcl_c2d_get_ota_original_filename(data, 0);
    const char *sw_ver = iotcl_c2d_get_ota_sw_version(data);
    const char *hw_ver = iotcl_c2d_get_ota_hw_version(data);
    if (!url || !hostname || !resource || !sw_ver || !hw_ver) {
        return; // by default, the library will print the error in the log if there was an OOM or a any kind of error
    }
    printf("\n\nNumber of OTA URLs: %d\n", iotcl_c2d_get_ota_url_count(data));
    printf("Ack ID:     %s\n", ack_id);
    printf("URL:        %s\n", url);
    printf("hostname:   %s\n", hostname);
    printf("resource:   %s\n", resource);
    printf("filename:   %s\n", filename);
    printf("SW version: %s\n", sw_ver);
    printf("HW version: %s\n", hw_ver);

    iotcl_mqtt_send_ota_ack(ack_id, IOTCL_C2D_EVT_OTA_SUCCESS, NULL);

    // try generating a failure
    iotcl_mqtt_send_ota_ack(ack_id, IOTCL_C2D_EVT_OTA_DOWNLOAD_FAILED, NULL);
}

static void c2d_test(void) {
    IotclClientConfig config;

    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_AWS_SHARED;
    config.device.duid = "mydevice";
    config.device.cpid = "MYCPID";
    config.mqtt_send_cb = my_transport_send;
    config.events.cmd_cb = on_cmd;
    config.events.ota_cb = on_ota;
    iotcl_init_and_print_config(&config);

    IotclMqttConfig *mc = iotcl_mqtt_get_config();
    if (!mc) {
        // It should never be null because we called iotcl_init() just now.
        // Called function will print the error.
        return;
    }
    iotcl_mqtt_print_config();

    // This would be the expected C2D topic that we supposedly already obtained from mqtt config after configuring,
    // and we supposedly received the topic from subscription callback on a real MQTT client.
    // BWe are hardcoding it for the purpose of testing the topic generation.
    const char *const C2D_TOPIC = "iot/MYCPID-mydevice/cmd";
    iotcl_mqtt_receive(C2D_TOPIC, TEST_STR_COMMAND);

    iotcl_mqtt_receive_with_length(C2D_TOPIC, (uint8_t *) TEST_STR_OTA, strlen(TEST_STR_OTA)); // test _with_length as well...
    iotcl_deinit();
}


int main(void) {
    ht_reset_config();
    ht_init();
    iotcl_configure_dynamic_memory(ht_malloc, ht_free);

    c2d_test();

    ht_print_summary();
}

