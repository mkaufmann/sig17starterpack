#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd "$DIR"

mkdir -p build/Release
cd build/Release
if [ ! -f Makefile ]; then
   cmake -DCMAKE_BUILD_TYPE=Release ../../
fi
make -j

#cmake -DCMAKE_BUILD_TYPE=Debug ../../ # For debug build