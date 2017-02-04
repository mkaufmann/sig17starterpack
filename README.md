# Unofficial C++ Starter Pack for the ACM Sigmod Programming Contest 2017

This repository contains basic infrastructure to start a C/C++ based implementation for the 2017 ACM Sigmod Programing Contest [URL].
I am not affiliated with the organizers thus I can't make any guarantees about correctness.

Additionaly to porting the python reference implementation the repository also contains tools to efficently benchmark locally.
A workload generator allows to create arbitrarily large test workloads.
The corresponding test driver executes the generated workloads like in an offical setting.

## Build requirements:
 * CMake
 * C/C++ compiler
 * Google Test library

## Quick start
The project is CMake based and can be configured using the typical CMake commands.
Alternatively calling `./compile.sh` will compile all targets using the "Release" settings which use the maximum optimization level.


You can also immediately do a quick test benchmark with:

   `./bench_local sample.load`

This will automatically compile everything and will benchmark the reference implementation using the workload from the "sample.load" file.

## Further details
More workloads can be generated using the generator program. After calling `./compile.sh` it will be located under build/Release/bencher/generator.
A larger workload can e.g. be generated using:

   `build/Release/bencher/generator /usr/share/myspell/dicts/en_US.dic 5000 400 20 20 1 100000 12 0.1 1988 > 100000.load`

Note: Using a dictionary as source for the ngrams will probably not lead to a representativ workloads as there will be at most one word per ngram.

The `package.sh` file is configured to automatically package the whole project excluding the build folder, workload files and hidden files.

If not specified otherwise all files fall under the license declared in the file LICENSE.