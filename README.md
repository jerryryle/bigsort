# Sorting large files
A toy project to play with approaches to sorting large files. 

## Background
Problem: Sort a file on disk that is larger than available RAM.

## Getting Started
This project has been built with Jetbrains' CLion IDE. It can also be built and run with CMake.

### Requirements
* CMake >= v3.23
* Ninja >= v1.11
* C and C++ Toolchains with C17 and C++21 support, respectively.

### Build
`cmake --build cmake-build-debug --target all -j 12`

### Test
`(cd cmake-build-debug; ctest --extra-verbose --output-on-failure)`
