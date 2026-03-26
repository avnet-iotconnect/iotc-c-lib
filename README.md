# iotc-c-lib

iotc-c-lib is a library which abstracts /IOTCONNECT MQTT protocol messages,
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

* cJSON library v1.7.13 or greater - v1.7.19 is included as submodule at lib/cJSON.
* A dynamic memory management facility. For example malloc, FreeRTOS heap or ThreadX memory pools. 

## Licensing

This library is distributed under the [MIT License](LICENSE.md).

## Protocol Features

* Composing [Telemetry](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages/#Device) messages.
* Parsing [C2D OTA](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/c2d-messages/#OTA) messages and providing download details.
* Parsing [C2D Command](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/c2d-messages/#Device) messages.
* Composing [OTA and command acknowledgements](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages).
* Parsing HTTP [discovery](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/discovery-api/)
and [identity](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/identity-api/) Composing [Telemetry](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/d2c-messages/#Device) messages.
* Device Config JSON parsing - iotcDeviceConfig.json at the device *Info* page.

## General Features
* Easy to use message parsing and composition.
* Customizable dynamic memory allocation for the library and the cJSON dependency.
* /IOTCONNECT topic name generation for AWS embedded devices which do not have HTTP clients or not enough resources 
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
* [Using Discovery/Identity REST API](docs/examples/03-identity.md) to configure your MQTT connections.

Reference Implementations:
* [/IOTCONNECT MTB SDK](https://github.com/avnet-iotconnect/avnet-iotc-mtb-sdk) 
  Implementation for Infineon ModusToolbox using iotc-c-lib as a a git submodule and directly added to sources.
* [Microchip AVR Arduino SDK](https://github.com/avnet-iotconnect/iotc-arduino-mchp-avr-sdk) - 
  A low level hack example where c-lib files are dropped directly into the Arduino project.
* [/IOTCONNECT GreenGrass C SDK](https://github.com/avnet-iotconnect/iotc-greengras-c-sdk)
  AWS GreenGrass SDK and Component examples. This project uses the CMakeLists.txt in the root of the library.
* [/IOTCONNECT Generic C SDK](https://github.com/avnet-iotconnect/iotc-generic-c-sdk) - Paho OpenSSL implementation 
for Windows/Linux/MacOS and similar operating systems.
This project is the best way for users to get familiar with the library and test it on their machines.
However, the SDK has not been updated in a while and does not use the latest version of the library or the top level CMakeLists.txt. 

If you need to generate your own certificates for device testing:
* [Self-signed certificates with OpenSSL](tools/cert-generation-self-signed).
* [CA-signed certificates with OpenSSL](tools/cert-generation-ca). 

Also note that server CA certificates in C string PEM format are also available in library the sources at [iotcl_certs.h](core/include/iotcl_certs.h)  

See [unit test examples](tests/unit) for working samples that can compile and run with CMake and a PC compiler.


# Building and Testing

If using the library using a non-CMake build system, you can simply include the library sources in your build.
The best practice is to include this library as a submodule in your project.

The library supports building and including the library with CMake. 
You can add the library to your project as a submodule and then link to it with CMake's `add_subdirectory` and `target_link_libraries` commands.
See the Greengrass SDK as the reference implementation for CMake integration.

Build the tests with:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug # this will build the library and the tests in debug mode
make -C build -j$(nproc) # If not using Git Bash on windows, set the -j flag accordingly or omit it
/build/bin/test_telemetry # Run the telemetry test binary
```


# Development and Contribution

Please read the [CONTRIBUTING.md](CONTRIBUTING.md) before contributing to the project.

We recommend using VSCode on Linux for making modifications to the library.

We recommend using PyCharm's Git integration for Git work as it gives the user more visibility into the changes being made 
and better control over what ought to be committed.

* Install VSCode along with C/C++, clangd, **C/C++ DevTools** and **CMake Tools** extensions.
> [!NOTE]
> Later, when launching VSCode, if asked to disable C/C++ extension's default IntelliSense engine, 
> select an option to *Disable C/C++ extension Intellisense* and use clangd instead for better code analysis and navigation.

* Make sure you have CMake and a C compiler toolchain installed on your system. On Linux:

```bash
sudo apt update
sudo apt install build-essential cmake
```

* Clone this repository and open it with *Open Folder* in VSCode.
* Validate that you can build the tests with the instructions in the [Building and Testing](#building-and-testing) section above.
* Configure the CMake project with CMake Tools extension:
  * Open the *Command Palette* (Ctrl+Shift+P) and run CMake: Select a Kit.  The extension will scan your system and list available compilers (e.g., GCC, Clang). 
  * Choose the compiler you want to use (It will be GCC if you followed the steps above).
* On the left panel click the CMake button.
* In *Project Outline*, select a test target under *iotc-c-lib*, right click and select *Build* or *Debug* to run it.
* Optionally, you can set breakpoints in the code and debug the tests to step through the library code.
* If adding a new feature, please make sure to add at least a positive flow test for it in the tests/unit/ directory.


## Custom Integration Notes

If you are integrating the library into your own project, here are some general notes that may help your integration,
if you are using a non-CMake build system:

* Provide the required version of the cJSON library in your build or use the submodule in the lib directory.  
* Add relevant include directories to your includes and add sources to your build.
* Follow examples in this document and examples in the tests/unit/ directory to learn how to initialize and use the components in your project.
* Review [iotcl_example_config.h](core/include/iotcl_example_config.h) to make sure that default logging configuration for example will meet your needs.
 If needed, create your own configuration file, add it to the include path and pass it to the compiler 
 with -DIOTCL_USER_CONFIG_FILE="iotcl_config.h" **with the quotes in the actual define**.
 Please note that if you are building the iotc-c-lib as a shared or static library, you should make sure that the library 
 is built with the same configuration as your project.
 See [CMakeLists.txt](CMakeLists.txt) for more details.
* If you have SNTP, battery backed clock, network time from the mobile network or similar, consider providing a time function
to timestamp messages. You can skip this option even if you have the needed facilities in order to save on network bandwidth and
let the server timestamp messages as they arrive. Note that in this case, timestamping is not available, 
so you should not be sending "bulk" telemetry messages with iotcl_telemetry_add_new_data_set().
* Read the instructions in iotcl.h and relevant function to learn how properly configure the library to fit your needs best.
