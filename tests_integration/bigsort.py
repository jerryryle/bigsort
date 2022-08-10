import os
import subprocess


class BigSort:
    def __init__(self, bigsort_path):
        self._bigsort_path = bigsort_path

    def run(self, input_filename, output_filename, run_size=1000000):
        result = subprocess.run(
            [self._bigsort_path, f'--runsize={run_size}', input_filename, output_filename],
            capture_output=True, encoding='utf-8'
        )
        return result.returncode, result.stderr
