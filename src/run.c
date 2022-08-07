#include "run.h"
#include <assert.h>
#include <stdlib.h>

struct run_context {
    FILE *input_file;
    size_t nelements;
    uint32_t *data;
    bool finished;
};

static int compare_uint32_t(void const *left, void const *right) {
    uint32_t left_int = *(uint32_t const *)left;
    uint32_t right_int = *(uint32_t const *)right;
    if (left_int < right_int) {
        return -1;
    }
    if (left_int > right_int) {
        return  1;
    }
    return 0;
}

struct run_context *run_new(FILE *input_file, size_t num_elements) {
    assert(input_file);
    assert(num_elements > 0);

    struct run_context *run = (struct run_context *)malloc(sizeof(struct run_context));
    if (!run) {
        return NULL;
    }

    run->input_file = input_file;
    run->nelements = num_elements;
    run->data = (uint32_t *)malloc(run->nelements * sizeof(uint32_t));
    if (!run->data) {
        free(run);
        return NULL;
    }
    run->finished = false;
    return run;
}

bool run_finished(struct run_context *run) {
    assert(run);
    return run->finished;
}

bool run_create_run(struct run_context *run, FILE *output_file) {
    assert(run);
    assert(output_file);

    // Read a run's worth of uint32_t
    size_t num_read = fread(run->data, sizeof(uint32_t), run->nelements, run->input_file);
    if (ferror(run->input_file)) {
        return false;
    }

    // If we read any data, sort it and write it to the run file
    if (num_read > 0) {
        qsort(run->data, num_read, sizeof(uint32_t), compare_uint32_t);
        fwrite(run->data, sizeof(uint32_t), num_read, output_file);
        if (ferror(output_file)) {
            return false;
        }
    }

    // If we read less than the run size of data, then we must be at the end of the file.
    if (num_read < run->nelements) {
        run->finished = true;
    }
    return true;
}

void run_delete(struct run_context* run) {
    if (run) {
        free(run->data);
        free(run);
    }
}
