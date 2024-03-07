#!/bin/bash

set -e

sed 's#^\(.*\)$#"\1\\r\\n"#g' $1
