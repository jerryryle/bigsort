import os
import random
import struct


class DataFiles:
    @staticmethod
    def create_file_with_random_data(file_path, size):
        with open(file_path, 'wb') as file:
            file.write(os.urandom(size))

    @staticmethod
    def create_file_with_shuffled_ascending_integers(file_path, size):
        """
        Creates a list of strictly increasing, unsigned, 32-bit integers from 0 to num_ints, shuffles them randomly, and
        then writes them to the provided file_path.
        """
        with open(file_path, 'wb') as file:
            numbers = list(range(0, int(size/4)))
            random.shuffle(numbers)
            for number in numbers:
                file.write(struct.pack('=L', number))

    @staticmethod
    def find_first_unsorted_value(file_path):
        """
        Given a file that supposedly contains sorted data, returns the first unsorted integer value encountered as
        (integer offset, value) or an empty tuple if file is fully sorted.
        """
        with open(file_path, 'rb') as file:
            offset = previous_value = 0
            while True:
                byte_value = file.read(4)
                if not byte_value:
                    break
                current_value = struct.unpack('=L', byte_value)[0]
                # The values should always be increasing. If the current value is smaller than the last value, the file
                # is not sorted.
                if current_value < previous_value:
                    return offset, current_value, previous_value
                previous_value = current_value
                offset += 1

        return ()

    @staticmethod
    def find_first_incorrect_ascending_value(file_path):
        """
        Given a file that supposedly contains a list of strictly increasing, unsigned, 32-bit integers, returns the
        first incorrect integer value encountered as (integer offset, value, previous value) or an empty tuple if file
        correctly contains a list of strictly increasing, unsigned, 32-bit integers
        """
        with open(file_path, 'rb') as file:
            offset = 0
            previous_value = -1
            while True:
                byte_value = file.read(4)
                if not byte_value:
                    break
                current_value = struct.unpack('=L', byte_value)[0]
                # The values should always be increasing by 1. If the current value is not equal to the last value+1,
                # the file is not sorted, is missing values, or has extra values
                if current_value != previous_value+1:
                    return offset, current_value, previous_value
                previous_value = current_value
                offset += 1

        return ()
