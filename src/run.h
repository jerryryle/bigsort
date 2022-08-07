#ifndef RUN_H
#define RUN_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


struct run_context;

struct run_context *run_new(FILE *input_file, size_t run_size);
bool run_finished(struct run_context *run);
bool run_create_run(struct run_context *run, FILE *output_file);
void run_delete(struct run_context *run);

#endif // RUN_H
