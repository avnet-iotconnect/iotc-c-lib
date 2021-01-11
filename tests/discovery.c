#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotconnect_discovery.h"

#define EXAMPLE_DISCOVERY_RESPONSE \
 "{\"baseUrl\":\"https://avnetagent.iotconnect.io/api/2.0/agent/\",\"logInfo\":\"<logInfo><hostName /><user /><password /><topic /></logInfo>\"}"

#define EXAMPLE_GOOD_SYNC_RESPONSE \
 "{\"d\":{\"sc\":{\"hb\":null,\"log\":null,\"sf\":5,\"df\":30},\"p\":{\"n\":\"mqtt\",\"h\":\"poc-iotconnect-iothub-eu.azure-devices.net\",\"p\":8883,\"id\":\"your-device-duid\",\"un\":\"poc-iotconnect-iothub-eu.azure-devices.net/avtds-nrf-352656100880056/?api-version=2018-06-30\",\"pwd\":\"\",\"pub\":\"devices/avtds-nrf-352656100880056/messages/events/\",\"sub\":\"devices/avtds-nrf-352656100880056/messages/devicebound/#\"},\"d\":null,\"att\":null,\"set\":null,\"r\":null,\"ota\":{\"force\":false,\"guid\":\"87E9A4C5-35D2-4D49-9396-8E0E7D13E4E0\",\"urls\":[\"https://pociotconnectblobstorage.blob.core.windows.net/firmware/E853AD82-E207-4F29-A72A-7C6A15EF4136.bin?sv=2018-03-28&sr=b&sig=LOydmMfXxRj%2BF8BGJPxKmD4pnveeV4OAIUxUxmlDkIE%3D&se=2021-01-11T19%3A56%3A12Z&sp=r\"],\"ver\":{\"sw\":\"01.00.00\",\"hw\":\"1.0\"}},\"dtg\":\"891ed197-9621-44ea-819c-e7b2d40afafe\",\"cpId\":\"you-cpid\",\"rc\":0,\"ee\":0,\"at\":2,\"ds\":0}}"

#define DEVICE_NOT_FOUND_SYNC_RESPONSE \
 "{\"d\":{\"sc\":null,\"p\":null,\"d\":null,\"att\":null,\"set\":null,\"r\":null,\"ota\":null,\"dtg\":\"00000000-0000-0000-0000-000000000000\",\"cpId\":null,\"rc\":3,\"ee\":0,\"at\":0,\"ds\":3}}"

static void report_sync_error(IOTCL_SyncResponse *response, const char* sync_response_str) {
    if (NULL == response) {
        printf("IOTC_SyncResponse is NULL. Out of memory?\n");
        return;
    }
    switch(response->ds) {
        case IOTCL_SR_DEVICE_NOT_REGISTERED:
            printf("IOTC_SyncResponse error: Not registered\n");
            break;
        case IOTCL_SR_AUTO_REGISTER:
            printf("IOTC_SyncResponse error: Auto Register\n");
            break;
        case IOTCL_SR_DEVICE_NOT_FOUND:
            printf("IOTC_SyncResponse error: Device not found\n");
            break;
        case IOTCL_SR_DEVICE_INACTIVE:
            printf("IOTC_SyncResponse error: Device inactive\n");
            break;
        case IOTCL_SR_DEVICE_MOVED:
            printf("IOTC_SyncResponse error: Device moved\n");
            break;
        case IOTCL_SR_CPID_NOT_FOUND:
            printf("IOTC_SyncResponse error: CPID not found\n");
            break;
        case IOTCL_SR_UNKNOWN_DEVICE_STATUS:
            printf("IOTC_SyncResponse error: Unknown device status error from server\n");
            break;
        case IOTCL_SR_ALLOCATION_ERROR:
            printf("IOTC_SyncResponse internal error: Allocation Error\n");
            break;
        case IOTCL_SR_PARSING_ERROR:
            printf("IOTC_SyncResponse internal error: Parsing error. Please check parameters passed to the request.\n");
            break;
        default:
            printf("WARN: report_sync_error called, but no error returned?\n");
            break;
    }
    printf("Raw server response was:\n--------------\n%s\n--------------\n", sync_response_str);
}

static void test() {
    {
        IOTCL_DiscoveryResponse *dr;
        dr = IOTC_DiscoveryParseDiscoveryResponse(EXAMPLE_DISCOVERY_RESPONSE);
        printf("Discovery Response:\n");
        printf("url: %s\n", dr->url);
        printf("host: %s\n", dr->host);
        printf("path: %s\n", dr->path);
        IOTCL_DiscoveryFreeDiscoveryResponse(dr);
    }

    {
        IOTCL_SyncResponse *gsr; // good sync response example
        gsr = IOTCL_DiscoveryParseSyncResponse(EXAMPLE_GOOD_SYNC_RESPONSE);
        if (!gsr || gsr->ds != IOTCL_SR_OK) {
            report_sync_error(gsr, EXAMPLE_GOOD_SYNC_RESPONSE);
            exit(-1); // test failed
        }
        printf("dtg: %s\n", gsr->dtg);
        printf("cpid: %s\n", gsr->cpid);
        printf("MQTT host: %s\n", gsr->broker.host);
        printf("MQTT client ID: %s\n", gsr->broker.client_id);
        printf("MQTT user name: %s\n", gsr->broker.name);
        printf("MQTT password: %s\n", gsr->broker.pass);
        printf("MQTT pub topic: %s\n", gsr->broker.pub_topic);
        printf("MQTT sub topic: %s\n", gsr->broker.sub_topic);

        printf("Good response test success\n");
        IOTCL_DiscoveryFreeSyncResponse(gsr);
    }

    {
        IOTCL_SyncResponse *bsr; // bad sync response example
        bsr = IOTCL_DiscoveryParseSyncResponse(DEVICE_NOT_FOUND_SYNC_RESPONSE);
        if (!bsr || bsr->ds != IOTCL_SR_DEVICE_NOT_FOUND) {
            printf("False positive or wrong error reported. Test failed!\n");
        } else {
            report_sync_error(bsr, DEVICE_NOT_FOUND_SYNC_RESPONSE);
            printf("Bad response test success\n");
        }
       IOTCL_DiscoveryFreeSyncResponse(bsr);
    }
}




int main() {
    test();
}
