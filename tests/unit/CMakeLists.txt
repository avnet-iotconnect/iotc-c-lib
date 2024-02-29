cmake_minimum_required(VERSION 3.8)

project(iotc-c-lib VERSION 3.0)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/../../core/include
        ${CMAKE_SOURCE_DIR}/../../modules/heap-tracker
        ${CMAKE_SOURCE_DIR}/../../modules/discovery-v2
        ${CMAKE_SOURCE_DIR}/cJSON
)

aux_source_directory(../../core/src iotc_c_lib_sources)
aux_source_directory(../../modules/heap-tracker heap_tracker_sources)
aux_source_directory(../../modules/discovery-v2 iotc_c_lib_sources)

aux_source_directory(cJSON cjson)
list(REMOVE_ITEM cjson cJSON/test.c)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOTCL_USER_CONFIG_FILE='\"iotcl_config.h\"' -Wall -Wextra")

add_executable(test-discovery ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} discovery.c)
add_executable(test-event ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} event.c)
add_executable(test-telemetry ${iotc_c_lib_sources} ${heap_tracker_sources} ${cjson} telemetry.c)
