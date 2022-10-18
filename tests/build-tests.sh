#!/bin/bash

this_dir=$(dirname "$0")

pushd "$this_dir"
set -e
if [[ ! -d cJSON ]]; then
  git clone --depth 1 --branch v1.7.13 https://github.com/DaveGamble/cJSON.git
fi
test -d cJSON

make
popd