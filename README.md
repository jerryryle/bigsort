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
1. Generate a 1MB test file named test.bin
   - `./bigfile.py test.in 1MB`
2. Sort it using a run size of 1KB
   - `./cmake-build-debug/bigsort --runsize=100000 test.in test.out`
3. Verify that the resulting file is sorted
   - `./check_sorted.py test.out`

## Assumptions
- I'm going to keep this simple for now and assume large files of fixed-sized records. Specifically, I'll sort large binary files filled with 32-bit, unsigned integers that are aligned to 32-bit boundaries. There's no particular reason for choosing unsigned other than they're slightly easier for me to visually interpret from a hex dump, should I need to.
- I interpret the endianness of the file according to the current system's endianness. I do not try to normalize it to either big or little.

## Research
My initial hunch is that a merge sort strategy will be most applicable to the problem of sorting files that are larger than system memory. In order to sort such files, expensive disk reads/writes will be required. Minimizing these will be a necessary part of an efficient solution. Furthermore, sequential disk access is usually more efficient than random disk access--this is especially true for magnetic and optical storage where random access can be hundreds of times slower than sequential due to the mechanical nature of those systems. So, an algorithm that works in blocks and moves sequentially through a file will be much faster than an algorithm that makes large jumps around the data.

Busting out Volume 3, Second Edition of Knuth's, "The Art of Computer Programming," I've learned that sorting data larger than system memory is called "External Sorting." Knuth Chapter 5.4 on External Sorting quickly concludes that internal sorting of small blocks and then merging them together is a particularly effective approach to the problem of External Sorting.

### Simple, 2-way merge sort
I'm going to begin by splitting the input file up into runs that are each individually sorted using C's built-in `qsort()` function. Each run will be written to its own file. I'll then merge pairs of run files into single, longer run files using a simple, two-way merge. I'll continue to merge successively longer runs until only a single, fully-sorted run remains. This method will ensure sequential access of files; however, it will result in a significant number of file writes and merges.

To avoid having to keep regenerating test input files, I will retain the original input file and place the sorted output into a new file. For a file of size F, this merge sort approach will require roughly 2F disk space to complete. However, because I retain the input file, the total disk space needed will be 3F.

This simple, 2-way merge sort achieves the goals of keeping memory usage fixed while also ensuring sequential file access through all steps of the process. However, it results in O(log N) rewriting of the input file, where N is the input file size, divided by the run size. Intermediate rewrites are discarded as the merging progresses, so the 3F disk space requirement still stands. But it's quite a bit of rewriting. An improvement would be to use a data structure to efficiently perform a k-way merge. The k-way merge would allow more runs to be simultaneously merged, requiring fewer merge phases and, thus, less rewriting.

### k-way merge sort
Having gotten a simple, 2-way merge working, I moved on to tackle a *k*-way merge. This uses the same run creation strategy as described in the 2-way merge section above. Therefore, this section only discusses the change to the merge algorithm that I made to enable the *k*-way merge.

I first implemented an array-based [minheap](https://en.wikipedia.org/wiki/Heap_(data_structure)) whose elements each consist of a `uint32_t` key and a `FILE *` value. An element's key is the next value read from a sorted run file. An element's value is a pointer to the opened run file from which the key was read. Bounded by available memory and open file limits, the heap can hold *k* elements, which means we can simultaneously open and merge up to *k* input run files.

The merge algorithm begins by opening up to *k* input run files. As each file is opened, its first `uint32_t` value is read from the file. This value and the associated file pointer are pushed onto the heap. After up to *k* run files are pushed onto the heap, the merge proceeds by popping the element from the top of the heap. The minheap guarantees that this element's key will be the smallest of the *k* values that were placed on the heap. This `uint32_t` key value is written to the output file. The next `uint32_t` value is read from the element's input run file pointer. This new key and the associated file pointer are pushed back on the heap. When the end of the input run file is reached (EOF) and a new value cannot be read, then the element is not pushed back onto the heap and the heap remains one element smaller. This pop/write/read/push-if-not-EOF sequence repeats until the heap is exhausted.

To be clear, this is effectively accomplishing the same thing as the simple 2-way merge--it's just doing so with more than 2 run files at a time. Specifically, from a set of *k* sorted run files, it is choosing the next smallest value and writing that to the output file. The minheap is simply an efficient way to find the next smallest value from a set of *k* values.

If the initial run processing produces more than *k* input run files, then the k-way merge uses the same generational merging strategy as the 2-way merge. First, *k* run files are merged into an output file. Then, up to the next *k* run files are merged into another output file. This proceeds until there are no more input run files. Then, the output files become the next generation of input files, which are merged together. This proceeds until there's only a single output file remaining. The fact that we can now handle *k* files in each generation means that fewer generations are required to complete the merge. This means that fewer disk operations are performed.

### Thoughts on Further Improvements
While i/o-efficient, the *k* way merge is still relatively slow and is likely not maximizing hardware usage. I would need to do some profiling to determine where the bottlenecks are, but it's possible that the algorithm is now CPU-bound since it's only using a single thread and, therefore, only runs on one processor core. It might be possible to gain more efficiency with the same memory footprint by using smaller values of *k* across multiple cores. This would mean that we're producing more output files with smaller sets of input files, but if we are indeed CPU-bound rather than I/O-bound, this could still be a performance gain at the cost of more I/O.
