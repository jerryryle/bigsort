#!/usr/bin/env python3
import struct


def find_first_unsorted_value(filename):
    """
    Given a filename, returns the first unsorted integer value encountered as (integer offset, value) or
    an empty tuple if file is fully sorted.
    """
    with open(filename, 'rb') as file:
        offset = last_value = 0
        byte_value = 0
        while True:
            byte_value = file.read(4)
            if not byte_value:
                break
            current_value = struct.unpack('=L', byte_value)[0]
            # The values should always be increasing. If the current value is smaller than the last value, the file
            # is not sorted.
            if current_value < last_value:
                return offset, current_value, last_value
            last_value = current_value
            offset += 1

    return ()


def check_sorted():
    """
    Checks that a file containing unsigned 32-bit integers is sorted. The endianness of the file is assumed to be the
    current system's endianness.
    """
    import argparse

    parser = argparse.ArgumentParser(
        description='Check that a file containing unsigned 32-bit integers is sorted')
    parser.add_argument('infile', type=str, help='input file name')

    args = parser.parse_args()
    file_name = args.infile

    unsorted_value = find_first_unsorted_value(file_name)
    if unsorted_value:
        offset, value, previous_value = unsorted_value
        print(f"Integer value {hex(value)} at offset {offset} (byte offset {hex(offset*4)}) is mis-sorted.")
        print(f"Is is smaller than the previous value {hex(previous_value)}")
    else:
        print("File is fully sorted!")


if __name__ == '__main__':
    check_sorted()
