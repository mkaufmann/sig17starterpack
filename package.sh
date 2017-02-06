#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd "$DIR"

/bin/tar --exclude='*.load' --exclude='*.data' --exclude='.*' --exclude='build' --exclude='submission.tar.gz' --exclude='*ngrams.txt' --exclude='test-harness' -czf  submission.tar.gz *
