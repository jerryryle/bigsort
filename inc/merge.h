#ifndef MERGE_H
#define MERGE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

bool merge_files(int input1_fd, int input2_fd, int output_fd);

#endif // MERGE_H
