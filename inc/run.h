#ifndef RUN_H
#define RUN_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct run_context {
    FILE *input_file;
    size_t run_length;
    uint32_t *run_data;
};

bool run_begin(FILE *input_file, size_t run_length, struct run_context *run);
bool run_finished(struct run_context *run);
bool run_create_run(struct run_context *run, FILE *output_file);
bool run_end(struct run_context *run);

#endif // RUN_H
