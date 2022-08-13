#include "merge.h"
#include <assert.h>
#include <stdlib.h>
#include "min_heap.h"

struct merge_context {
    struct min_heap* heap;
    size_t file_capacity;
    size_t file_count;
};

enum read_uint32_result {
    READ_SUCCESS = 0,
    READ_ERROR,
    READ_EOF
};

static bool write_uint32(FILE *output_file, uint32_t val);
static enum read_uint32_result read_uint32(FILE *input_file, uint32_t *val);
static bool on_close_file_nop(FILE *file);

struct merge_context *merge_new(void *merge_data, size_t merge_data_size, size_t max_merge_files)
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

    // Set the merge capacity to the smaller of the minheap capacity or the specific maximum number of files to merge.
    // If max_merge_files is zero, then just use the minheap capacity.
    merge->file_capacity = min_heap_capacity(merge->heap);
    if ((max_merge_files > 0) && (max_merge_files < merge->file_capacity)) {
        merge->file_capacity = max_merge_files;
    }

    return merge;
}

size_t merge_capacity(struct merge_context *merge)
{
    assert(merge);
    return merge->file_capacity;
}

bool merge_add_input_file(struct merge_context *merge, FILE *file)
{
    assert(merge);
    assert(file);

    // Don't exceed our file capacity.
    if (merge->file_count >= merge->file_capacity) {
        return false;
    }

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
    merge->file_count++;

    return true;
}

bool merge_perform_merge(struct merge_context *merge, FILE *output_file, bool (*on_close_file)(FILE *file))
{
    assert(merge);
    assert(output_file);

    if (!on_close_file) {
        on_close_file = on_close_file_nop;
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
        if (read_result == READ_EOF) {
            // If we're at the end of the file, close the file and don't put anything back on the heap.
            // Eventually the heap will be exhausted and we'll be finished.
            if (!on_close_file(input_file)){
                return false;
            }
        } else {
            // Place the new value along with its file back on the heap.
            if (!min_heap_add(merge->heap, value, input_file)) {
                return false;
            }
        }
    }
    return true;
}

void merge_cleanup_merge(struct merge_context *merge, bool (*on_close_file)(FILE *file))
{
    assert(merge);

    uint32_t value = 0;
    FILE *input_file = NULL;

    if (!on_close_file) {
        on_close_file = on_close_file_nop;
    }

    // Close any remaining open files in the heap. There should not normally be anything left in the heap,
    // but if the merge fails, heap elements may remain.
    while (min_heap_pop(merge->heap, &value, &input_file)) {
        on_close_file(input_file);
    }

    merge->file_count = 0;
}

void merge_delete(struct merge_context *merge)
{
    if (merge) {
        min_heap_delete(merge->heap);
        free(merge);
    }
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

static bool on_close_file_nop(FILE *file)
{
    assert(file);
    return true;
}
