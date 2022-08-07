#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "merge.h"
#include "run.h"

static size_t const DEFAULT_RUN_SIZE = (size_t)1 * (1<<20); // (1<<20) is 1MB

struct options {
    bool print_help;
    char const *input_filename;
    char const *output_filename;
    size_t run_size;
};

void print_usage() {
    printf(
            "usage: bigsort [-h] [-r runsize] infile outfile\n" \
            "\n" \
            "Sort a large file filled with unsigned, 32-bit integers\n" \
            "\n" \
            "positional arguments:\n" \
            "  infile                  input file name\n" \
            "  outfile                 output file name\n" \
            "\n" \
            "optional arguments:\n" \
            "  -h, --help              show this help message and exit\n" \
            "  -r, --runsize=SIZE      size of initial runs\n"
            "                          (which also determines memory usage)\n");
}

size_t round_up_to_multiple_of_4(size_t num) {
    // This rounds up by adding 3 and then clearing the lowest two bits, ensuring a multiple of 4
    return (num + 3) & ~0x3;
}

void get_options(int argc, char * const argv[], struct options *opts) {
    static struct option const long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"runsize", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    // Set default options
    opts->print_help = false;
    opts->input_filename = NULL;
    opts->output_filename = NULL;
    opts->run_size = DEFAULT_RUN_SIZE;

    // Loop over arguments, looking for any option flags. These must come before any positional arguments.
    for (;;) {
        int opt = getopt_long(argc, argv, "hr:", long_options, NULL);
        if (opt == -1) {
            break;
        }
        switch (opt) {
        case 'h':
            opts->print_help = true;
            break;
        case 'r':
            opts->run_size = (size_t)strtoul(optarg, NULL, 0);
            break;
        default:
            break;
        }
    }
    // First positional argument is the input filename
    if (optind < argc) {
        opts->input_filename = argv[optind];
        optind++;
    }
    // Second positional argument is the output filename
    if (optind < argc) {
        opts->output_filename = argv[optind];
        optind++;
    }
    // Ensure that the run size is a multiple of 4
    opts->run_size = round_up_to_multiple_of_4(opts->run_size);
}

bool check_file_size(int input_fd) {
    struct stat file_status = {0};
    fstat(input_fd, &file_status);
    // Check that the file size is a multiple of 4. This bit magic checks that the lowest two bits are zero. If they
    // are, then the file size is a multiple of 4.
    return ((file_status.st_size & 0x03) == 0);
}

/*
 * This creates the initial sorted runs given an acquired run context.
 */
size_t create_runs_with_context(struct run_context *run, char const *output_filename) {
    size_t num_runs = 0;
    while (!run_finished(run)) {
        // Format the next run filename using the output filename as a base. We'll use the format:
        // "[output_filename].[generation_number].[run_number]"
        // The generation number starts at zero for the initial runs. This will increment later
        // during the merging phase.
        char run_filename[PATH_MAX] = {0};
        snprintf(run_filename, sizeof(run_filename), "%s.0.%lu", output_filename, num_runs);

        // Create and open the run file
        int run_fd = open(run_filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (run_fd < 0) {
            fprintf(stderr, "ERROR: unable to create run file: %s\n", strerror(errno));
            return 0;
        }

        // Generate the run
        bool success = run_create_run(run, run_fd);

        // Close the run file
        close(run_fd);

        if (!success) {
            fprintf(stderr, "ERROR: unable to create run.\n");
            return 0;
        }

        // Update run counter
        num_runs++;
    }
    return num_runs;
}

/*
 * This creates the initial sorted runs. It acquires needed resources, calls another function to create the runs, and
 * then ensures that the resources are released.
 */
size_t create_runs(int input_fd, char const *output_filename, size_t run_size) {
    if (!check_file_size(input_fd)) {
        fprintf(stderr, "ERROR: input file's size must be a multiple of 4.\n");
        return 0;
    }

    struct run_context *run = run_new(input_fd, run_size);
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
bool merge_single_run(
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
bool merge_two_runs(
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

    int input1_run_fd = open(input1_run_filename, O_RDONLY);
    if (input1_run_fd < 0) {
        fprintf(stderr, "ERROR: unable to open run file: %s\n", strerror(errno));
        return false;
    }

    int input2_run_fd = open(input2_run_filename, O_RDONLY);
    if (input2_run_fd < 0) {
        fprintf(stderr, "ERROR: unable to open run file: %s\n", strerror(errno));
        close(input1_run_fd);
        return false;
    }

    // Create and open the output run file
    int output_run_fd = open(output_run_filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (output_run_fd < 0) {
        fprintf(stderr, "ERROR: unable to create run file: %s\n", strerror(errno));
        close(input2_run_fd);
        close(input1_run_fd);
        return false;
    }

    // Perform the merge with the opened files.
    bool success = merge_files(input1_run_fd, input2_run_fd, output_run_fd);

    // Close all files
    close(output_run_fd);
    close(input2_run_fd);
    close(input1_run_fd);

    // Delete input files now that they've been merged into a new file
    if ((remove(input1_run_filename) != 0) || (remove(input2_run_filename) != 0)) {
        fprintf(stderr, "ERROR: unable to remove run file: %s\n", strerror(errno));
        return false;
    }

    return success;
}

/*
 * This merges the initial, sorted runs down into a single, fully sorted, fully merged file.
 * It does so by first merging each pair of first-generation runs into larger, next-generation runs. It then proceeds
 * to merge pairs of next-generation runs into even larger next-next-generation runs. This continues until only one
 * large, final-generation runs remains. This is then renamed to the final output file.
 */
bool merge_runs(char const *output_filename, size_t num_runs) {
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
                    return false;
                }
                // Update the run counter to reflect that we've merged one run
                input_current_run++;
            } else {
                // Merge the next two runs
                if (!merge_two_runs(
                        output_filename,
                        generation, input_current_run,
                        output_generation, num_runs_in_output_generation)) {
                    return false;
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

    // We've now merged down to a single run. Just rename the run file to the final output and return.
    return merge_single_run(output_filename, generation, 0, 0, 0);
}

int main(int argc, char *argv[]) {
    struct options opts = {0};
    get_options(argc, argv, &opts);
    if (opts.print_help) {
        print_usage();
        return EXIT_SUCCESS;
    }
    if (!opts.input_filename) {
        fprintf(stderr, "ERROR: Missing input filename\n");
        print_usage();
        return EXIT_FAILURE;
    }
    if (!opts.output_filename) {
        fprintf(stderr, "ERROR: Missing output filename\n");
        print_usage();
        return EXIT_FAILURE;
    }

    printf(
            "Proceeding with:\n" \
            "  input file: %s\n" \
            "  output file: %s\n" \
            "  run size: %lu\n",
            opts.input_filename, opts.output_filename, opts.run_size);

    // Open the input file to sort
    int input_fd = open(opts.input_filename, O_RDONLY);
    if (input_fd < 0) {
        fprintf(stderr, "ERROR: unable to open input file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Create the initial runs
    size_t num_runs = create_runs(input_fd, opts.output_filename, opts.run_size);
    close(input_fd);

    if (!num_runs) {
        fprintf(stderr, "ERROR: unable to create runs.\n");
        return EXIT_FAILURE;
    }

    // Merge the initial runs into the final output file
    if (!merge_runs(opts.output_filename, num_runs)) {
        fprintf(stderr, "ERROR: unable to merge runs.\n");
        return EXIT_FAILURE;
    }
    printf("Completed successfully!\n");
    return EXIT_SUCCESS;
}
