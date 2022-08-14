#include "merge.h"
#include <assert.h>
#include <stdlib.h>
#include "min_heap.h"

struct merge_context {
    struct min_heap* heap;
};

enum read_uint32_result {
    READ_SUCCESS = 0,
    READ_ERROR,
    READ_EOF
};

static bool do_merge(struct merge_context *merge, FILE * const *input_files, size_t num_input_files, FILE *output_file);
static bool add_input_file(struct merge_context *merge, FILE *file);
static bool write_uint32(FILE *output_file, uint32_t val);
static enum read_uint32_result read_uint32(FILE *input_file, uint32_t *val);

struct merge_context *merge_new(void *merge_data, size_t merge_data_size)
{
    struct merge_context *merge = (struct merge_context *)malloc(sizeof(struct merge_context));
    if (!merge) {
        return NULL;
    }
    merge->heap = min_heap_new(merge_data,merge_data_size);
    if (!merge->heap) {
        free(merge);
        return NULL;
    }
    return merge;
}

size_t merge_get_max_input_files(struct merge_context const *merge)
{
    assert(merge);
    return min_heap_capacity(merge->heap);
}

bool merge_perform_merge(
        struct merge_context *merge,
        FILE * const *input_files, size_t num_input_files,
        FILE *output_file)
{
    assert(merge);
    assert(input_files);
    assert(output_file);

    // Don't exceed our input file capacity.
    if (num_input_files > merge_get_max_input_files(merge)) {
        return false;
    }

    // Perform the merge
    bool success = do_merge(merge, input_files, num_input_files, output_file);

    // In the case of a failure, data may be left on the minheap.
    // Clear the heap so that it can be reused in subsequent merges.
    min_heap_clear(merge->heap);

    return success;
}

void merge_delete(struct merge_context *merge)
{
    if (merge) {
        min_heap_delete(merge->heap);
        free(merge);
    }
}

static bool do_merge(struct merge_context *merge, FILE * const *input_files, size_t num_input_files, FILE *output_file)
{
    // Add all of the files to the minheap.
    for (size_t i = 0; i< num_input_files; i++) {
        assert(input_files[i]);
        if (!add_input_file(merge, input_files[i])) {
            return false;
        }
    }

    uint32_t value = 0;
    FILE *input_file = NULL;

    while (min_heap_pop(merge->heap, &value, &input_file)) {

        // Write the smallest value to the output file.
        if (!write_uint32(output_file, value)) {
            return false;
        }

        // Read the next value from this input file and, if the file isn't empty, place it back on the heap
        enum read_uint32_result read_result = read_uint32(input_file, &value);
        if (read_result == READ_ERROR) {
            return false;
        }
        if (read_result != READ_EOF) {
            // Place the new value along with its file back on the heap.
            if (!min_heap_add(merge->heap, value, input_file)) {
                return false;
            }
        }
        // If we are at the end of the file, do nothing. Specifically, put nothing back on the heap.
        // Eventually the heap will be exhausted and thus the merge will be finished.
    }
    return true;
}

static bool add_input_file(struct merge_context *merge, FILE *file)
{
    uint32_t key;
    enum read_uint32_result result = read_uint32(file, &key);
    if (result == READ_ERROR) {
        // Couldn't read from the file. This is an error.
        return false;
    }
    if (result == READ_EOF) {
        // File was empty. This isn't an error, but don't add the file since there's nothing to process. Just return
        // success.
        return true;
    }

    // Add the first value and its associated file to the heap.
    if (!min_heap_add(merge->heap, key, file)) {
        return false;
    }

    return true;
}

static bool write_uint32(FILE *output_file, uint32_t val)
{
    fwrite(&val, sizeof(val), 1, output_file);
    return !ferror(output_file);
}

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

static void on_close_file_nop(FILE *file)
{
    assert(file);
}
