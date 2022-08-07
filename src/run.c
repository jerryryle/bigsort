#include "run.h"
#include <assert.h>
#include <stdlib.h>

bool run_begin(FILE *input_file, size_t run_length, struct run_context* run) {
    assert(input_file);
    assert(run_length > 0);
    assert(run);

    run->input_file = input_file;
    run->run_length = run_length;
    run->run_data = (uint32_t *)malloc(run_length/sizeof(uint32_t));
    if (!run->run_data) {
        return false;
    }
    return true;
}

bool run_finished(struct run_context *run) {
    assert(run);
    return false;
}

bool run_create_run(struct run_context *run, FILE* output_file) {
    assert(run);
    assert(output_file);
    return false;
}

bool run_end(struct run_context* run) {
    assert(run);
    if (run->run_data) {
        free(run->run_data);
        run->run_data = NULL;
        return true;
    }
    return false;
}
