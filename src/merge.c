#include "merge.h"
#include <assert.h>

static bool write_uint32(FILE *output_file, uint32_t val)
{
    fwrite(&val, sizeof(val), 1, output_file);
    return !ferror(output_file);
}

enum read_uint32_result {
    READ_SUCCESS = 0,
    READ_ERROR,
    READ_EOF
};

static enum read_uint32_result read_uint32(FILE *input_file, uint32_t *val)
{
    assert(val);

    size_t num_read = fread(val, sizeof(*val), 1, input_file);
    if (ferror(input_file)) {
        return READ_ERROR;
    }
    if (num_read != 1) {
        return READ_EOF;
    }
    return READ_SUCCESS;
}

static bool copy_file(FILE *input_file, FILE *output_file) {
    enum read_uint32_result read_result = READ_ERROR;
    uint32_t val = 0;

    // Read values until a read fails.
    while ((read_result = read_uint32(input_file, &val)) == READ_SUCCESS) {
        // Write out the value
        if (!write_uint32(output_file, val)) {
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

bool merge_files(FILE *input1_file, FILE *input2_file, FILE *output_file) {
    assert(input1_file);
    assert(input2_file);
    assert(output_file);

    uint32_t val1 = 0;
    uint32_t val2 = 0;
    enum read_uint32_result read_result = READ_ERROR;

    // Read initial value from input1
    read_result = read_uint32(input1_file, &val1);
    if (read_result == READ_ERROR) {
        return false;
    }
    if (read_result == READ_EOF) {
        // If there's no initial value from input1, the file must be empty.
        // In this case, just copy input2 to the output.
        return copy_file(input2_file, output_file);
    }

    // Read initial value from input2
    read_result = read_uint32(input2_file, &val2);
    if (read_result == READ_ERROR) {
        return false;
    }
    if (read_result == READ_EOF) {
        // If there's no initial value from input2, the file must be empty.
        // In this case, just write the first value from input1 and copy the remainder of input1 to the output.
        if (!write_uint32(output_file, val1)) {
            return false;
        }
        return copy_file(input1_file, output_file);
    }

    for (;;) {
        if (val1 <= val2) {
            if (!write_uint32(output_file, val1)) {
                break;
            }
            read_result = read_uint32(input1_file, &val1);
            if (read_result == READ_ERROR) {
                break;
            }
            if (read_result == READ_EOF) {
                // If we've reached the end of input1, then just write out the rest of input2
                if (!write_uint32(output_file, val2)) {
                    break;
                }
                return copy_file(input2_file, output_file);
            }
        } else {
            if (!write_uint32(output_file, val2)) {
                break;
            }
            read_result = read_uint32(input2_file, &val2);
            if (read_result == READ_ERROR) {
                break;
            }
            if (read_result == READ_EOF) {
                // If we've reached the end of input2, then just write out the rest of input1
                if (!write_uint32(output_file, val1)) {
                    break;
                }
                return copy_file(input1_file, output_file);
            }
        }
    }
    return false;
}
