#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "runs.h"

static size_t const DEFAULT_RUN_SIZE = (size_t)1 * (1<<20); // (1<<20) is 1MB

struct options {
    bool print_help;
    char const *input_filename;
    char const *output_filename;
    size_t run_size;
};

void print_usage() {
    printf(
            "usage: bigsort [-h] [-r runlength] infile outfile\n" \
            "\n" \
            "Sort a large file filled with unsigned, 32-bit integers\n" \
            "\n" \
            "positional arguments:\n" \
            "  infile                  input file name\n" \
            "  outfile                 output file name\n" \
            "\n" \
            "optional arguments:\n" \
            "  -h, --help              show this help message and exit\n" \
            "  -r, --runlength LENGTH  length of initial runs\n"
            "                          (which also determines memory usage)\n");
}

void get_options(int argc, char * const argv[], struct options *opts) {
    static struct option const long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"runlength", required_argument, 0, 'r'},
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
            "  run length: %lu\n",
            opts.input_filename, opts.output_filename, opts.run_size);

    FILE *ifptr = fopen(opts.input_filename,"rb");
    if (!ifptr) {
        printf("ERROR: unable to open input file.\n");
        return -1;
    }
    if (!runs_check_file_size(ifptr)) {
        printf("ERROR: input file's size must be a multiple of 4.\n");
        fclose(ifptr);
        return -1;
    }
    fclose(ifptr);
    return 0;
}
