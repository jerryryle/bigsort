#include "runs.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

bool runs_check_file_size(FILE *input_file) {
    struct stat file_status;
    fstat(fileno(input_file), &file_status);
    if ((file_status.st_size & 0x03) == 0) {
        return true;
    }
    return false;
}

int runs_create(FILE* input_file, char const *output_filename, size_t run_length) {
    return 0;
}
