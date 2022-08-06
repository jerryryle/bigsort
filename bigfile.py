#!/usr/bin/env python3
import os
from math import ceil


def write_random_data_to_file(file, size, chunk_size, progress_callback=None):
    """
    Writes random data to a provided file object.

    This function generates random data writes it to the provided file object in chunks
    so that it doesn't exhaust memory for large files and so it can indicate progress
    via a provided callback. The progress callback must take one integer parameter, which
    is the progress of the operation as a percentage.
    """
    size_remaining = size
    last_progress = -1
    while size_remaining > 0:
        write_size = min(chunk_size, size_remaining)
        file.write(os.urandom(write_size))
        size_remaining -= write_size
        if progress_callback is not None:
            progress = round(((size-size_remaining)/size)*100)
            # Don't update the progress if it hasn't changed from the last round
            if progress != last_progress:
                progress_callback(progress)
                last_progress = progress


def make_big_file(filename, size, chunk_size, progress_callback=None):
    """
    Given a filename, creates a file object and writes random data to that file object.
    """
    with open(filename, 'wb') as file:
        write_random_data_to_file(file, size, chunk_size, progress_callback)


def round_size_up_to_multiple_of_4(size):
    """
    Rounds "size" up to a multiple of 4 and returns the new size.
    """
    return 4 * ceil(size / 4)


def bigfile():
    """
    Generates a large file filled with random data, allowing the caller to specify parameters for the generation via
    command line arguments.
    """
    import argparse
    import sys
    # The humanfriendly library over-qualifies its internal imports so there's no simple way to
    # import it with something like `from third_party.humanfriendly.humanfriendly import format_size, parse_size`.
    # It really wants to be installed. To avoid requiring a setup step for this toy program, we extend the path so that
    # the interpreter can locate it as though it were installed.
    sys.path.append(os.path.join(os.path.dirname(__file__), 'third_party/humanfriendly'))
    from humanfriendly import format_size, parse_size

    parser = argparse.ArgumentParser(
        description='Create a large file filled with random binary data, aligned to 32-bit boundary')
    parser.add_argument('outfile', type=str,
                        help='output file name')
    parser.add_argument('size', type=str,
                        help='size of file to generate in bytes or human-friendly format (e.g. "1GB")')
    parser.add_argument('--chunksize', type=str, default='1MB',
                        help='size of chunks to use when writing a file (e.g. "1MB")')

    # Parse the arguments and round the size up to a multiple of 4 to ensure the file is completely filled with
    # valid 32-bit integers
    args = parser.parse_args()
    file_name = args.outfile
    file_size = round_size_up_to_multiple_of_4(parse_size(args.size))
    chunk_size = parse_size(args.chunksize)

    # Generate the big file full of random bytes.
    make_big_file(file_name, file_size, chunk_size, lambda progress: print(f"{progress}%"))


if __name__ == '__main__':
    bigfile()
