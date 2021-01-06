# iotc-c-lib

This is a C library intended for generally for embedded systems which abstracts IoTConnect MQTT protocol messages
and the discovery HTTP protocol.

This library not an SDK. As each embedded platform/os generally has its own MQTT and HTTP 
implementation, the library does not have direct dependentcies on any specific one. 

The library only abstracts JSON responses from the cloud and provides mechanisms for composing JSON
messages that need to be sent to the cloud. The general use case would require os/platform specific HTTP 
and MQTT implementation to be handled by the user code, where the library would only provide mechanisms
for interpreting or composing the JSON that needs to be received or sent with underlying specific 
protocols.

## Features

* Composing of D2C JSON for MQTT telemetry messages
* Parsing C2D JSON for MQTT OTA messages and providing download URLs
* Parsing C2D JSON for MQTT command messages
* Composing D2C JSON for MQTT OTA and command acknowledgements
* Parsing HTTP discovery and sync JSON responses, in order to obtain DTG value, required for telemetry  

## Dependencies

* cJSON library v1.7.13 or greater
* Time subsystem (time() function) needs to be available on the system

## Integration Notes

* 
* The library attempts to be lightweight, allowing you to, for example, only include and
 build only telemetry-related source files into your project, so the user can choose to include only
 specific files with their platform's build system.  
* Include the desired source and header files fo this library into your embedded project.
* Follow examples in tests/ directory to learn how to make initialize and use your project. 