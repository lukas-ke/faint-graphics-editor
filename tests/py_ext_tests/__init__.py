import os
import unittest
from . import test_extra
from . import test_svg

def extra():
    """Tests faint.extra. That is, less central faint code, possibly
    requiring extensions (e.g. tesseract or GraphViz dot).

    """
    return unittest.defaultTestLoader.discover("py_ext_tests/test_extra",
                                               top_level_dir="py_ext_tests/")
def normal():
    """Regular Python extension unit tests."""
    return unittest.defaultTestLoader.discover("py_ext_tests/test_faint",
                                               top_level_dir="py_ext_tests/")

def svg():
    """More detailed SVG-related unit tests."""
    return unittest.defaultTestLoader.discover("py_ext_tests/test_svg",
                                               top_level_dir="py_ext_tests/")

def load_tests(loader, standard_tests, pattern):
    """The default used by 'unittest' for test discovery when no command
    line argument given.

    """
    s = loader.discover("py_ext_tests/test_faint", top_level_dir="py_ext_tests/")
    s.addTests(svg())
    return s

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
