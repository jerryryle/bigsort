#include "bigsort.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "merge.h"
#include "run.h"

static bool check_file_size(FILE *input_file) {
    struct stat file_status = {0};
    fstat(fileno(input_file), &file_status);
    // Check that the file size is a multiple of 4. This bit magic checks that the lowest two bits are zero. If they
    // are, then the file size is a multiple of 4.
    return ((file_status.st_size & 0x03) == 0);
}

/*
 * This creates the initial sorted runs given an acquired run context.
 */
static size_t create_runs_with_context(struct run_context *run, char const *output_filename) {
    size_t num_runs = 0;
    while (!run_finished(run)) {
        // Format the next run filename using the output filename as a base. We'll use the format:
        // "[output_filename].[generation_number].[run_number]"
        // The generation number starts at zero for the initial runs. This will increment later
        // during the merging phase.
        char run_filename[PATH_MAX] = {0};
        snprintf(run_filename, sizeof(run_filename), "%s.0.%lu", output_filename, num_runs);

        // Create and open the run file
        FILE *run_file = fopen(run_filename, "wb");
        if (!run_file) {
            fprintf(stderr, "ERROR: unable to create run file: %s\n", strerror(errno));
            return 0;
        }

        // Generate the run
        bool success = run_create_run(run, run_file);

        // Close the run file
        fclose(run_file);

        if (!success) {
            fprintf(stderr, "ERROR: unable to create run.\n");
            return 0;
        }

        // Update run counter
        num_runs++;
    }
    return num_runs;
}

size_t create_runs(FILE *input_file, char const *output_filename, size_t run_size) {
    if (!check_file_size(input_file)) {
        fprintf(stderr, "ERROR: input file's size must be a multiple of 4.\n");
        return 0;
    }

    struct run_context *run = run_new(input_file, run_size/sizeof(uint32_t));
    if (!run) {
        fprintf(stderr, "ERROR: Failed to create run context\n");
        return 0;
    }

    size_t runs = create_runs_with_context(run, output_filename);

    run_delete(run);
    return runs;
}

/*
 * This "merges" a single sorted run. It does so by moving the current generation run file to the next generation. This
 * is simply a rename operation that updates the filename to reflect the new generation.
 */
static bool merge_single_run(
        char const *output_filename,
        size_t run_generation, size_t run_number,
        size_t new_generation, size_t new_run_number) {
    char input_run_filename[PATH_MAX] = {0};
    char output_run_filename[PATH_MAX] = {0};

    snprintf(input_run_filename, sizeof(input_run_filename),
             "%s.%lu.%lu", output_filename, run_generation, run_number);

    if (new_generation == 0) {
        // This is a special case that renames the final-generation run to the final output file.
        snprintf(output_run_filename, sizeof(output_run_filename),
                 "%s", output_filename);
    } else {
        snprintf(output_run_filename, sizeof(output_run_filename),
                 "%s.%lu.%lu", output_filename, new_generation, new_run_number);
    }

    // No need to copy data. Just rename the input file to the new output file.
    if (rename(input_run_filename, output_run_filename) != 0) {
        return false;
    }
    return true;
}

/*
 * This merges a pair of sorted runs. It does so by acquiring all input/output file resources and then passing those to
 * a library function that performs the actual merge.
 */
static bool merge_two_runs(
        char const *output_filename,
        size_t run_generation, size_t run_number,
        size_t new_generation, size_t new_run_number) {
    char input1_run_filename[PATH_MAX] = {0};
    char input2_run_filename[PATH_MAX] = {0};
    char output_run_filename[PATH_MAX] = {0};

    snprintf(input1_run_filename, sizeof(input1_run_filename),
             "%s.%lu.%lu", output_filename, run_generation, run_number);
    snprintf(input2_run_filename, sizeof(input2_run_filename),
             "%s.%lu.%lu", output_filename, run_generation, run_number+1);
    snprintf(output_run_filename, sizeof(output_run_filename),
             "%s.%lu.%lu", output_filename, new_generation, new_run_number);

    FILE *input1_run_file = fopen(input1_run_filename, "rb");
    if (!input1_run_file) {
        fprintf(stderr, "ERROR: unable to open run file: %s\n", strerror(errno));
        return false;
    }

    FILE *input2_run_file = fopen(input2_run_filename, "rb");
    if (!input2_run_file) {
        fprintf(stderr, "ERROR: unable to open run file: %s\n", strerror(errno));
        fclose(input1_run_file);
        return false;
    }

    // Create and open the output run file
    FILE *output_run_file = fopen(output_run_filename, "wb");
    if (!output_run_file) {
        fprintf(stderr, "ERROR: unable to create run file: %s\n", strerror(errno));
        fclose(input2_run_file);
        fclose(input1_run_file);
        return false;
    }

    // Perform the merge with the opened files.
    bool success = merge_files(input1_run_file, input2_run_file, output_run_file);

    // Close all files
    fclose(output_run_file);
    fclose(input2_run_file);
    fclose(input1_run_file);

    // Delete input files now that they've been merged into a new file
    if ((remove(input1_run_filename) != 0) || (remove(input2_run_filename) != 0)) {
        fprintf(stderr, "ERROR: unable to remove run file: %s\n", strerror(errno));
        return false;
    }

    return success;
}

size_t merge_runs(char const *output_filename, size_t num_runs) {
    size_t generation = 0; // Generation counter
    size_t num_runs_in_generation = num_runs;

    // Keep merging runs into new generations of longer runs until there are no more runs to merge.
    while (num_runs_in_generation >= 2) {
        size_t const output_generation = generation + 1;
        size_t input_current_run = 0;
        size_t num_runs_in_output_generation = 0;

        // Merge all runs in the current generation.
        while (input_current_run < num_runs_in_generation) {
            if (num_runs_in_generation - input_current_run < 2) {
                // If there's only one run left in the current generation, merge it with itself. This just
                // renames the file so it becomes a run in the next generation.
                if (!merge_single_run(
                        output_filename,
                        generation, input_current_run,
                        output_generation, num_runs_in_output_generation)) {
                    return 0;
                }
                // Update the run counter to reflect that we've merged one run
                input_current_run++;
            } else {
                // Merge the next two runs
                if (!merge_two_runs(
                        output_filename,
                        generation, input_current_run,
                        output_generation, num_runs_in_output_generation)) {
                    return 0;
                }
                // Update the run counter to reflect that we've merged two runs
                input_current_run += 2;
            }
            // Track how many runs we've produced in the next generation
            num_runs_in_output_generation++;
        }

        // Update the current generation for the next loop iteration.
        generation = output_generation;
        num_runs_in_generation = num_runs_in_output_generation;
    }

    // We've now merged down to a single run. Just rename the run file to the final output.
    if(!merge_single_run(output_filename, generation, 0, 0, 0)) {
        return 0;
    }

    // Return the number of generations that the merge took.
    return generation;
}
