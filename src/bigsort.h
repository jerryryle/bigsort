#ifndef BIGSORT_H
#define BIGSORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * This creates the initial sorted runs. It acquires needed resources, calls another function to create the runs, and
 * then ensures that the resources are released.
 *
 * Returns: The number of runs created (will always be at least 1 if creation succeeds), or zero if an error occurs.
 */
size_t create_runs(FILE *input_file, char const *output_filename, void *run_data, size_t run_data_size);

/*
 * This merges the initial, sorted runs down into a single, fully sorted, fully merged file.
 * It does so by first merging each pair of first-generation runs into larger, next-generation runs. It then proceeds
 * to merge pairs of next-generation runs into even larger next-next-generation runs. This continues until only one
 * large, final-generation runs remains. This is then renamed to the final output file.
 *
 * Returns: The number of generations that the merge required (will always be at least 1 if merge succeeds), or zero if
 *          an error occurs.
 */
size_t merge_runs(char const *output_filename, size_t num_runs);


#endif // BIGSORT_H
