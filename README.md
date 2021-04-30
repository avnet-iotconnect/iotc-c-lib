# iotc-c-lib

This is a C library intended generally for embedded systems which abstracts IoTConnect MQTT protocol messages
and the discovery HTTP protocol.

This library not an SDK. As each embedded platform/os generally has its own MQTT and HTTP 
implementation, the library does not have direct dependencies on any specific one. 

The library only abstracts JSON responses from the cloud and provides mechanisms for composing JSON
messages that need to be sent to the cloud. The general use case would require os/platform specific HTTP 
and MQTT implementation to be handled by the user code, where the library would only provide mechanisms
for interpreting or composing the JSON that needs to be received or sent with underlying specific 
protocols.

Visit the [iotc-nrf-sdk](https://github.com/Avnet/iotc-nrf-sdk) or [iotc-wiced](https://github.com/Avnet/iotc-wiced) GitHub repositories 
for examples on use of this library.

## Features

* Composing of D2C JSON for MQTT telemetry messages
* Parsing C2D JSON for MQTT OTA messages and providing download URLs
* Parsing C2D JSON for MQTT command messages
* Composing D2C JSON for MQTT OTA and command acknowledgements
* Parsing HTTP discovery and sync JSON responses, in order to obtain DTG value, required for telemetry 
* Helpers for parsing raw HTTP headers for discovery and sync responses

# Example Usage

(Optional Step) Use HTTP sync rest call to obtain dtg for telemetry configuration. 
The response can be used to help configure telemetry module and your MQTT client code:

```c
#include "iotconnect_discovery.h"
// use http to get the JSON response from IOTCONNECT_DISCOVERY_HOSTNAME declared in iotconnect_discovery.h
// use your-cpid and your-env as part of the url:
const char* discovery_response_str = native_http_get(IOTCONNECT_DISCOVERY_HOSTNAME, "/api/sdk/cpid/your-cpid/lang/M_C/ver/2.0/env/your-env");

IotclDiscoveryResponse *dr;
dr = IOTC_DiscoveryParseDiscoveryResponse(discovery_response_str);
printf("Discovery Response:\n");
printf("url: %s\n", dr->url);
printf("host: %s\n", dr->host);
printf("path: %s\n", dr->path);

// pass either dr->url) or host/path combination to your http code to GET the sync reponse
const char * sync_response_str = native_http_get(dr->host, dr->path);

// Once we have obtained the sync response JSON, we should no longer need the discovery response
iotcl_discovery_free_discovery_response(dr);

IotclSyncResponse *sr = iotcl_discovery_parse_sync_response(sync_response_str);
// the reponse now contains the dtg and MQTT information that you can pass to telemetry module of this library and your MQTT client implementation 
printf("dtg: %s\n", sr->dtg);
printf("cpid: %s\n", sr->cpid);
printf("MQTT host: %s\n", sr->broker.host);
// ... etc.

// pass the info and initialize your MQTT client with the broker information
intialize_mqtt_client(sr->broker);

// Free the synce response once you are done using the information provided by it
iotcl_discovery_free_sync_response(gsr);
```

Configure the Telemetry module, compose and send a message to MQTT.  

```c
#include "iotconnect_lib.h"
#include "iotconnect_common.h"
#include "iotconnect_telemetry.h"

void send_telemetry() {
    
    IotclConfig config;
    memset(&config, 0, sizeof(config));
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";
    config.device.env = "prod";
    
    config.telemetry.dtg = "5a913fef-428b-4a41-9927-6e0f4a1602ba";
    // or pass from discovery response above
    config.telemetry.dtg = sr->dtg;
    
    iotcl_init(&config);
    
    IotclMessageHandle msg = iotcl_telemetry_create();
    // Initial entry will be created with system timestamp
    // You can call AddWith*Time before making Set* calls in order to add a custom timestamp
    
    iotcl_telemetry_set_number(msg, "number-value", 123);
    iotcl_telemetry_set_string(msg, "string-value", "myvalue");
    // ... etc. See the telemetry header file and tests/telemetry.c for more examples
    
    // construct json string from message handle (false = not pretty-formatted to reduce string size)
    const char *json_str = iotcl_create_serialized_string(msg, false);
    
    // We no longer need the message handle. We have the json string
    iotcl_telemetry_destroy(msg);
    
    // pass the string to mqtt sybsystem
    printf("%s\n", str);
    send_string_to_pub_topic(str);
    
    // we are done with the serialized string, so destroy it here
    iotcl_destroy_serialized(str);
}
```

Configure the Event module, process raw mqtt json strings and receive commands and OTA download URLs 
in your configured callback functions.

```c
#include "iotconnect_lib.h"

// declare on_cmd and on_ota callback handlers

void on_cmd(IotclEventData data) {
    const char *command = iotcl_clone_command(data);
    printf("Command is: %s\n", command); // handle the command string here... tokenize etc.
    free(command);

    // end ack true (success) to the command. you can also pass false (failure) and a message reponse
    const char *ack_json = iotcl_create_ack_string_and_destroy_event(data, true, NULL);
    send_string_to_pub_topic(ack_json)
    printf("Sent CMD ack: %s\n", ack);
    free(ack);
}

void on_ota(IotclEventData data) {
    const char *url = iotcl_clone_download_url(data, 0);
    const char *sw_ver = iotcl_clone_sw_version(data);
    const char *hw_ver = iotcl_clone_hw_version(data);
    printf("Download URL is: %s\n", url);
    printf("SW Version is: %s\n", sw_ver);
    printf("HW Version is: %s\n", hw_ver);

    // your handler for OTA ...
    app_download_ota(url);

    free(url);
    free(sw_ver);
    free(hw_ver);

    // end ack true (success) to the command. you can also pass false (failure) and a message reponse
    const char *ack = iotcl_create_ack_string_and_destroy_event(data, true, NULL);
    send_string_to_pub_topic(ack_json)
    printf("Sent CMD ack: %s\n", ack);
    free(ack);
}

void on_mqtt_sub_message(const char* json_str) {
    if (!iotcl_process_event(json_str)) {
        printf("Error encountered while processing %s\n", TEST_STR_V1);
    }
}

void confiigure_events() {
    IotclConfig config;
    memset(&config, 0, sizeof(config));

    config.device.env = "prod";
    config.device.cpid = "MyCpid";
    config.device.duid = "my-device-id";

    config.event_functions.ota_cb = on_ota;
    config.event_functions.cmd_cb = on_cmd;

    iotcl_init(&config);
}
```

## Dependencies

* cJSON library v1.7.13 or greater
* Time subsystem (time() function) needs to be available on the system

## Integration Notes

* The library attempts to be lightweight, allowing you to, for example, only include and
 build only telemetry-related source files into your project, so the user can choose to include only
 specific files with their platform's build system.  
* Include the desired source and header files fo this library into your embedded project.
* Follow examples in tests/ directory to learn how to initialize and use the components in your project.
 
