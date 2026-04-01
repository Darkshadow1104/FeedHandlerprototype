#!/bin/bash

set -e

echo "Starting client..."

taskset -c 3 ./Build/client
