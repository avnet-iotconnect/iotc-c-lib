# iotc-c-lib

iotc-c-lib is a library which abstracts IoTConnect MQTT protocol messages,
device configuration and the Discovery/Identity HTTP protocol

Use the main branch for [protocol 2.1](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/) devices.

Use the rel-protocol-1.0 branch for [protocol 1.0](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-1-0/) devices.

This library not an SDK. Each target platform/os generally has its own MQTT and HTTP 
implementation, so the library does not have direct dependencies on any specific one. 

The library provides mechanisms for parsing JSON notifications from the cloud (c2d) and composing JSON
telemetry and acknowledgement messages. The general use case would require os/platform specific HTTP 
and MQTT implementation to be handled by the user code, where the library would only provide mechanisms
for interpreting or composing the JSON that needs to be received or sent with underlying specific 
protocols.

## Dependencies

* cJSON library v1.7.13 or greater - v1.7.17 is included as submodule at lib/cJSON.
* A dynamic memory management facility. For example malloc, FreeRTOS heap or ThreadX memory pools. 

## Licensing

This library is distributed under the [MIT License](LICENSE.md).

## Protocol Features

* Composing [Telemetry](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages/#Device) messages.
* Parsing [C2D OTA](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/c2d-messages/#OTA) messages and providing download details.
* Parsing [C2D Command](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/c2d-messages/#Device) messages.
* Composing [OTA and command acknowledgements](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages) acknowledgements.
* Parsing HTTP [discovery](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/discovery-api/)
and [identity](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/identity-api/) Composing [Telemetry](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages/#Device) messages.

## General Features
* Easy to use message parsing and composition.
* Customizable dynamic memory allocation for the library and the cJSON dependency.
* IoTConnect topic name generation for AWS embedded devices which do not have HTTP clients or not enough resources 
 to implement HTTPS protocol or the discovery mechanism.
* Customizable error reporting.
    * Reduces the amount of error handling required by the client application or SDK.
    * Provides an option to completely remove logging and save on const strings RAM/ROM footprint.
    * Logging with optional error handling hooks, to potentially reset or halt the device.
* Optional telemetry timestamp reporting for devices which have access to SNTP or battery backed clock.  

# Library Integration and Examples 

Before using the library, one should read the header comments at [iotcl.h](core/include/iotcl.h)
to get familiar with the library concepts, goals, features and configuration.

For a quick-start, follow the links to examples to learn how to use the library:
* [Sending Telemetry](docs/examples/01-telemetry.md)
* [Receiving command and OTA messages and sending acknowledgements](docs/examples/02-c2d.md)

Reference Implementations:
* [IoTConnect Generic C SDK](https://github.com/avnet-iotconnect/iotc-generic-c-sdk) - Paho OpenSSL implementation 
for Windows/Linux/MacOS and similar operating systems. 

If you need to generate your own certificates for device testing:
* [Self-signed certificates with OpenSSL](tools/cert-generation-self-signed).
* [CA-signed certificates with OpenSSL](tools/cert-generation-ca). 

Also note that server CA certificates in C string PEM format are also available in library the sources at [iotcl_certs.h](core/include/iotcl_certs.h)  

See [unit test examples](tests/unit) for working samples that can compile and run with CMake and a PC compiler.

## Integration Notes

* Provide cJSON library v1.7.13 or greater to your build. If you already have an older version, you will need to upgrade it. 
* Add relevant include directories to your includes and add sources to your build.
* Follow examples in this document and examples in the tests/unit/ directory to learn how to initialize and use the components in your project.
* Review iotcl_sample_config.h to make sure that default logging configuration for example will meet your needs.
 If needed, create your own configuration file, add it to the include path and and pass it to the compiler 
 with -DIOTCL_USER_CONFIG_FILE="iotcl_config.h" **with the quotes in the actual define**. 
 See [tests/uint/CMakeLists.txt](tests/uint/CMakeLists.txt) for an reference example.
* If you have SNTP, battery backed clock, network time from the mobile network or similar, consider providing a time function 
to timestamp messages. You can skip this option even if you have the needed facilities in order to save on network bandwidth and
let the server timestamp messages as they arrive. Note that in this case time is not available, 
you should not be sending "bulk" telemetry messages with iotcl_telemetry_add_new_data_set().
* Read the instructions in iotcl.h and relevant function to learn how properly configure the library to fit your needs best. 
 
