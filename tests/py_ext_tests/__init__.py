import os
import unittest
from . import test_extra
from . import test_svg

def load_tests(loader, standard_tests, pattern):
    s = loader.discover("py_ext_tests/test_faint", top_level_dir="py_ext_tests/")
    s.addTests(svg())
    return s

def extra():
    return unittest.defaultTestLoader.discover("py_ext_tests/test_extra",
                                               top_level_dir="py_ext_tests/")
def normal():
    return unittest.defaultTestLoader.discover("py_ext_tests/test_faint",
                                               top_level_dir="py_ext_tests/")

def svg():
    return unittest.defaultTestLoader.discover("py_ext_tests/test_svg",
                                               top_level_dir="py_ext_tests/")

TEST_OUT_PATH = None
TEST_DATA_PATH = None

def make_test_dir(test):
    if TEST_OUT_PATH is None:
        raise ValueError("TEST_OUT not set.")

    if not os.path.exists(TEST_OUT_PATH):
        os.mkdir(TEST_OUT_PATH)

    path = os.path.join(TEST_OUT_PATH, test.__class__.__name__ + "." +
                        test._testMethodName)
    if not os.path.exists(path):
        os.mkdir(path)

    return path

def make_common_test_dir(test):
    """Same as make_test_dir, but excluding method name."""
    if TEST_OUT_PATH is None:
        raise ValueError("TEST_OUT not set.")

    if not os.path.exists(TEST_OUT_PATH):
        os.mkdir(TEST_OUT_PATH)

    path = os.path.join(TEST_OUT_PATH, test.__class__.__name__)
    if not os.path.exists(path):
        os.mkdir(path)

    return path

def test_data_dir():
    if TEST_DATA_PATH is None:
        raise ValueError("TEST_DATA_PATH not set")
    return TEST_DATA_PATH
