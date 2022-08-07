#include "run.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

struct run_context {
    int input_fd;
    size_t size;
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

struct run_context *run_new(int input_fd, size_t run_size) {
    assert(input_fd > -1);
    assert(run_size > 0);

    struct run_context *run = (struct run_context *)malloc(sizeof(struct run_context));
    if (!run) {
        return NULL;
    }

    run->input_fd = input_fd;
    run->size = run_size;
    run->data = (uint32_t *)malloc(run_size / sizeof(uint32_t));
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

bool run_create_run(struct run_context *run, int output_fd) {
    assert(run);
    assert(output_fd > -1);

    // Read a run's worth of data
    ssize_t num_read = read(run->input_fd, run->data, run->size);
    if (num_read < 0) {
        return false;
    }

    // If we read any data, sort it and write it to the run file
    if (num_read > 0) {
        qsort(run->data, num_read / sizeof(uint32_t), sizeof(uint32_t), compare_uint32_t);
        ssize_t num_written = write(output_fd, run->data, num_read);
        if (num_written < 0) {
            return false;
        }
    }

    // If we read less than the run size of data, then we must be at the end of the file.
    if (num_read < run->size) {
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
