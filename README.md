# Sorting large files
A toy project to play with approaches to sorting large files. 

## Background
Problem: Sort a file on disk that is larger than available RAM.

## Getting Started
This project has been built with Jetbrains' CLion IDE. It can also be built and run with CMake.

### Requirements
* CMake >= v3.23
* Ninja >= v1.11
* C and C++ Toolchains with C17 and C++20 support, respectively.

### Build
`cmake --build cmake-build-debug --target all -j 12`

### Test
`(cd cmake-build-debug; ctest --extra-verbose --output-on-failure)`

### Generate a 1GB test file named test.bin
`./bigfile test.bin 1GB`

## Assumptions
- I'm going to keep this simple for now and assume large files of fixed-sized records. Specifically, I'll sort large binary files filled with 32-bit, unsigned integers that are aligned to 32-bit boundaries. There's no particular reason for choosing unsigned other than they're slightly easier for me to visually interpret from a hex dump, should I need to.

## Research
My initial hunch is that a merge sort strategy will be most applicable to the problem of sorting files that are larger than system memory. In order to sort such files, expensive disk reads/writes will be required. Minimizing these will be a necessary part of an efficient solution. Furthermore, sequential disk access is usually more efficient than random disk access--this is especially true for magnetic and optical storage where random access can be hundreds of times slower than sequential due to the mechanical nature of those systems. So, an algorithm that works in blocks and moves sequentially through a file will be much faster than an algorithm that makes large jumps around the data.

Busting out Volume 3, Second Edition of Knuth's, "The Art of Computer Programming," I've learned that sorting data larger than system memory is called "External Sorting." Knuth Chapter 5.4 on External Sorting quickly concludes that internal sorting of small blocks and then merging them together is a particularly effective approach to the problem of External Sorting.

### Simple, 2-way merge sort
I'm going to begin by splitting the input file up into runs that are each individually sorted using C's built-in `qsort()` function. Each run will be written to its own file. I'll then merge pairs of run files into single, longer run files using a simple, two-way merge. I'll continue to merge successively longer runs until only a single, fully-sorted run remains. This method will ensure sequential access of files; however, it will result in a significant number of file writes and merges.

To avoid having to keep regenerating input files, I will retain the original input file and place the sorted output into a new file. For a file of size N, this merge sort approach will require roughly 2N disk space to complete. However, because I retain the input file, the total disk space needed will be 3N.
