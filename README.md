# Damerau–Levenshtein Baseline Implementation

This repository contains a sequential C++ implementation of the **True Damerau–Levenshtein** algorithm,
developed as part of an undergraduate final-year research project.

## Features
- Supports insertion, deletion, substitution, and adjacent transposition
- Sequential baseline implementation (no parallelism or vectorisation)
- Batch benchmarking with averaged runtime measurements
- CSV output for performance analysis

## Build and Run
- Language: C++17
- Compiler: MSVC / GCC / Clang

Compile and run the main source file to execute unit tests and benchmarking.

## Notes
This repository represents the current progress stage of the project and serves as a baseline
for future SIMD and GPU-based optimisations.
