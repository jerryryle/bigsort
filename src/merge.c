#include "merge.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

static bool write_uint32(int output_fd, uint32_t val)
{
    ssize_t num_written = write(output_fd, &val, sizeof(val));
    if (num_written != sizeof(val)) {
        // Error writing file
        return false;
    }
    return true;
}

enum read_uint32_result {
    READ_SUCCESS = 0,
    READ_ERROR,
    READ_EOF
};

static enum read_uint32_result read_uint32(int input_fd, uint32_t *val)
{
    ssize_t num_read = read(input_fd, val, sizeof(*val));
    if (num_read == sizeof(*val)) {
        return READ_SUCCESS;
    }
    if (num_read == 0) {
        return READ_EOF;
    }
    return READ_ERROR;
}

static bool copy_file(int input_fd, int output_fd) {
    enum read_uint32_result read_result;
    uint32_t val;

    // Read values until a read fails.
    while ((read_result = read_uint32(input_fd, &val)) == READ_SUCCESS) {
        // Write out the value
        if (!write_uint32(output_fd, val)) {
            return false;
        }
    }
    if (read_result == READ_ERROR) {
        // Read failure was due to an error reading the file
        return false;
    }
    // Read failure was due to end of file. This just means we're done.
    return true;
}

bool merge_files(int input1_fd, int input2_fd, int output_fd) {
    uint32_t val1;
    uint32_t val2;
    int read_result;

    // Read initial value from input1
    read_result = read_uint32(input1_fd, &val1);
    if (read_result == READ_ERROR) {
        return false;
    }
    if (read_result == READ_EOF) {
        // If there's no initial value from input1, the file must be empty.
        // In this case, just copy input2 to the output.
        return copy_file(input2_fd, output_fd);
    }

    // Read initial value from input2
    read_result = read_uint32(input2_fd, &val2);
    if (read_result == READ_ERROR) {
        return false;
    }
    if (read_result == READ_EOF) {
        // If there's no initial value from input2, the file must be empty.
        // In this case, just write the first value from input1 and copy the remainder of input1 to the output.
        if (!write_uint32(output_fd, val1)) {
            return false;
        }
        return copy_file(input1_fd, output_fd);
    }

    for (;;) {
        if (val1 <= val2) {
            if (!write_uint32(output_fd, val1)) {
                return false;
            }
            read_result = read_uint32(input1_fd, &val1);
            if (read_result == READ_ERROR) {
                return false;
            }
            if (read_result == READ_EOF) {
                // If we've reached the end of input1, then just write out the rest of input2
                if (!write_uint32(output_fd, val2)) {
                    return false;
                }
                return copy_file(input2_fd, output_fd);
            }
        } else {
            if (!write_uint32(output_fd, val2)) {
                return false;
            }
            read_result = read_uint32(input2_fd, &val2);
            if (read_result == READ_ERROR) {
                return false;
            }
            if (read_result == READ_EOF) {
                // If we've reached the end of input2, then just write out the rest of input1
                if (!write_uint32(output_fd, val1)) {
                    return false;
                }
                return copy_file(input1_fd, output_fd);
            }
        }
    }
    return false;
}
