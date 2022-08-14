#ifndef MERGE_H
#define MERGE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct merge_context;

struct merge_context *merge_new(void *merge_data, size_t merge_data_size);

size_t merge_get_max_input_files(struct merge_context const *merge);

bool merge_perform_merge(
        struct merge_context *merge,
        FILE *const *input_files, size_t num_input_files,
        FILE *output_file);

void merge_delete(struct merge_context *merge);

#endif // MERGE_H
