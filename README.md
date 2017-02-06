# Unofficial C++ Starter Pack for the ACM Sigmod Programming Contest 2017

This repository contains basic infrastructure to start a C/C++ based implementation for the [2017 ACM Sigmod Programing Contest](http://sigmod17contest.athenarc.gr/index.shtml).
I am not affiliated with the organizers thus I can't make any guarantees about correctness.

Additionally to porting the python reference implementation the repository also contains tools to efficiently benchmark locally.
A workload generator allows to create arbitrarily large test workloads.
The corresponding test driver executes the generated workloads like in an official setting.

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
The benchmark harness in this project is different from the official harness. The official harness requires that the complete workload fits into
memory. The harness in this project only has the requirement that a single operation batch fits into memory.

## Generator
More workloads can be generated using the generator program. After calling `./compile.sh` it will be located under build/Release/bencher/generator.
A larger workload can e.g. be generated using:

   `build/Release/bencher/generator /usr/share/myspell/dicts/en_US.dic 5000 400 20 20 1 100000 12 0.1 1988 > 100000.load`

The program supports the following parameters:

   `generator <ngramFile> <initialNgramCount> <weightQuery> <weightAdd> <weightRemove> <weightFlush> <numOperations> <avgNgramsPerDoc> <ngramMatchProb> (seed)`

   * ngramFile: This is a file containing one ngram per line. These are used both for creating the add/delete operations as well as for synthesizing the documents.
   * initialNgramCount: How many ngrams should be indexed before sending operations
   * weight*: These parameters give integer weights to the different operations.
   * numOperations: Specifies how many operations are generated in total.
   * avgNgramsPerDoc: Specifies how many ngram there should be in average per document
   * ngramMatchProb: Set the probability with which a ngram in the document matches the currently indexed ngrams.

Note: Using a dictionary as source for the ngrams will probably not lead to a representative workloads as there will be at most one word per ngram.
Additionally compared to the official workload the match probability will probably be too uniform.

# Stat Runner & Next Generator

The normal generator generates can create very artificial workloads.
The official published workloads use real text which does not have a uniform distribution for the ngrams used in the document.
To extract the information about the ngrams used inside the documents and the ngrams that are search for the "statrunner" program was added.
Running the test-harness with the stat runner give thes following output:

`test-harness/harness test-harness/small.init test-harness/small.work test-harness/small.result build/Release/reference/statrunner`

    Initial: 2000
    NumQueries: 500
    NumAdditions: 2000
    NumDeletes: 400
    NumFlushes: 3
    WordsPerDoc: 5673
    MatchProbability: 0.0304828

Additionally it will also create the files "search_ngrams.txt" and "doc_ngrams.txt".
These can be used with the "next_generator" to create more natural workloads.

Example:

`build/Release/bencher/next_generator search_ngrams.txt doc_ngrams.txt 1500 500 1100 1000 3 10000 5670 0.0340 1988 > workloads/10000_real.load`

If you use the search_ngrams file as extracted from the official small workload beware that it only contains a limited number of ngrams.
To create a workload containing many additions or that runs for a long time you will have to add more to that file.

## Packaging
The `package.sh` file is configured to automatically package the whole project excluding the build folder, workload files and hidden files.

If not specified otherwise all files fall under the license declared in the file LICENSE.