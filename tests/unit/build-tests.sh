#!/bin/bash

this_dir=$(dirname "$0")

pushd "$this_dir"
set -e

# pull in cJSON if it is not pulled in already
git submodule update --init --recursive

cmake .
cmake --build . --target test-rest-api test-event test-telemetry

popd