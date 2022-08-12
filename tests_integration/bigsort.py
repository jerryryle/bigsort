import re
import subprocess


class BigSortRunResults:
    def __init__(self, return_code, num_runs, num_generations, stderr):
        self.return_code = return_code
        self.num_runs = num_runs
        self.num_generations = num_generations
        self.stderr = stderr


class BigSort:
    def __init__(self, bigsort_path):
        self._bigsort_path = bigsort_path

    def run(self, input_filename, output_filename, run_size=1000000, quiet=False) -> BigSortRunResults:
        if quiet:
            cmd = [self._bigsort_path, '--quiet', f'--runsize={run_size}', input_filename, output_filename]
        else:
            cmd = [self._bigsort_path, f'--runsize={run_size}', input_filename, output_filename]
        result = subprocess.run(cmd, capture_output=True, encoding='utf-8')
        num_runs, num_generations = BigSort._extract_stats(result.stdout)

        return BigSortRunResults(
            return_code=result.returncode,
            num_runs=num_runs,
            num_generations=num_generations,
            stderr=result.stderr)

    @staticmethod
    def _extract_stats(stdout_string) -> (int, int):
        try:
            result = re.search(r'.*initial runs: (\d+)\n', stdout_string, re.MULTILINE)
            num_runs = int(result[1])
            result = re.search(r'.*merge generations: (\d+)\n', stdout_string, re.MULTILINE)
            num_generations = int(result[1])
        except IndexError:
            num_runs = 0
            num_generations = 0
        return num_runs, num_generations
