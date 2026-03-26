# iotc-c-lib - IoTConnect C Library
# Copyright (C) 2020-2026 Avnet
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22)
project(iotc-c-lib VERSION 3.0 LANGUAGES C)

option(IOTCL_BUILD_TESTS "Build tests and examples" ${PROJECT_IS_TOP_LEVEL})
option(IOTCL_ENABLE_DRA "Build Device REST API module" ON)
option(IOTCL_ENABLE_HEAP_TRACKER "Build heap tracker module" OFF)

# Tests depend on all modules
if(IOTCL_BUILD_TESTS)
  set(IOTCL_ENABLE_DRA ON CACHE BOOL "" FORCE)
  set(IOTCL_ENABLE_HEAP_TRACKER ON CACHE BOOL "" FORCE)
endif()

set(ENABLE_CJSON_TEST OFF CACHE BOOL "cJSON - Build Tests")
set(ENABLE_CUSTOM_COMPILER_FLAGS OFF CACHE BOOL "cJSON - Custom Compiler Flags")

# Disable warnings from cJSON about Compatibility with CMake < 3.5 will be removed from a future version of [cmake]   CMake.
# This triggers on cmake 4.0 and even 3.28.3. Unsure exactly why this happens.
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/cJSON EXCLUDE_FROM_ALL)
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

# Core sources
set(IOTCL_SOURCES
  core/src/iotcl.c
  core/src/iotcl_c2d.c
  core/src/iotcl_internal.c
  core/src/iotcl_telemetry.c
  core/src/iotcl_util.c
)

# Optional modules
if(IOTCL_ENABLE_DRA)
  list(APPEND IOTCL_SOURCES
    modules/device-rest-api/iotcl_dra_discovery.c
    modules/device-rest-api/iotcl_dra_identity.c
    modules/device-rest-api/iotcl_dra_json_config.c
    modules/device-rest-api/iotcl_dra_url.c
  )
endif()

if(IOTCL_ENABLE_HEAP_TRACKER)
  list(APPEND IOTCL_SOURCES
    modules/heap-tracker/heap_tracker.c
  )
endif()

# Library
if(PROJECT_IS_TOP_LEVEL)
  add_library(iotc-c-lib ${IOTCL_SOURCES})
else()
  add_library(iotc-c-lib EXCLUDE_FROM_ALL ${IOTCL_SOURCES})
endif()

target_link_libraries(iotc-c-lib PRIVATE cjson)

target_include_directories(iotc-c-lib PUBLIC core/include)
target_include_directories(iotc-c-lib PRIVATE lib/cJSON)

if(IOTCL_ENABLE_DRA)
  target_include_directories(iotc-c-lib PUBLIC modules/device-rest-api)
endif()

if(IOTCL_ENABLE_HEAP_TRACKER)
  target_include_directories(iotc-c-lib PUBLIC modules/heap-tracker)
endif()

target_compile_features(iotc-c-lib PUBLIC c_std_99)
target_compile_options(iotc-c-lib PRIVATE
  -Wall -Wextra -pedantic
  -Wno-format-zero-length
  -Wfloat-conversion -Wconversion -Wdouble-promotion
)

# Optional user config file
if(DEFINED IOTCL_USER_CONFIG_FILE)
  target_compile_definitions(iotc-c-lib PUBLIC
    IOTCL_USER_CONFIG_FILE="${IOTCL_USER_CONFIG_FILE}"
  )
endif()

# Install
if(PROJECT_IS_TOP_LEVEL)
  include(GNUInstallDirs)
  install(TARGETS iotc-c-lib)
endif()

# Tests and examples
if(IOTCL_BUILD_TESTS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

  file(GLOB TEST_SRCS CONFIGURE_DEPENDS "tests/unit/*.c")
  foreach(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WLE)
    add_executable(test_${test_name} ${test_src})
    target_link_libraries(test_${test_name} PRIVATE iotc-c-lib)
    target_include_directories(test_${test_name} PRIVATE tests/unit)
    target_compile_definitions(test_${test_name} PRIVATE
      IOTCL_USER_CONFIG_FILE="iotcl_config.h"
    )
  endforeach()
endif()
