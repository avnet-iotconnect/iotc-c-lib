cmake_minimum_required(VERSION 3.8)

project(iotc-c-lib VERSION 3.0)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/../../core/include
        ${CMAKE_SOURCE_DIR}/../../modules/heap-tracker
        ${CMAKE_SOURCE_DIR}/../../modules/device-rest-api
        ${CMAKE_SOURCE_DIR}/../../lib/cJSON
)

aux_source_directory(../../core/src iotc_c_lib_sources)
aux_source_directory(../../modules/heap-tracker heap_tracker_sources)
aux_source_directory(../../modules/device-rest-api dra_sources)

aux_source_directory(../../lib/cJSON cjson)
list(REMOVE_ITEM cjson ../../lib/cJSON/test.c)

set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOTCL_USER_CONFIG_FILE='\"iotcl_config.h\"' -Wall -Wextra -std=c99")

# Throw in some extra warning checks. Feel free to adjust if needed. Not sure if some are included with each other.
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wextra -Wno-format-zero-length")


add_executable(test-discovery ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} ${dra_sources} device_rest_api.c)
add_executable(test-event ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} event.c)
add_executable(test-telemetry ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} telemetry.c)
