#!/bin/bash

set -e

echo "Running full demo..."

# build first
./scripts/build.sh

# start server in background
taskset -c 2 ./Build/server &
SERVER_PID=$!

echo "Server started (PID=$SERVER_PID)"
sleep 1

# start client
taskset -c 3 ./Build/client

# cleanup on exit
kill $SERVER_PID
