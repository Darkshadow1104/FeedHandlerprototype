#!/bin/bash

set -e

echo "Building project..."

BUILD_DIR=Build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake ..
make -j$(nproc)

echo "Build completed."
