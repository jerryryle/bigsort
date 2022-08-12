import os
import pytest
from pathlib import Path
from .bigsort import BigSort
from .data_files import DataFiles


@pytest.fixture(autouse=True, scope='module')
def make_cache_path():
    cache_path = Path('./.test_cache')
    cache_path.mkdir(parents=True, exist_ok=True)

    def _make_cache_path(filename=''):
        return cache_path / filename

    return _make_cache_path


@pytest.fixture
def in_file_path(make_cache_path):
    return make_cache_path('test.in')


@pytest.fixture
def out_file_path(make_cache_path):
    return make_cache_path('test.out')


@pytest.fixture
def bigsort():
    return BigSort(os.getenv('BIGSORT_PATH', './cmake-build-debug/bigsort'))


def test_run_size_smaller_than_data_size(in_file_path, out_file_path, bigsort):
    DataFiles.create_file_with_shuffled_ascending_integers(in_file_path, 1000000)
    result = bigsort.run(
        input_filename=in_file_path,
        output_filename=out_file_path,
        run_size=100000)
    assert result.return_code == 0
    assert result.num_runs == 11
    assert result.num_generations == 4

    result = DataFiles.find_first_incorrect_ascending_value(out_file_path)
    assert result == ()


def test_run_size_same_as_data_size(in_file_path, out_file_path, bigsort):
    DataFiles.create_file_with_shuffled_ascending_integers(in_file_path, 1000000)
    result = bigsort.run(
        input_filename=in_file_path,
        output_filename=out_file_path,
        run_size=1000000)
    assert result.return_code == 0
    assert result.num_runs == 2
    assert result.num_generations == 1

    result = DataFiles.find_first_incorrect_ascending_value(out_file_path)
    assert result == ()
