#!/bin/bash

set -e

echo "Starting server..."

taskset -c 2 ./Build/server
