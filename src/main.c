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
static size_t const DEFAULT_MAX_FILES = (size_t)1000;

struct options {
    bool print_help;
    char const *input_filename;
    char const *output_filename;
    size_t run_size;
    size_t max_files;
    bool quiet;
};

void print_usage() {
    printf(
            "usage: bigsort [-h] [-q] [-r runsize] [-max maxfiles] infile outfile\n" \
            "\n" \
            "Sort a large file filled with unsigned, 32-bit integers\n" \
            "\n" \
            "positional arguments:\n" \
            "  infile                  input file name\n" \
            "  outfile                 output file name\n" \
            "\n" \
            "optional arguments:\n" \
            "  -h, --help               Show this help message and exit\n" \
            "  -q, --quiet              Do not display progress/stats/completion output\n" \
            "  -r, --runsize=SIZE       Size of initial runs. This drives memory usage since\n" \
            "                             a buffer of size 'SIZE' will be allocated for\n" \
            "                             reading and sorting file data.\n" \
            "                             Defaults to 1MB if not specified.\n" \
            "  -m, --maxfiles=NUM       Maximum number of open files for merge phase. This\n" \
            "                             also drives memory usage since 'NUM' buffered file\n" \
            "                             handles will be opened simultaneously. This flag\n" \
            "                             specifies a maximum. The actual number of open\n" \
            "                             files will be determined by the number of file\n" \
            "                             structures that can fit in the 'SIZE' memory\n" \
            "                             allocated for the initial run processing.\n" \
            "                             Defaults to 1000 if not specified. Specify 0 to\n" \
            "                             open as many files as possible with 'SIZE' memory.\n" \
            "                             (too large of a value may fail due to OS limits)\n" \
            );
}

void get_options(int argc, char * const argv[], struct options *opts) {
    static struct option const long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"runsize", required_argument, 0, 'r'},
            {"quiet", required_argument, 0, 'q'},
            {0, 0, 0, 0}
    };

    // Set default options
    opts->print_help = false;
    opts->input_filename = NULL;
    opts->output_filename = NULL;
    opts->run_size = DEFAULT_RUN_SIZE;
    opts->max_files = DEFAULT_MAX_FILES;
    opts->quiet = false;

    // Loop over arguments, looking for any option flags. These must come before any positional arguments.
    for (;;) {
        int opt = getopt_long(argc, argv, "hqr:", long_options, NULL);
        if (opt == -1) {
            break;
        }
        switch (opt) {
            case 'h':
                opts->print_help = true;
                break;
            case 'q':
                opts->quiet = true;
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

    if (!opts.quiet) {
        printf(
                "--[ Parameters ]-------------------------------\n" \
                "  input file: %s\n" \
                " output file: %s\n" \
                "    run size: %lu\n",
                opts.input_filename, opts.output_filename, opts.run_size);
    }

    // Open the input file to sort
    FILE *input_file = fopen(opts.input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "ERROR: unable to open input file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Allocate working memory based on the requested run size.
    size_t const working_memory_size = opts.run_size;
    void *working_memory = malloc(working_memory_size);
    if (!working_memory) {
        fclose(input_file);
        fprintf(stderr, "ERROR: unable to allocate working memory: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Create the initial runs
    size_t num_runs = create_runs(input_file, opts.output_filename, working_memory, working_memory_size);
    fclose(input_file);

    if (!num_runs) {
        free(working_memory);
        fprintf(stderr, "ERROR: unable to create runs.\n");
        return EXIT_FAILURE;
    }

    // Merge the initial runs into the final output file
    size_t num_generations = merge_runs(opts.output_filename, num_runs, working_memory, working_memory_size, opts.max_files);
    if (!num_generations) {
        free(working_memory);
        fprintf(stderr, "ERROR: unable to merge runs.\n");
        return EXIT_FAILURE;
    }
    if (!opts.quiet) {
        printf("--[ Stats ]------------------------------------\n");
        printf("       initial runs: %lu\n", num_runs);
        printf("  merge generations: %lu\n", num_generations);
        printf("-----------------------------------------------\n");
        printf("Completed successfully!\n");
    }
    return EXIT_SUCCESS;
}
