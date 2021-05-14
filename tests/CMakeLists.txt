cmake_minimum_required(VERSION 3.8)
message("hello")

project(iotc-c-lib VERSION 1.0)

include_directories(
        ${CMAKE_SOURCE_DIR}/../include
        ${CMAKE_SOURCE_DIR}/cJSON
)
#file(GLOB iotc_c_lib_sources RELATIVE "../src" "*.c")
aux_source_directory(../src iotc_c_lib_sources)
#
aux_source_directory(cJSON cjson)
list(REMOVE_ITEM cjson cJSON/test.c)
message("hello" ${cjson})

#add_library(cjson
#        STATIC
#        ${CMAKE_SOURCE_DIR}/cJSON/cJSON.c
#        )
add_executable(test-discovery ${iotc_c_lib_sources} ${cjson} discovery.c)
add_executable(test-event ${iotc_c_lib_sources} ${cjson} event.c)
add_executable(test-telemetry ${iotc_c_lib_sources} ${cjson} telemetry.c)
add_executable(test-request ${iotc_c_lib_sources} ${cjson} request.c)
