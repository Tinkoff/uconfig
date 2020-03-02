#!/bin/sh
set -e

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  ..
make -j$(nproc)
cd ..
