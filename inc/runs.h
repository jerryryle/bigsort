#ifndef RUNS_H
#define RUNS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

bool runs_check_file_size(FILE *input_file);
int runs_create(FILE* input_file, char const *output_filename, size_t run_length);

#endif // RUNS_H
