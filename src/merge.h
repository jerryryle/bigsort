#ifndef MERGE_H
#define MERGE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool merge_files(FILE *input1_file, FILE *input2_file, FILE *output_file);

#endif // MERGE_H
