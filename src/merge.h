#ifndef MERGE_H
#define MERGE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct merge_context;

struct merge_context *merge_new(void *merge_data, size_t merge_data_size, size_t max_merge_files);
size_t merge_capacity(struct merge_context *merge);
bool merge_add_input_file(struct merge_context *merge, FILE *file);
bool merge_perform_merge(struct merge_context *merge, FILE *output_file, bool (*on_close_file)(FILE *file));
void merge_cleanup_merge(struct merge_context *merge, bool (*on_close_file)(FILE *file));
void merge_delete(struct merge_context *merge);

#endif // MERGE_H
