#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd "$DIR"

/bin/tar --exclude='*.load' --exclude='*.data' --exclude='.*' --exclude='build' -czf  submission.tar.gz *