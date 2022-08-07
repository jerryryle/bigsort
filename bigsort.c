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

size_t round_up_to_mutliple_of_4(size_t num) {
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
    if (optind < argc) {
        opts->input_filename = argv[optind];
        optind++;
    }
    if (optind < argc) {
        opts->output_filename = argv[optind];
        optind++;
    }
    // Ensure that the run size is a multiple of 4
    opts->run_size = round_up_to_mutliple_of_4(opts->run_size);
}

bool check_file_size(int input_fd) {
    struct stat file_status;
    fstat(input_fd, &file_status);
    if ((file_status.st_size & 0x03) == 0) {
        return true;
    }
    return false;
}

size_t create_runs_with_context(struct run_context *run, char const *output_filename) {
    size_t num_runs = 0;
    while (!run_finished(run)) {
        // Format the next run filename using the output filename as a base
        char run_filename[PATH_MAX];
        snprintf(run_filename, sizeof(run_filename), "%s.%lu", output_filename, num_runs);

        // Create and open the run file
        int run_fd = open(run_filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (run_fd < 0) {
            printf("ERROR: unable to create run file: %s\n", strerror(errno));
            return 0;
        }

        // Generate the run
        bool success = run_create_run(run, run_fd);

        // Close the run file
        if (close(run_fd) != 0) {
            printf("ERROR: unable to close run file: %s\n", strerror(errno));
            return 0;
        }

        if (!success) {
            printf("ERROR: unable to create run.\n");
            return 0;
        }

        // Update run counter
        num_runs++;
    }
    return num_runs;
}

size_t create_runs(int input_fd, char const *output_filename, size_t run_size) {
    if (!check_file_size(input_fd)) {
        printf("ERROR: input file's size must be a multiple of 4.\n");
        return 0;
    }

    struct run_context *run = run_new(input_fd, run_size);
    if (!run) {
        printf("ERROR: Failed to create run context\n");
        return 0;
    }

    size_t runs = create_runs_with_context(run, output_filename);

    if (!run_delete(run)) {
        printf("ERROR: Failed to delete run context\n");
        return 0;
    }
    return runs;
}

int main(int argc, char *argv[]) {
    struct options opts;
    get_options(argc, argv, &opts);
    if (opts.print_help) {
        print_usage();
        return 0;
    }
    if (!opts.input_filename) {
        printf("ERROR: Missing input filename\n");
        print_usage();
        return -1;
    }
    if (!opts.output_filename) {
        printf("ERROR: Missing output filename\n");
        print_usage();
        return -1;
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
        printf("ERROR: unable to open input file: %s\n", strerror(errno));
        return -1;
    }

    // Create the initial runs
    bool success = create_runs(input_fd, opts.output_filename, opts.run_size);
    if (close(input_fd) < 0) {
        printf("ERROR: unable to close input file: %s\n", strerror(errno));
        return -1;
    }
    if (!success) {
        printf("ERROR: unable to create runs.\n");
        return -1;
    }

    return 0;
}
