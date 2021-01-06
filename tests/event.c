/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotconnect_lib.h"

// test for older version of iotconnect protocol
static const char *TEST_STR_V1 =
        "{\"cmdType\":\"0x02\",\"data\":{\"cpid\":\"MyCpid\",\"guid\":\"13bf86a9-f907-4ce9-b8cb-750a34614fcd\",\"uniqueId\":\"MyDeviceId\",\"command\":\"ota https://pociotconnectblobstorage.blob.core.windows.net/firmware/681AEAC1-90F2-4D94-BD24-5D1E76612D9D.txt?sv=2018-03-28&sr=b&sig=sGF8qpwqt0eo4Ve66XIl8WksGSzkObQP%2BbMb4Rz1coc%3D&se=2020-05-27T17%3A32%3A34Z&sp=r\",\"ack\":false,\"ackId\":\"8a4510e3-1f87-452a-9763-05647fdc3f0e\",\"cmdType\":\"0x02\",\"ver\":{\"sw\":\"0.1\",\"hw\":\"1.0\"},\"urls\":[\"https://pociotconnectblobstorage.blob.core.windows.net/firmware/681AEAC1-90F2-4D94-BD24-5D1E76612D9D.txt?sv=2018-03-28&sr=b&sig=sGF8qpwqt0eo4Ve66XIl8WksGSzkObQP%2BbMb4Rz1coc%3D&se=2020-05-27T17%3A32%3A34Z&sp=r\"]}}";
// test for newer version of iotconnect protocol
static const char *TEST_STR_V2 =
        "{\"cmdType\":\"0x02\",\"data\":{\"cpid\":\"MyCpid\",\"guid\":\"6ac6e586-68f1-40be-9d52-c11e75367f60\",\"uniqueId\":\"MyDeviceId\",\"command\":\"ota\",\"ack\":false,\"ackId\":\"7bd13714-dc07-44ea-99f2-c606f8cf2d2a\",\"cmdType\":\"0x02\",\"ver\":{\"sw\":\"0.2\",\"hw\":\"0.1\"},\"urls\":[{\"url\":\"https://forlifeblobstorage.blob.core.windows.net/firmware/4B82F95B-5B0F-408F-BDAF-A7A7C2F027AF.sfb?sv=2018-03-28&sr=b&sig=gQqmSF8Fs9tYolrzE4n%2Fj9NBdSq7%2BFCN89wZYVnFN6U%3D&se=2020-06-30T20%3A08%3A53Z&sp=r\"}]}}";

void on_event(IOTCL_EVENT_DATA data, IotConnectEventType type) {
    (void) data;
    printf("Got event type %d\n", type);
}

void on_cmd(IOTCL_EVENT_DATA data) {
    const char *command = IOTCL_CloneCommand(data);
    if (NULL != command) {
        printf("Command is: %s\n", command);
        free((void *) command);
    }
    const char *ack = IOTCL_CreateAckStringAndDestroyEvent(data, true, NULL);
    if (NULL != ack) {
        printf("Sent CMD ack: %s\n", ack);
        free((void *) ack);
    } else {
        printf("Error while creating the ack JSON");
    }

}

void on_ota(IOTCL_EVENT_DATA data) {
    const char *url = IOTCL_CloneDownloadUrl(data, 0);
    if (NULL != url) {
        printf("Download URL is: %s\n", url);
        free((void *) url);
    }
    const char *command = IOTCL_CloneCommand(data);
    if (NULL != command) {
        printf("Command is: %s\n", command);
        free((void *) command);
    }
    const char *sw_ver = IOTCL_CloneSwVersion(data);
    if (NULL != sw_ver) {
        printf("SW Version is: %s\n", sw_ver);
        free((void *) sw_ver);
    }
    const char *hw_ver = IOTCL_CloneHwVersion(data);
    if (NULL != hw_ver) {
        printf("HW Version is: %s\n", hw_ver);
        free((void *) hw_ver);
    }
    const char *ack = IOTCL_CreateAckStringAndDestroyEvent(data, true, NULL);
    if (NULL != ack) {
        printf("Sent OTA ack: %s\n", ack);
        free((void *) ack);
    }
}


/* Test output:
Got event type 2
Download URL is: https://pociotconnectblobstorage.blob.core.windows.net/firmware/681AEAC1-90F2-4D94-BD24-5D1E76612D9D.txt?sv=2018-03-28&sr=b&sig=sGF8qpwqt0eo4Ve66XIl8WksGSzkObQP%2BbMb4Rz1coc%3D&se=2020-05-27T17%3A32%3A34Z&sp=r
Command is: ota https://pociotconnectblobstorage.blob.core.windows.net/firmware/681AEAC1-90F2-4D94-BD24-5D1E76612D9D.txt?sv=2018-03-28&sr=b&sig=sGF8qpwqt0eo4Ve66XIl8WksGSzkObQP%2BbMb4Rz1coc%3D&se=2020-05-27T17%3A32%3A34Z&sp=r
SW Version is: 0.1
Sent OTA ack: {"mt":11,"t":"2020-07-29T18:15:58.000Z","uniqueId":"my-device-id","cpId":"MyCpid","sdk":{"l":"M_C","v":"2.0","e":"avnetpoc"},"d":{"ackId":"8a4510e3-1f87-452a-9763-05647fdc3f0e","msg":"","st":7}}

Got event type 2
Download URL is: https://forlifeblobstorage.blob.core.windows.net/firmware/4B82F95B-5B0F-408F-BDAF-A7A7C2F027AF.sfb?sv=2018-03-28&sr=b&sig=gQqmSF8Fs9tYolrzE4n%2Fj9NBdSq7%2BFCN89wZYVnFN6U%3D&se=2020-06-30T20%3A08%3A53Z&sp=r
Command is: ota
SW Version is: 0.2
Sent OTA ack: {"mt":11,"t":"2020-07-29T18:15:58.000Z","uniqueId":"my-device-id","cpId":"MyCpid","sdk":{"l":"M_C","v":"2.0","e":"avnetpoc"},"d":{"ackId":"7bd13714-dc07-44ea-99f2-c606f8cf2d2a","msg":"","st":7}}
*/
void test() {
    IOTCL_CONFIG config;
    memset(&config, 0, sizeof(config));

    config.device.env = "prod";
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";

    config.event_functions.ota_cb = on_ota;
    config.event_functions.cmd_cb = on_cmd;
    config.event_functions.msg_cb = on_event;

    IOTCL_Init(&config);

    if (!IOTCL_ProcessEvent(TEST_STR_V1)) {
        printf("Error encountered while processing %s\n", TEST_STR_V1);
    }
    if (!IOTCL_ProcessEvent(TEST_STR_V2)) {
        printf("Error encountered while processing %s\n", TEST_STR_V2);
    }
}


int main() {
    test();
}
