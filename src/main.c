#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bigsort.h"
#include "round.h"

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
    FILE *input_file = fopen(opts.input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "ERROR: unable to open input file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Create the initial runs
    size_t num_runs = create_runs(input_file, opts.output_filename, opts.run_size);
    fclose(input_file);

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
