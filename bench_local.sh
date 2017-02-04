#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd "$DIR"

if [ "$#" -ne 1 ]; then
    echo "Error: Invalid number of parameters"
    echo "Usage: ./bench_local.sh <workload_file>"
    exit 1
fi

./compile.sh
build/Release/bencher/bencher "$1" ./run.sh